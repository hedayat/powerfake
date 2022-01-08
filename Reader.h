/*
 * Reader.h
 *
 *  Created on: ۷ بهمن ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2018.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef READER_H_
#define READER_H_

#include <stdio.h>
#include <string>
#include <ext/stdio_filebuf.h>
#include <istream>

class Reader
{
    public:
        Reader(FILE *input_file);
        ~Reader();

        const char *ReadLine();

    protected:
        FILE *in_file;
        __gnu_cxx::stdio_filebuf<char> filebuf;
        std::istream in_stream;
        std::string line_buffer;
};

class FileReader: public Reader
{
    public:
        FileReader(std::string file_name);
        ~FileReader();
};

class PipeReader: public Reader
{
    public:
        PipeReader(std::string cmd);
        ~PipeReader();
};

#endif /* READER_H_ */
