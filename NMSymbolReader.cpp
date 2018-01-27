/*
 * NMSymbolReader.cpp
 *
 *  Created on: ۷ بهمن ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2018.
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
    const char *nm_line = reader->ReadLine();
    if (!nm_line)
        return nullptr;

    const char *symbol_name = strrchr(nm_line, ' ');
    if (!symbol_name)
    {
        std::cerr << "Unknown input from nm: " << nm_line << std::endl;
        return nullptr;
    }
    ++symbol_name;
    if (leading_underscore && symbol_name[0] == '_')
        ++symbol_name;

    return symbol_name;
}
