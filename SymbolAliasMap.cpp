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

void SymbolAliasMap::FindWrappedSymbol(WrapperBase::Prototypes protos,
    const std::string &demangled, const char *symbol_name)
{
    // skip decorated symbol names
    if (strstr(symbol_name, "__PRETTY_FUNCTION__")
            || strstr(symbol_name, "__FUNCTION__")
            || strstr(symbol_name, "__func__"))
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
