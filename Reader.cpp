/*
 * Reader.cpp
 *
 *  Created on: ۷ بهمن ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2018.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#include "Reader.h"

#include <stdexcept>

using namespace std;


Reader::Reader(FILE *input_file) :
        in_file(input_file), filebuf(in_file, std::ios::in), in_stream(&filebuf)
{
}

Reader::~Reader()
{
}

const char* Reader::ReadLine()
{
    if (!std::getline(in_stream, line_buffer))
        return nullptr;
    return line_buffer.c_str();
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
