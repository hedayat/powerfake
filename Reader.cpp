/*
 * Reader.cpp
 *
 *  Created on: ۷ بهمن ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2018-2022.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#include "Reader.h"

#include <stdexcept>

using namespace std;


Reader::Reader(FILE *input_file) :
        in_file(input_file)
#ifdef __GLIBCXX__
        , filebuf(in_file, std::ios::in), in_stream(&filebuf)
#endif
{
    line_buffer.reserve(1000);
}

const char* Reader::ReadLine()
{
#ifdef __GLIBCXX__
    if (!std::getline(in_stream, line_buffer))
        return nullptr;
    return line_buffer.c_str();
#else
    // a standard implementation
    line_buffer.clear();
    int ch;
    while ((ch = getc(in_file)) != EOF && ch != '\n')
        line_buffer += static_cast<char>(ch);
    if (ch == EOF && line_buffer.empty())
        return nullptr;
    return line_buffer.c_str();
#endif
}

FileReader::FileReader(std::string file_name) :
        Reader(fopen(file_name.c_str(), "r"))
{
}

FileReader::~FileReader()
{
    if (in_file)
        fclose(in_file);
}

PipeReader::PipeReader(std::string cmd): Reader(popen(cmd.c_str(), "r"))
{
    if (!in_file)
        throw std::runtime_error("Error running command: " + cmd);
}

PipeReader::~PipeReader()
{
    if (in_file)
        pclose(in_file);
}
