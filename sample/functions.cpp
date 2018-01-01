/*
 * functions.cpp
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *      Author: Hedayat Vatankhah
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
