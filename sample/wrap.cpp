/*
 * wrap.cpp
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *      Author: Hedayat Vatankhah
 */

#include <powerfake.h>
#include "functions.h"

WRAP_FUNCTION(void (*)(int), overloaded, overloaded_int_fake);
WRAP_FUNCTION(void (*)(float), overloaded, overloaded_float_fake);
WRAP_FUNCTION(normal_func, normal_func_fake);
