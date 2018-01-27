/*
 * NMSymbolReader.cpp
 *
 *  Created on: ۷ بهمن ۱۳۹۶
 *      Author: Hedayat Vatankhah
 */

#include "NMSymbolReader.h"

#include <cstring>
#include <iostream>

using namespace std;


NMSymbolReader::NMSymbolReader(FILE *nm_symbol_file, bool leading_underscore) :
        in_stream(nm_symbol_file), leading_underscore(leading_underscore)
{
    line_buf.reserve(1000);
}

NMSymbolReader::~NMSymbolReader()
{
}

const char* NMSymbolReader::NextSymbol()
{
    const char *nm_line = ReadLine();
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

const char* NMSymbolReader::ReadLine()
{
    line_buf.clear();
    int ch;
    while ((ch = getc(in_stream)) != EOF && ch != '\n')
        line_buf += static_cast<char>(ch);
    if (line_buf.empty())
        return nullptr;
    return line_buf.c_str();
}
