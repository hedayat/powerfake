/*
 * wrap.cpp
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#include <powerfake.h>

#include "functions.h"
#include "SampleClass.h"

using namespace FakeTest;


// You can define the 'wrap namespace'
// usually not needed, but if WRAP_FUNCTION() is used in multiple files, it
// should be done
#undef POWRFAKE_WRAP_NAMESPACE
#define POWRFAKE_WRAP_NAMESPACE Folan

WRAP_FUNCTION(std::string (*)(int), overloaded2);
WRAP_FUNCTION(std::string (*)(float), overloaded2);
WRAP_FUNCTION(void (*)(int), overloaded);
WRAP_FUNCTION(void (*)(float), overloaded);
WRAP_FUNCTION(normal_func);

WRAP_STATIC_MEMBER(SampleClass, SampleClass::StaticFunc);
WRAP_FUNCTION(SampleClass::CallThis);
WRAP_FUNCTION(SampleClass::CallThisNoExcept);
WRAP_FUNCTION(int (SampleClass::*)(), SampleClass::OverloadedCall);
WRAP_FUNCTION(int (SampleClass::*)(int), SampleClass::OverloadedCall);
WRAP_FUNCTION(int (SampleClass::*)(int) const, SampleClass::OverloadedCall);
WRAP_FUNCTION(SampleClass::GetIntPtr);
WRAP_FUNCTION(SampleClass::GetIntPtrReference);
WRAP_FUNCTION(SampleClass::GetIntPtrConstReference);


WRAP_FUNCTION(void (SampleClass2::*)(int), SampleClass2::CallThis);
WRAP_FUNCTION(SampleClass2::CallVirtual);
WRAP_FUNCTION(VirtualSample::CallVirtual);

WRAP_FUNCTION(non_copyable_ref);

WRAP_FUNCTION(noexcept_func);
