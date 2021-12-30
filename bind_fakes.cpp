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
        bool passive_mode = false;
        bool leading_underscore = false;
        bool use_objcopy = true;
        string output_prefix = "powerfake";
        int argc_inc = 0;

        for (int i = 1; i < argc; ++i)
        {
            if (argv[i] == "--leading-underscore"s)
            {
                leading_underscore = true;
                argc_inc++;
            }
            else if (argv[i] == "--passive"s)
            {
                passive_mode = true;
                argc_inc++;
            }
            else if (argv[i] == "--objcopy"s) // old option, for compatibility
            {
                use_objcopy = true;
                argc_inc++;
            }
            else if (argv[i] == "--no-objcopy"s)
            {
                use_objcopy = false;
                argc_inc++;
            }
            else if (argv[i] == "--output-prefix"s)
            {
                if (i >= argc)
                    throw runtime_error("--output-prefix needs a parameter");
                output_prefix = argv[++i];
                argc_inc+=2;
            }
            else
                break;
        }

        vector<string> object_files;
        for (int i = argc_inc + 2; i < argc; ++i)
            object_files.push_back(argv[i]);

        SymbolAliasMap symmap;
        // Found real symbols which we want to wrap
        {
            unique_ptr<Reader> reader(GetReader(passive_mode,
                argv[argc_inc + 1]));
            NMSymbolReader nm_reader(reader.get(), leading_underscore);

            const char *symbol;
            while ((symbol = nm_reader.NextSymbol()))
                symmap.AddSymbol(symbol);
        }

        if (!symmap.FoundAllWrappedSymbols())
            throw std::runtime_error("(BUG?) cannot find all wrapped "
                    "symbols in the given library file(s)");

        // Create powerfake.link_flags containing link flags for linking
        // test binary
        const string sym_prefix = leading_underscore ? "_" : "";
        ofstream link_flags(output_prefix + ".link_flags");
        ofstream link_script(output_prefix + ".link_script");
        for (const auto &syms: symmap.Map())
        {
            if (syms.second->second.fake_type == internal::FakeType::WRAPPED)
                link_flags << "-Wl,--wrap=" << syms.second->second.symbol << endl;
            else if (syms.second->second.fake_type == internal::FakeType::HIDDEN)
                link_script << "EXTERN(" << sym_prefix + syms.second->second.symbol << ");" << endl;
        }

        // Rename our wrap/real symbols (which are mangled) to the ones expected
        // by ld linker
        for (const auto &objfile: object_files)
        {
            unique_ptr<Reader> reader(GetReader(passive_mode, objfile));
            NMSymbolReader nm_reader(reader.get(), leading_underscore);

            string objcopy_params;
            const char *symbol;
            while ((symbol = nm_reader.NextSymbol()))
            {
                if (symbol[0] == '.')
                    continue;
                for (const auto &syms: symmap.Map())
                {
                    string wrapper_name = TMP_WRAPPER_NAME_STR(syms.first);
                    string real_name = TMP_REAL_NAME_STR(syms.first);
                    string symbol_str = symbol;
                    if (symbol_str.find(wrapper_name) != string::npos)
                    {
                        cout << "Found wrapper symbol to rename/use: "
                                << symbol_str << ' '
                                << boost::core::demangle(symbol) << endl;
                        if (syms.second->second.fake_type == internal::FakeType::HIDDEN)
                            link_script << "PROVIDE(" << sym_prefix
                                << syms.second->second.symbol << " = "
                                << sym_prefix << symbol_str << ");" << endl;
                        else if (!use_objcopy)
                            link_flags << "-Wl,--defsym=" << sym_prefix << "__wrap_"
                                << syms.second->second.symbol << '=' << sym_prefix
                                << symbol_str << endl;
                        else
                            objcopy_params += " --redefine-sym " + sym_prefix
                                + symbol_str + "=" + sym_prefix + "__wrap_"
                                + syms.second->second.symbol;
                    }
                    if (symbol_str.find(real_name) != string::npos)
                    {
                        cout << "Found real symbol to rename: " << symbol_str
                                << ' ' << boost::core::demangle(symbol) << endl;
                        if (!use_objcopy)
                            link_flags << "-Wl,--defsym=" << sym_prefix
                                << symbol_str << '=' << sym_prefix << "__real_"
                                << syms.second->second.symbol << endl;
                        else
                            objcopy_params += " --redefine-sym " + sym_prefix
                                + symbol_str + "=" + sym_prefix + "__real_"
                                + syms.second->second.symbol;
                    }
                }
            }
            if (use_objcopy)
            {
                if (passive_mode)
                {
                    // Create <objname>.objcopy_params containing objcopy
                    // params to modify symbol names
                    ofstream objcopy_params_file(objfile + ".objcopy_params");
                    objcopy_params_file << objcopy_params << endl;
                }
                else if (!objcopy_params.empty())
                {
                    string cmd = "objcopy -p" + objcopy_params + ' ' + objfile;
                    int ret = system(cmd.c_str());
    #ifdef _XOPEN_SOURCE
                    if (!WIFEXITED(ret) || WEXITSTATUS(ret) != 0)
                        throw runtime_error("Running objcopy failed");
    #endif
                }
            }
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
