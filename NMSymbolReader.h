/*
 * NMSymbolReader.h
 *
 *  Created on: ۷ بهمن ۱۳۹۶
 *      Author: Hedayat Vatankhah
 */

#ifndef NMSYMBOLREADER_H_
#define NMSYMBOLREADER_H_

#include <stdio.h>
#include <string>

class NMSymbolReader
{
    public:
        NMSymbolReader(FILE *nm_symbol_file, bool leading_underscore = false);
        ~NMSymbolReader();

        const char *NextSymbol();

    private:
        FILE *in_stream;
        bool leading_underscore;
        std::string line_buf;

        const char *ReadLine();
};

#endif /* NMSYMBOLREADER_H_ */
