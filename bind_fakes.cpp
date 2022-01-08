/*
 * bind_fakes.cpp
 *
 *  Created on: ۲۶ شهریور ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017, 2018.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#include <charconv>
#include <chrono>
#include <cstring>
#include <iostream>
#include <fstream>
#include <map>
#include <memory>
#include <boost/core/demangle.hpp>

#include "powerfake.h"
#include "NMSymbolReader.h"
#include "SymbolAliasMap.h"

#define TO_STR(a) #a
#define BUILD_NAME_STR(pref, base, post) TO_STR(pref) + base + TO_STR(post)
#define TMP_WRAPPER_NAME_STR(name) BUILD_NAME_STR(TMP_WRAPPER_PREFIX, name, TMP_POSTFIX)
#define TMP_REAL_NAME_STR(name) BUILD_NAME_STR(TMP_REAL_PREFIX, name, TMP_POSTFIX)

using namespace std;
using namespace PowerFake;


string NMCommand(string objfile);
Reader *GetReader(bool passive, string file);
int GetAliasNo(string_view symbol);

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "At least one base library name and one wrapping object/library"
                " are required: " << argv[0] << " <base_lib.a> <wrapper.o/.a>..."
                << endl;
        return 1;
    }

    try
    {
        bool timing = false;
        bool verbose = false;
        bool verify = false;
        bool use_objcopy = true;
        bool passive_mode = false;
        bool enable_symcache = false;
        bool leading_underscore = false;
        bool collecting_symbol_files = false;
        bool collecting_wrapper_files = false;
        string output_prefix = "powerfake";
        vector<string> symbol_files, wrapper_files;

        for (int i = 1; i < argc; ++i)
        {
            bool continue_collect = false;
            if (argv[i] == "--leading-underscore"s)
                leading_underscore = true;
            else if (argv[i] == "--passive"s)
                passive_mode = true;
            else if (argv[i] == "--no-objcopy"s)
                use_objcopy = false;
            else if (argv[i] == "--cache"s)
                enable_symcache = true;
            else if (argv[i] == "--timing"s)
                timing = true;
            else if (argv[i] == "--verbose"s)
                verbose = true;
            else if (argv[i] == "--verify"s)
                verify = true;
            else if (argv[i] == "--output-prefix"s)
            {
                if (i >= argc)
                    throw runtime_error("--output-prefix needs a parameter");
                output_prefix = argv[++i];
            }
            else if (argv[i] == "--symbol-files"s)
            {
                collecting_symbol_files = true;
                collecting_wrapper_files = false;
                continue_collect = true;
            }
            else if (argv[i] == "--wrapper-files"s)
            {
                collecting_wrapper_files = true;
                collecting_symbol_files = false;
                continue_collect = true;
            }
            else if (collecting_symbol_files)
            {
                continue_collect = true;
                symbol_files.push_back(argv[i]);
            }
            else if (collecting_wrapper_files)
            {
                continue_collect = true;
                wrapper_files.push_back(argv[i]);
            }
            else if (argv[i][0] == '-')
                throw runtime_error("Unknown option found: "s + argv[i]);
            else if (symbol_files.empty())
                symbol_files.push_back(argv[i]);
            else if (wrapper_files.empty())
                wrapper_files.push_back(argv[i]);

            if (!continue_collect)
                collecting_symbol_files = collecting_wrapper_files = false;
        }

        auto process_start = chrono::system_clock::now();
        SymbolAliasMap symmap(WrapperBase::WrappedFunctions(), verbose, verify);

        if (enable_symcache)
            symmap.Load(output_prefix + ".symcache");

        // Found real symbols which we want to wrap
        for (const auto &symfile: symbol_files)
        {
            if (!verify && symmap.FoundAllWrappedSymbols())
                break;
            if (verbose)
                cout << "\nLooking for symbol names in " << symfile
                    << "\n------------------------------------------------------------------------------------------------------"
                    << endl;
            unique_ptr<Reader> reader(GetReader(passive_mode, symfile));
            NMSymbolReader nm_reader(reader.get(), leading_underscore);

            const char *symbol;
            while ((symbol = nm_reader.NextSymbol())
                    && (verify || !symmap.FoundAllWrappedSymbols()))
                symmap.AddSymbol(symbol);
        }

        if (!symmap.FoundAllWrappedSymbols())
            throw std::runtime_error("(BUG?) cannot find all wrapped "
                    "symbols in the given library file(s)");

        if (enable_symcache)
            symmap.Save(output_prefix + ".symcache");

        auto symfind_end = chrono::system_clock::now();

        // Create powerfake.link_flags containing link flags for linking
        // test binary
        const string sym_prefix = leading_underscore ? "_" : "";
        ofstream link_flags(output_prefix + ".link_flags");
        ofstream link_script(output_prefix + ".link_script");
        std::multimap<int, const internal::FunctionInfo*> alias_map;
        for (const auto &wf: WrapperBase::WrappedFunctions())
        {
            // if there are multiple wrap files, there can be more than one alias
            // with the same alias number (but different prefix)
            alias_map.insert({ GetAliasNo(wf.prototype.alias), &wf });
            if (wf.fake_type == internal::FakeType::WRAPPED)
                link_flags << "-Wl,--wrap=" << wf.symbol << '\n';
            else if (wf.fake_type == internal::FakeType::HIDDEN)
                link_script << "EXTERN(" << sym_prefix + wf.symbol << ");\n";
        }

        auto symmatch_start = chrono::system_clock::now();
        // Rename our wrap/real symbols (which are mangled) to the ones expected
        // by ld linker
        for (const auto &wrapperfile: wrapper_files)
        {
            if (verbose)
                cout << "\nMatch real symbols with our fake ones in "
                    << wrapperfile
                    << "\n------------------------------------------------------------------------------------------------------"
                    << endl;

            unique_ptr<Reader> reader(GetReader(passive_mode, wrapperfile));
            NMSymbolReader nm_reader(reader.get(), leading_underscore);

            string objcopy_params;
            const char *symbol;
            while ((symbol = nm_reader.NextSymbol()))
            {
                auto alias_no = GetAliasNo(symbol);
                if (symbol[0] == '.' || alias_no == -1)
                    continue;
                auto alias_range = alias_map.equal_range(alias_no);
                for (auto alias_func_it = alias_range.first;
                        alias_func_it != alias_range.second; ++alias_func_it)
                {
                    const auto &wf = *alias_func_it->second;
                    string wrapper_name = TMP_WRAPPER_NAME_STR(wf.prototype.alias);
                    string real_name = TMP_REAL_NAME_STR(wf.prototype.alias);
                    string symbol_str = symbol;
                    if (symbol_str.find(wrapper_name) != string::npos)
                    {
                        if (verbose)
                            cout << "Found wrapper symbol to rename/use: "
                                    << symbol_str << ' '
                                    << boost::core::demangle(symbol) << '\n';
                        if (wf.fake_type == internal::FakeType::HIDDEN)
                            link_script << "PROVIDE(" << sym_prefix
                                << wf.symbol << " = "
                                << sym_prefix << symbol_str << ");\n";
                        else if (!use_objcopy)
                            link_flags << "-Wl,--defsym=" << sym_prefix << "__wrap_"
                                << wf.symbol << '=' << sym_prefix
                                << symbol_str << '\n';
                        else
                            objcopy_params += " --redefine-sym " + sym_prefix
                                + symbol_str + "=" + sym_prefix + "__wrap_"
                                + wf.symbol;
                    }
                    else if (symbol_str.find(real_name) != string::npos)
                    {
                        if (verbose)
                            cout << "Found real symbol to rename: " << symbol_str
                                    << ' ' << boost::core::demangle(symbol) << '\n';
                        if (!use_objcopy)
                            link_flags << "-Wl,--defsym=" << sym_prefix
                                << symbol_str << '=' << sym_prefix << "__real_"
                                << wf.symbol << '\n';
                        else
                            objcopy_params += " --redefine-sym " + sym_prefix
                                + symbol_str + "=" + sym_prefix + "__real_"
                                + wf.symbol;
                    }
                }
            }
            if (use_objcopy)
            {
                if (passive_mode)
                {
                    // Create <objname>.objcopy_params containing objcopy
                    // params to modify symbol names
                    ofstream objcopy_params_file(wrapperfile + ".objcopy_params");
                    objcopy_params_file << objcopy_params << endl;
                }
                else if (!objcopy_params.empty())
                {
                    string cmd = "objcopy -p" + objcopy_params + ' ' + wrapperfile;
                    int ret = system(cmd.c_str());
    #ifdef _XOPEN_SOURCE
                    if (!WIFEXITED(ret) || WEXITSTATUS(ret) != 0)
                        throw runtime_error("Running objcopy failed");
    #endif
                }
            }
        }

        auto symmatch_end = chrono::system_clock::now();
        if (timing)
        {
            auto ts_diff = [](auto end, auto start) {
                return chrono::duration_cast<chrono::milliseconds>(end - start).count();
            };
            cout << "Timing:\n---------------------------\n"
                    "- Finding Symbols: " << ts_diff(symfind_end, process_start)
                    << "ms\n- Matching Symbols: " << ts_diff(symmatch_end,
                        symmatch_start) << "ms\n"
                    "- Total: " << ts_diff(symmatch_end, process_start)
                    << "ms\n";
        }
    }
    catch (exception &e)
    {
        cerr << "Exception: " << e.what() << endl;
        return 1;
    }

    return 0;
}

string NMCommand(string objfile)
{
    return "nm -po " + objfile;
}

Reader *GetReader(bool passive, string file)
{
    if (passive) return new FileReader(file);
    return new PipeReader(NMCommand(file));
}

int GetAliasNo(string_view symbol)
{
    auto p = symbol.find("_alias_");
    if (p == string_view::npos)
        return -1;

    int res;
    auto nums = symbol.substr(p + strlen("_alias_"));
    auto [ptr, ec] = std::from_chars(nums.data(), nums.data() + nums.size(), res);
    if (ec != std::errc())
        return -1;
    return res;
}
