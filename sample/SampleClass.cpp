/*
 * SampleClass.cpp
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
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

void SampleClass::CallThis() const
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

int SampleClass::OverloadedCall(int b [[gnu::unused]]) const
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
    return 0;
}

std::unique_ptr<int> SampleClass::GetIntPtr()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
    return make_unique<int>(4);
}

std::unique_ptr<int> &SampleClass::GetIntPtrReference()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
    static auto myuniq = make_unique<int>(4);
    return myuniq;
}

const std::unique_ptr<int> &SampleClass::GetIntPtrConstReference()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
    static auto myuniq = make_unique<int>(4);
    return myuniq;
}

void SampleClass2::DerivedFunc()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

void SampleClass2::CallThis(int a  [[gnu::unused]])
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
}


void SampleClass2::CallVirtual(int b [[gnu::unused]])
{
    cout << "Real virtual function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

void VirtualSample::CallVirtual(int b [[gnu::unused]])
{
    cout << "Real virtual function: " << __PRETTY_FUNCTION__ << " called." << endl;
}
