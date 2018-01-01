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

WRAP_FUNCTION(void (*)(int), overloaded, overloaded_int_fake);
WRAP_FUNCTION(void (*)(float), overloaded, overloaded_float_fake);
WRAP_FUNCTION(normal_func, normal_func_fake);

WRAP_FUNCTION(SampleClass::CallThis, SampleClass_CallThis_fk);
WRAP_FUNCTION(int (SampleClass::*)(), SampleClass::OverloadedCall,
    SampleClass_OverloadedCall_fk);
WRAP_FUNCTION(int (SampleClass::*)(int), SampleClass::OverloadedCall,
    SampleClass_OverloadedCall2_fk);

WRAP_FUNCTION(void (SampleClass2::*)(int), SampleClass2::CallThis,
    SampleClass2_CallThis2_fk);
