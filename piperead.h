/*
 * piperead.h
 *
 *  Created on: ۱۲ مهر ۱۳۹۶
 *      Author: Hedayat Vatankhah
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

        const char *ReadLine()
        {
            ssize_t read = getline(&linebuf, &buf_size, run_stream);
            if (read == -1)
                return nullptr;
            if (read > 0)
                linebuf[read - 1] = '\0';
            else // read = 0
                linebuf[0] = '\0';
            return linebuf;
        }

    private:
        FILE *run_stream = nullptr;
        size_t buf_size = 0;
        char *linebuf = nullptr;
};

#endif /* POWERFAKE_PIPEREAD_H_ */
