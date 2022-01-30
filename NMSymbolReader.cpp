/*
 * NMSymbolReader.cpp
 *
 *  Created on: ۷ بهمن ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2018-2022.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#include "NMSymbolReader.h"

#include <cstring>
#include <iostream>

using namespace std;


NMSymbolReader::NMSymbolReader(Reader *reader, bool leading_underscore) :
        reader(reader), leading_underscore(leading_underscore)
{
}

NMSymbolReader::~NMSymbolReader()
{
}

const char* NMSymbolReader::NextSymbol()
{
    const char *symbol_name;

    do
    {
        const char *nm_line = reader->ReadLine();
        if (!nm_line)
            return nullptr;

        symbol_name = strrchr(nm_line, ' ');
    } while (!symbol_name);

    ++symbol_name;
    if (leading_underscore && symbol_name[0] == '_')
        ++symbol_name;

    return symbol_name;
}
