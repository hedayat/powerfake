/*
 * functions.h
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef SAMPLE_FUNCTIONS_H_
#define SAMPLE_FUNCTIONS_H_

#include <string>
#include <memory>

void overloaded(int a);
void overloaded(float a);
std::string overloaded2(int a);
std::string overloaded2(float a);

void normal_func(int b);

void call_virtual_func(class SampleClass2 *sc);

std::unique_ptr<int> &non_copyable_ref();

#endif /* SAMPLE_FUNCTIONS_H_ */
