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
        NMSymbolReader(FILE *nm_symbol_file);
        ~NMSymbolReader();

        const char *NextSymbol();

    private:
        FILE *in_stream = nullptr;
        std::string line_buf;

        const char *ReadLine();
};

#endif /* NMSYMBOLREADER_H_ */
