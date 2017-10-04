/*
 * bind_fakes.cpp
 *
 *  Created on: ۲۶ شهریور ۱۳۹۶
 *      Author: Hedayat Vatankhah <hedayat.fwd@gmail.com>
 */

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <map>
#include <boost/core/demangle.hpp>

#include "powerfake.h"

#define TO_STR(a) #a
#define BUILD_NAME_STR(pref, base, post) TO_STR(pref) + base + TO_STR(post)
#define TMP_WRAPPER_NAME_STR(name) BUILD_NAME_STR(TMP_WRAPPER_PREFIX, name, TMP_POSTFIX)
#define TMP_REAL_NAME_STR(name) BUILD_NAME_STR(TMP_REAL_PREFIX, name, TMP_POSTFIX)

using namespace std;
using namespace PowerFake;


class PipeRead
{
    public:
        PipeRead(string cmd)
        {
            run_stream = popen(cmd.c_str(), "r");
            if (!run_stream)
                throw runtime_error("Error running nm on the library");
        }
        ~PipeRead()
        {
            pclose(run_stream);
        }

        const char *ReadLine()
        {
            ssize_t read = getline(&linebuf, &buf_size, run_stream);
            if (read == -1)
                return nullptr;
            if (read > 0)
                linebuf[read - 1] = '\0';
            else // read = 0
                linebuf[0] = '\0';
            return linebuf;
        }

    private:
        FILE *run_stream = nullptr;
        size_t buf_size = 0;
        char *linebuf = nullptr;
};

class NMReader
{
    public:
        NMReader(string object_file): pipe_reader("nm -o " + object_file)
        {}

        const char *NextSymbol()
        {
            const char *nm_line = pipe_reader.ReadLine();
            if (!nm_line)
                return nullptr;

            const char *symbol_name = strrchr(nm_line, ' ');
            if (!symbol_name)
            {
                cerr << "Unknown input from nm: " << nm_line << endl;
                return nullptr;
            }
            return ++symbol_name;
        }

    private:
        PipeRead pipe_reader;
};

void ParseNMSymbol(const char *symbol_name);

static map<std::string, std::string> symmap;

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
        vector<string> object_files;
        for (int i = 2; i < argc; ++i)
            object_files.push_back(argv[i]);

        // Found real symbols which we want to wrap
        {
            NMReader nm_reader(argv[1]);

            const char *symbol;
            while ((symbol = nm_reader.NextSymbol()))
                ParseNMSymbol(symbol);
        }

        // Create powerfake.link_flags containing link flags for linking
        // test binary
        {
            ofstream link_flags("powerfake.link_flags");
            for (const auto &syms: symmap)
                link_flags << "-Wl,--wrap=" << syms.second << endl;
        }

        // Rename our wrap/real symbols (which are mangled) to the ones expected
        // by ld linker
        for (const auto &objfile: object_files)
        {
            NMReader nm_reader(objfile);

            string objcopy_params;
            const char *symbol;
            while ((symbol = nm_reader.NextSymbol()))
                for (const auto &syms: symmap)
                {
                    string wrapper_name = TMP_WRAPPER_NAME_STR(syms.first);
                    string real_name = TMP_REAL_NAME_STR(syms.first);
                    string symbol_str = symbol;
                    if (symbol_str.find(wrapper_name) != string::npos)
                    {
                        cout << "Found wrapper symbol to replace: " << symbol_str
                                << ' ' << boost::core::demangle(symbol) << endl;
                        objcopy_params += " --redefine-sym " + symbol_str + "=__wrap_"
                                + syms.second;
                    }
                    if (symbol_str.find(real_name) != string::npos)
                    {
                        cout << "Found real symbol to replace: " << symbol_str
                                << ' ' << boost::core::demangle(symbol) << endl;
                        objcopy_params += " --redefine-sym " + symbol_str + "=__real_"
                                + syms.second;
                    }
                }
            if (!objcopy_params.empty())
            {
                string cmd = "objcopy" + objcopy_params + ' ' + objfile;
                system(cmd.c_str());
            }
            // todo: check return value
        }
    }
    catch (exception &e)
    {
        cerr << "Exception: " << e.what() << endl;
        return 1;
    }

    return 0;
}

/**
 * For each symbol in the main library, finds its alias if it is wrapped and
 * inserts the alias and the actual symbol of the target function in symmap.
 *
 * @param symbol_name the name of a symbol in main library, which might be faked
 */
void ParseNMSymbol(const char *symbol_name)
{
    string demangled = boost::core::demangle(symbol_name);

    // todo: probably use a more efficient code, e.g. using a map
    for (auto func: WrapperBase::WrappedFunctions())
    {
        const string sig = func.name + func.params;
        auto lookup = demangled == func.name ? 0 : demangled.find(sig);
        if (lookup == 0 ||
                demangled == func.return_type + ' ' + sig)
        {
            auto inserted = symmap.insert(make_pair(func.alias, symbol_name));
            if (inserted.second)
                cout << "Found symbol for " << func.return_type << ' '
                        << sig << " == " << symbol_name << " ("
                        << demangled << ") " << endl;
            else if (inserted.first->second != symbol_name)
            {
                cerr << "BUG: Error, duplicate symbols found for: "
                        << func.return_type << ' ' << sig << ":\n"
                        << '\t' << symmap[func.alias] << '\n'
                        << '\t' << symbol_name << endl;
                exit(1);
            }
        }
    }
}
