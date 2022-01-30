/*
 * test_main.cpp
 *
 *  Created on: ۱ آبان ۱۳۹۵ ه‍.ش.
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2017.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#include <iostream>

// Define the test module. It must be defined before including unit_test.hpp
#define BOOST_TEST_MODULE RFlowManagerTester

/* Including this header will include all needed Boost.Test sources so that
 * there is no need to link with any libraries. But it makes compilation of
 * this file very slow. It's better to keep actual tests in other source files.
 */
//#include <boost/test/included/unit_test.hpp>

// Include the following header instead of the above if you link with compiled
// boost_test library
#include <boost/test/unit_test.hpp>

// Disable buffering for stdout, specially useful for XML output
// This is needed to get progress bar in Eclipse
static struct DisableStdCoutBuffering
{
        DisableStdCoutBuffering()
        {
            std::cout.setf(std::ios_base::unitbuf);
        }
} s_disableStdCoutBuffering;
