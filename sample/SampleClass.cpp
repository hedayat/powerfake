/*
 * SampleClass.cpp
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2017-2022.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#include "SampleClass.h"

#include <iostream>

using namespace std;

namespace FakeTest
{


SampleClass::SampleClass()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

SampleClass::~SampleClass()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

void SampleClass::StaticFunc()
{
    cout << "Class static function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

void SampleClass::CallThis() const
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

void SampleClass::CallThisNoExcept() const noexcept
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

int SampleClass::OverloadedCall()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
    return 0;
}

int SampleClass::OverloadedCall(int b [[maybe_unused]])
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
    return 0;
}

int SampleClass::OverloadedCall(int b [[maybe_unused]]) const
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

void SampleClass::PrivateStaticFunc()
{
    cout << "Class private static function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

void FakeTest::SampleClass::SamplePrivate()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
    cout << "Private value: " << private_val << endl;
}

void FakeTest::SampleClass::OverloadedPrivate(int a)
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << "(" << a << ") called."
            << endl;
}

void FakeTest::SampleClass::OverloadedPrivate(float b )
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << "(" << b << ") called."
            << endl;
}

void SampleClass2::DerivedFunc()
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

void SampleClass2::CallThis(int a  [[maybe_unused]])
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called." << endl;
}


void SampleClass2::CallVirtual(int b [[maybe_unused]])
{
    cout << "Real virtual function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

void SampleBase::CallNonPublics(int b)
{
    CallProtectedVirtual(b);
    CallPrivateVirtual(b);
}

void VirtualSample::CallVirtual(int b [[maybe_unused]])
{
    cout << "Real virtual function: " << __PRETTY_FUNCTION__ << " called." << endl;
}

}  // namespace FakeTest
