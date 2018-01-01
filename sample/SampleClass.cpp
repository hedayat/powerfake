/*
 * SampleClass.cpp
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *      Author: Hedayat Vatankhah
 */

#include "SampleClass.h"

#include <iostream>

using namespace std;


SampleClass::SampleClass()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

SampleClass::~SampleClass()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

void SampleClass::CallThis()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

int SampleClass::OverloadedCall()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
    return 0;
}

int SampleClass::OverloadedCall(int b [[gnu::unused]])
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
    return 0;
}

void SampleClass2::DerivedFunc()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

void SampleClass2::CallThis(int a  [[gnu::unused]])
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
}
