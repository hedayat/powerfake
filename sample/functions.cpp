/*
 * functions.cpp
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */


#include "functions.h"

#include <iostream>

using namespace std;


void overloaded(int a)
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called with " << a
            << endl;
}

void overloaded(float a)
{
    cout << "Real function: " << __func__ << " called with " << a
            << endl;
}

void normal_func(int b)
{
    cout << "Normal function: " << __FUNCTION__ << " called with " << b
            << endl;
}
