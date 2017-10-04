/*
 * nmreader.h
 *
 *  Created on: ۱۲ مهر ۱۳۹۶
 *      Author: Hedayat Vatankhah
 */

#ifndef POWERFAKE_NMREADER_H_
#define POWERFAKE_NMREADER_H_

#include <string>
#include <cstring>
#include <iostream>
#include "piperead.h"

class NMReader
{
    public:
        NMReader(std::string object_file): pipe_reader("nm -o " + object_file)
        {}

        const char *NextSymbol()
        {
            const char *nm_line = pipe_reader.ReadLine();
            if (!nm_line)
                return nullptr;

            const char *symbol_name = strrchr(nm_line, ' ');
            if (!symbol_name)
            {
                std::cerr << "Unknown input from nm: " << nm_line << std::endl;
                return nullptr;
            }
            return ++symbol_name;
        }

    private:
        PipeRead pipe_reader;
};

#endif /* POWERFAKE_NMREADER_H_ */
