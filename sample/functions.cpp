/*
 * functions.cpp
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2017-2022.
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

[[gnu::optimize("no-optimize-sibling-calls")]]
void normal_func(int b, const char **const *c, const std::string &d,
    const char *(*e)(const char *))
{
    cout << "Normal function: " << __FUNCTION__ << " called with " << b
            << ' ' << c << ' ' << d << ' ' << e << endl;
    called_by_normal_func(b);
}

void vararg_func(int count, ...)
{
    cout << "VarArg func: " << __FUNCTION__ << " called with: " << count << endl;
}

[[gnu::noinline]]
void called_by_normal_func(int b)
{
    cout << "Normal (internal call) function: " << __FUNCTION__ << " called with " << b
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

void noexcept_func(int *) noexcept
{
    cout << "A noexcept function: " << __FUNCTION__ << endl;
}
