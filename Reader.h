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

class Reader
{
    public:
        Reader(FILE *input_file);

        const char *ReadLine();

    protected:
        FILE *in_file;
        std::string line_buf;
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
