/*
 * SymbolAliasMap.h
 *
 *  Created on: ۱۸ مهر ۱۳۹۶
 *      Author: Hedayat Vatankhah
 */

#ifndef SYMBOLALIASMAP_H_
#define SYMBOLALIASMAP_H_

#include "powerfake.h"
#include <map>
#include <string>

using PowerFake::WrapperBase;

class SymbolAliasMap
{
    public:
        typedef std::map<std::string, std::string> MapType;

    public:
        void AddSymbol(const char *symbol_name);
        const MapType &Map() const { return sym_map; }

    private:
        MapType sym_map;

        void FindWrappedSymbol(WrapperBase::Prototypes protos,
            const std::string &demangled, const char *symbol_name);
};


#endif /* SYMBOLALIASMAP_H_ */
