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


NMSymbolReader::NMSymbolReader(FILE *nm_symbol_file): in_stream(nm_symbol_file)
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
        cerr << "Unknown input from nm: " << nm_line << endl;
        return nullptr;
    }
    return ++symbol_name;
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
