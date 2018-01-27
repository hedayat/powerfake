/*
 * piperead.h
 *
 *  Created on: ۱۲ مهر ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef POWERFAKE_PIPEREAD_H_
#define POWERFAKE_PIPEREAD_H_

#include <cstdio>
#include <string>
#include <stdexcept>

class PipeRead
{
    public:
        PipeRead(std::string cmd)
        {
            run_stream = popen(cmd.c_str(), "r");
            if (!run_stream)
                throw std::runtime_error("Error running nm on the library");
        }
        ~PipeRead()
        {
            pclose(run_stream);
        }

        FILE *InputStream() { return run_stream; }

    private:
        FILE *run_stream = nullptr;
};

#endif /* POWERFAKE_PIPEREAD_H_ */
