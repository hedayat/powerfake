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

Reader::Reader(FILE *input_file) :
        in_file(input_file), line_buffer(nullptr), buffer_size(0)
{
}

Reader::~Reader()
{
    free(line_buffer);
}

const char* Reader::ReadLine()
{
    ssize_t ret = getline(&line_buffer, &buffer_size, in_file);
    if (ret == -1)
        return nullptr;

    if (line_buffer[ret - 1] == '\n')
        line_buffer[ret - 1] = 0;
    return line_buffer;
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
