/*
 * powerfake_test.cpp
 *
 *  Created on: ۱۱ مهر ۱۳۹۶
 *      Author: Hedayat Vatankhah
 */

#include "powerfake.h"

#include <boost/test/unit_test.hpp>


using TestFuncType = void (int);
struct Tag;

namespace PowerFake
{
template <>
class Wrapper<Tag>
{
    public:
        typedef std::function<TestFuncType> FakeType;
        FakeType fake;
};
}  // namespace PowerFake

using namespace PowerFake;

void sample_function(int)
{
}

BOOST_AUTO_TEST_CASE(FunctionFakeTest)
{
    Wrapper<Tag> folan;

    Wrapper<Tag>::FakeType sample = sample_function;
    {
        auto myfake = MakeFake(folan, sample);
        BOOST_TEST(*folan.fake.target<TestFuncType*>() == *sample.target<TestFuncType*>());
    }
    BOOST_TEST(folan.fake.target<TestFuncType*>() == nullptr);
}
