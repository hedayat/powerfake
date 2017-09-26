/*
 * powerfake.cpp
 *
 *  Created on: ۲۷ شهریور ۱۳۹۶
 *      Author: Hedayat Vatankhah
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

std::vector<FunctionPrototype> WrapperBase::wrapped_funcs;

const std::vector<FunctionPrototype> &WrapperBase::WrappedFunctions()
{
    return wrapped_funcs;
}

// TODO: GCC > 7 supports [[maybe_unused]] (C++17) for [[gnu::unused]]
void WrapperBase::AddFunction(FunctionPrototype prototype [[gnu::unused]])
{
#ifdef LINK_MOCKER
    std::cout << "Add function(" << prototype.alias << "): " << prototype.return_type
            << ' ' << prototype.name << prototype.params << std::endl;
    wrapped_funcs.push_back(prototype);
#endif
}

}  // namespace PowerFake
