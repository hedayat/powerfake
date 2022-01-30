/*
 * NMSymbolReader.h
 *
 *  Created on: ۷ بهمن ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2018-2022.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef NMSYMBOLREADER_H_
#define NMSYMBOLREADER_H_

#include <stdio.h>
#include <string>

#include "Reader.h"

class NMSymbolReader
{
    public:
        NMSymbolReader(Reader *reader, bool leading_underscore = false);
        ~NMSymbolReader();

        const char *NextSymbol();

    private:
        Reader *reader;
        bool leading_underscore;
};

#endif /* NMSYMBOLREADER_H_ */
