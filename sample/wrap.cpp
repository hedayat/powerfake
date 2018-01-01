/*
 * wrap.cpp
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *      Author: Hedayat Vatankhah
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
