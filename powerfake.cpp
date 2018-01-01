/*
 * powerfake.cpp
 *
 *  Created on: ۲۷ شهریور ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */


#include "powerfake.h"
#include <vector>

/*
 * These are actually used by bind_fakes, but is also needed to satisfy link
 * time dependencies of main library. It is used so that we don't need to
 * compile object files using WRAP_FUNCTION another time specifically for
 * bind_fakes
 */

namespace PowerFake
{

WrapperBase::Prototypes WrapperBase::wrapped_funcs;

const WrapperBase::Prototypes &WrapperBase::WrappedFunctions()
{
    return wrapped_funcs;
}

// TODO: GCC > 7 supports [[maybe_unused]] (C++17) for [[gnu::unused]]
void WrapperBase::AddFunction(FunctionPrototype prototype [[gnu::unused]])
{
#ifdef BIND_FAKES
    std::cout << "Add function(" << prototype.alias << "): " << prototype.return_type
            << ' ' << prototype.name << prototype.params << std::endl;
    wrapped_funcs.push_back(prototype);
#endif
}

}  // namespace PowerFake
