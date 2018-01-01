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
    cout << "Real function: " << "overloaded(int)" << " called." << endl;
}

void overloaded(float a)
{
    cout << "Real function: " << "overloaded(float)" << " called." << endl;
}

void normal_func(int b)
{
    cout << "Normal function: " << "normal_func(int)" << " called." << endl;
}
