/*
 * wrap.cpp
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2017-2022.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

//#define DISABLE_PFK_SIMPLE_NAMES
// You can define above to prevent defining PRIVATE/STATIC macros
// You can always use these macros with PFK_ prefix, e.g. PFK_STATIC
#include <powerfake.h>

#include "functions.h"
#include "SampleClass.h"

using namespace FakeTest;


// You can define the 'wrap namespace'
// usually not needed, but if WRAP_FUNCTION() is used in multiple files, it
// should be done
#undef POWRFAKE_WRAP_NAMESPACE
#define POWRFAKE_WRAP_NAMESPACE Folan

// You can use type hints if standalone mode have problems in overload resolution
// Example:
//TYPE_HINT("std::__cxx11::basic_string<char>",
//    "std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >");

WRAP_FUNCTION(std::string (int), overloaded2);
WRAP_FUNCTION(std::string (float), overloaded2);
WRAP_FUNCTION(void (int), overloaded);
WRAP_FUNCTION(void (float), overloaded);
WRAP_FUNCTION(normal_func);

#ifdef HIDE_FUNCTION
HIDE_FUNCTION(called_by_normal_func);
#endif

WRAP_FUNCTION(STATIC(SampleClass, SampleClass::StaticFunc));
WRAP_FUNCTION(STATIC(SampleClass, PRIVATE(SampleClass::PrivateStaticFunc)));
//WRAP_FUNCTION(PRIVATE(STATIC(SampleClass, SampleClass::PrivateStaticFunc))); // this works too! :)
WRAP_FUNCTION(SampleClass::CallThis);
WRAP_FUNCTION(SampleClass::CallThisNoExcept);
WRAP_FUNCTION(int (), SampleClass::OverloadedCall);
WRAP_FUNCTION(int (int), SampleClass::OverloadedCall);
WRAP_FUNCTION(int (int) const, SampleClass::OverloadedCall);
WRAP_FUNCTION(SampleClass::GetIntPtr);
WRAP_FUNCTION(SampleClass::GetIntPtrReference);
WRAP_FUNCTION(SampleClass::GetIntPtrConstReference);
WRAP_FUNCTION(PFK_PRIVATE(SampleClass::SamplePrivate));
WRAP_FUNCTION(void (int), PRIVATE(SampleClass::OverloadedPrivate));
WRAP_FUNCTION(void (float), PRIVATE(SampleClass::OverloadedPrivate));


WRAP_FUNCTION(void (int), SampleClass2::CallThis);
WRAP_FUNCTION(SampleClass2::CallVirtual);
WRAP_FUNCTION(VirtualSample::CallVirtual);

WRAP_FUNCTION(non_copyable_ref);

WRAP_FUNCTION(noexcept_func);
