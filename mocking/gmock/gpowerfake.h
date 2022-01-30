/*
 * gpowerfake.h
 *
 *  Created on: ۳۰ ژانویهٔ ۲۰۲۲
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2022.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef MOCKING_GMOCK_GPOWERFAKE_H_
#define MOCKING_GMOCK_GPOWERFAKE_H_

#include <powerfake.h>
#include <gmock/gmock.h>


/*
 * GPowerFake Main API
 */
#define GPFK_MOCK_METHOD(RET, FUNC, PARAMS, ...) \
    IGPFK_MOCK_METHOD_BASE(RET, FUNC, ClassType::FUNC, \
        PARAMS __VA_OPT__(,) __VA_ARGS__)

#define GPFK_MOCK_FUNCTION(RET, FUNC, PARAMS, ...) \
    IGPFK_MOCK_METHOD_BASE(RET, FUNC, ::FUNC, PARAMS __VA_OPT__(,) __VA_ARGS__)

namespace PowerFake
{

template <typename C>
class GPowerFake
{
    protected:
        using ClassType = C;
};

} /* namespace PowerFake */


/*
 * Implementation
 */
#define IGPFK_MOCK_METHOD_BASE(...) \
    PFK_SELECT_9TH(__VA_ARGS__,,,, IGPFK_MOCK_METHOD_FULL, \
        IGPFK_MOCK_METHOD_SIMPLE)(__VA_ARGS__)

#define IGPFK_MOCK_METHOD_SIMPLE(RET, FUNC_NAME, FUNC, PARAMS) \
    IGPFK_MOCK_METHOD_FULL(RET, FUNC_NAME, FUNC, PARAMS, ())

#define IGPFK_MOCK_METHOD_FULL(RET, FUNC_NAME, FUNC, PARAMS, SPEC) \
    FakePtr PFK_BUILD_NAME(POWRFAKE_WRAP_NAMESPACE, __mfk__, __LINE__) = \
        MakeFake<IGPFK_FUNC_SIG(RET, PARAMS, SPEC)>(&FUNC, \
            std::function<GMOCK_INTERNAL_SIGNATURE(RET, PARAMS)>( \
                [this] (auto&&... args) -> RET { \
                    return FUNC_NAME(args...); \
                }) \
        ); \
    MOCK_METHOD(RET, FUNC_NAME, PARAMS, SPEC)

#define IGPFK_FUNC_SIG(RET, PARAMS, SPEC) \
    GMOCK_INTERNAL_SIGNATURE(RET, PARAMS) \
    IGPFK_GET_SPEC(GMOCK_INTERNAL_HAS_CONST(SPEC), \
          GMOCK_INTERNAL_GET_REF_SPEC(SPEC))

#define IGPFK_GET_SPEC(CONSTNESS, REF_SPEC) \
    GMOCK_PP_IF(CONSTNESS, const, ) REF_SPEC

// Not available on gmock <= 1.10
#ifndef GMOCK_INTERNAL_GET_REF_SPEC
#define GMOCK_INTERNAL_GET_REF_SPEC(SPEC)
#endif

#endif /* MOCKING_GMOCK_GPOWERFAKE_H_ */
