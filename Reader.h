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
#include <istream>
#ifdef __GLIBCXX__
#include <ext/stdio_filebuf.h>
#endif

class Reader
{
    public:
        Reader(FILE *input_file);

        const char *ReadLine();

    protected:
        FILE *in_file;
#ifdef __GLIBCXX__
        __gnu_cxx::stdio_filebuf<char> filebuf;
        std::istream in_stream;
#endif
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
