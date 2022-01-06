/*
 * SymbolAliasMap.h
 *
 *  Created on: ۱۸ مهر ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef SYMBOLALIASMAP_H_
#define SYMBOLALIASMAP_H_

#include "powerfake.h"
#include <map>
#include <string>

using PowerFake::internal::WrapperBase;

class SymbolAliasMap
{
    public:
        typedef std::map<std::string, WrapperBase::Functions::iterator> MapType;

    public:
        SymbolAliasMap(bool verbose = false): verbose(verbose) {}
        void AddSymbol(const char *symbol_name);
        const MapType &Map() const { return sym_map; }
        bool FoundAllWrappedSymbols() const;

    private:
        bool verbose;
        MapType sym_map;

        void FindWrappedSymbol(WrapperBase::Functions &protos,
            const std::string &demangled, const char *symbol_name);
        bool IsFunction(const char *symbol_name, const std::string &demangled);
        bool IsSameFunction(const std::string &demangled,
            const PowerFake::internal::FunctionPrototype &proto);
        std::string FunctionName(const std::string &demangled);
};


#endif /* SYMBOLALIASMAP_H_ */
