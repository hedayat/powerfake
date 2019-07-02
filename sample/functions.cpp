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
#include "SampleClass.h"

using namespace std;


std::string overloaded2(int a)
{
    cout << "Real function: " << __PRETTY_FUNCTION__ << " called with " << a
            << endl;
    return "";
}

std::string overloaded2(float a)
{
    cout << "Real function: " << __func__ << " called with " << a
            << endl;
    return "";
}

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

void call_virtual_func(FakeTest::SampleClass2 *sc)
{
	sc->CallVirtual(20);
}

std::unique_ptr<int> &non_copyable_ref()
{
	static std::unique_ptr<int> felfel(new int(4));
	return felfel;
}

void noexcept_func() noexcept
{
    cout << "A noexcept function: " << __FUNCTION__ << endl;
}
