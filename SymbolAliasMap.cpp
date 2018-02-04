/*
 * SymbolAliasMap.cpp
 *
 *  Created on: ۱۸ مهر ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#include "SymbolAliasMap.h"

#include <iostream>
#include <regex>
#include <cstring>
#include "powerfake.h"

using namespace PowerFake;
using namespace std;


/**
 * For each symbol in the main library, finds its alias if it is wrapped and
 * inserts the alias and the actual symbol of the target function in sym_map.
 *
 * @param symbol_name the name of a symbol in main library, which might be faked
 */
void SymbolAliasMap::AddSymbol(const char *symbol_name)
{
    std::string demangled = boost::core::demangle(symbol_name);

    FindWrappedSymbol(WrapperBase::WrappedFunctions(), demangled, symbol_name);
}

/**
 * For a given symbol and its demangled name, finds corresponding prototype
 * from @a protos set and stores the mapping
 * @param protos all wrapped function prototypes
 * @param demangled the demangled form of @a symbol_name
 * @param symbol_name a symbol in the object file
 */
void SymbolAliasMap::FindWrappedSymbol(WrapperBase::Prototypes protos,
    const std::string &demangled, const char *symbol_name)
{
    if (!IsFunction(symbol_name, demangled))
        return;

    // todo: probably use a more efficient code, e.g. using a map
    for (auto func : protos)
    {
        if (IsSameFunction(demangled, func))
        {
            const string sig = func.name + func.params;
            auto inserted = sym_map.insert(make_pair(func.alias, symbol_name));
            if (inserted.second)
                cout << "Found symbol for " << func.return_type << ' ' << sig
                        << " == " << symbol_name << " (" << demangled << ") "
                        << endl;
            else if (inserted.first->second != symbol_name)
            {
                cerr << "BUG: Error, duplicate symbols found for: "
                        << func.return_type << ' ' << sig << ":\n" << '\t'
                        << sym_map[func.alias] << '\n' << '\t' << symbol_name
                        << endl;
                exit(1);
            }
        }
    }
}

bool SymbolAliasMap::IsFunction(const char *symbol_name  [[gnu::unused]],
    const std::string &demangled)
{
    // detect static variables inside functions, which are demangled in
    // this format: function_name()::static_var_name
    auto static_var_separator = demangled.find(")::");
    if (static_var_separator != string::npos
            && demangled.find('(', static_var_separator) == string::npos)
        return false;
    return true;
}

bool SymbolAliasMap::IsSameFunction(const std::string &demangled,
    const PowerFake::FunctionPrototype &proto)
{
    const string base_sig = proto.name + proto.params;
    if (demangled == proto.name)
        return true;
    if (demangled.find(base_sig) == 0)
        return true;
    if (demangled == proto.return_type + ' ' + base_sig)
        return true;

    // signature might contain an abi tag, e.g. func[abi:cxx11](int)
    return ((demangled.find(proto.name + "[") == 0
            || demangled.find(proto.return_type + ' ' + proto.name + "[") == 0)
            && demangled.find("]" + proto.params) != string::npos);
}
