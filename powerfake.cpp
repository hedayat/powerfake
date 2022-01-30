/*
 * powerfake.cpp
 *
 *  Created on: ۲۷ شهریور ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2017-2022.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */


#include "powerfake.h"

#include <stdexcept>

/*
 * These are actually used by bind_fakes, but is also needed to satisfy link
 * time dependencies of main library. It is used so that we don't need to
 * compile object files using WRAP_FUNCTION another time specifically for
 * bind_fakes
 */

namespace PowerFake
{

namespace internal
{

static void AddQual(std::string &res, const char *q)
{
    if (!res.empty())
        res += ' ';
    res += q;
}
std::string ToStr(uint32_t q, bool mangled)
{
    switch (q)
    {
        case Qualifiers::LV_REF:
            return "&";
        case Qualifiers::RV_REF:
            return "&&";
        case Qualifiers::CONST_REF:
            return "const &";
    }
    std::string res;
    if (q & Qualifiers::CONST)
        AddQual(res, "const");
    if (q & Qualifiers::VOLATILE)
        AddQual(res, "volatile");
    if (!mangled && (q & Qualifiers::NOEXCEPT)) // noexcept is not mangled
        AddQual(res, "noexcept");
    return res;
}

// using pointers, as we can't rely on the order of construction of static
// objects
WrapperBase::Functions *WrapperBase::wrapped_funcs = nullptr;
WrapperBase::FunctionWrappers *WrapperBase::wrappers = nullptr;

WrapperBase::Functions &WrapperBase::WrappedFunctions()
{
    if (!wrapped_funcs)
        wrapped_funcs = new Functions();
    return *wrapped_funcs;
}

void WrapperBase::AddFunction(FunctionKey func_key,
    FunctionPrototype prototype [[maybe_unused]],
    FakeType fake_type [[maybe_unused]])
{
#ifdef BIND_FAKES
    if (!wrapped_funcs)
        wrapped_funcs = new Functions();
//    std::cout << "Add function prototype(" << prototype.alias << "): "
//            << prototype.Str() << std::endl;
    wrapped_funcs->push_back({ prototype, fake_type });
#endif
//    std::cout << this << ": Add function(" << prototype.alias << ")["
//            << func_key.first << ", "
//            << boost::core::demangle(func_key.second.name()) << "]: "
//            << prototype.return_type
//            << ' ' << prototype.name << prototype.params << std::endl;
    if (!wrappers)
        wrappers = new FunctionWrappers;
    auto [it, inserted] = wrappers->insert({func_key, this});
    if (!inserted)
        throw std::runtime_error("Cannot register wrapped function: "
            + prototype.Str() + "! Either it is marked with HIDE/WRAP_FUNCTION "
                    "macros multiple times, or this is a PowerFake bug. In the "
                    "latter case please report an issue to PowerFake project.");
}

}  // namespace internal

}  // namespace PowerFake
