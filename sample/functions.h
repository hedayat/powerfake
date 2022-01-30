/*
 * functions.h
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2017-2022.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef SAMPLE_FUNCTIONS_H_
#define SAMPLE_FUNCTIONS_H_

#include <string>
#include <memory>
#include <vector>

namespace FakeTest
{
class SampleClass2;
}  // namespace FakeTest


void overloaded(int a);
void overloaded(float a);
std::string overloaded2(int a);
std::string overloaded2(float a);

void normal_func(int b, const char **const *c, const std::string &d,
    const char *(*e)(const char *));

void called_by_normal_func(int b);

void call_virtual_func(FakeTest::SampleClass2 *sc);

std::unique_ptr<int> &non_copyable_ref();

void noexcept_func() noexcept;

#endif /* SAMPLE_FUNCTIONS_H_ */
