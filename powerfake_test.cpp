/*
 * powerfake_test.cpp
 *
 *  Created on: ۱۱ مهر ۱۳۹۶
 *      Author: Hedayat Vatankhah
 */

#include "powerfake.h"

#include <boost/test/unit_test.hpp>
#include <type_traits>

using namespace std;
using namespace PowerFake;

struct Tag {};
using TestMemberFuncType = void (Tag::*)(int);

namespace PowerFake
{
template <>
class Wrapper<Tag>
{
    public:
        typedef std::function<void (int)> FakeType;
        FakeType fake;
};

template <>
class Wrapper<TestMemberFuncType>
{
    public:
        typedef typename PrototypeExtractor<TestMemberFuncType>::FakeType FakeType;
        FakeType fake;
};

struct NsTag {};
}  // namespace PowerFake


BOOST_AUTO_TEST_CASE(PrototypeExtractorTest)
{
    BOOST_TEST((is_same<PrototypeExtractor<TestMemberFuncType>::FakeType,
            function<void (Tag *, int)>>::value));
    BOOST_TEST((std::is_same<PrototypeExtractor<void (*)(int)>::FakeType,
            function<void (int)>>::value));

    auto proto_normal = PrototypeExtractor<void (*)(int)>::Extract("folan");
    BOOST_TEST(proto_normal.return_type == "void");
    BOOST_TEST(proto_normal.name == "folan");
    BOOST_TEST(proto_normal.params == "(int)");

    auto proto_mfn = PrototypeExtractor<TestMemberFuncType>::Extract(
        "Tag::folan");
    BOOST_TEST(proto_mfn.return_type == "void");
    BOOST_TEST(proto_mfn.name == "Tag::folan");
    BOOST_TEST(proto_mfn.params == "(int)");

    auto proto_ns_mfn = PrototypeExtractor<void (NsTag::*)(int)>::Extract(
        "NsTag::folan");
    BOOST_TEST(proto_ns_mfn.return_type == "void");
    BOOST_TEST(proto_ns_mfn.name == "PowerFake::NsTag::folan");
    BOOST_TEST(proto_ns_mfn.params == "(int)");

}

BOOST_AUTO_TEST_CASE(FunctionFakeTest)
{
    Wrapper<Tag> folan;
    BOOST_TEST(!folan.fake);

    {
        bool called_ok = false;
        auto myfake = MakeFake(folan, [&called_ok](int a){ called_ok = (a == 4); });
        BOOST_TEST(static_cast<bool>(folan.fake));

        folan.fake(4);
        BOOST_TEST(called_ok);
    }
    BOOST_TEST(!folan.fake);
}

BOOST_AUTO_TEST_CASE(MemberFunctionSimpleFakeTest)
{
    Wrapper<TestMemberFuncType> folan;
    BOOST_TEST(!folan.fake);

    {
        bool called_ok = false;
        auto myfake = MakeFake(folan, [&called_ok](int a){ called_ok = (a == 4); });
        BOOST_TEST(static_cast<bool>(folan.fake));

        folan.fake(nullptr, 4);
        BOOST_TEST(called_ok);
    }
    BOOST_TEST(!folan.fake);
}

BOOST_AUTO_TEST_CASE(MemberFunctionFullFakeTest)
{
    Wrapper<TestMemberFuncType> folan;
    BOOST_TEST(!folan.fake);

    {
        bool called_ok = false;
        auto myfake = MakeFake(folan, [&called_ok](Tag *ptr, int a) {
            called_ok = (ptr == nullptr && a == 4); });
        BOOST_TEST(static_cast<bool>(folan.fake));

        folan.fake(nullptr, 4);
        BOOST_TEST(called_ok);
    }
    BOOST_TEST(!folan.fake);
}
