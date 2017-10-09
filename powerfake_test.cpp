/*
 * powerfake_test.cpp
 *
 *  Created on: ۱۱ مهر ۱۳۹۶
 *      Author: Hedayat Vatankhah
 */

#include "powerfake.h"
#include "piperead.h"

#include <boost/test/unit_test.hpp>
#include <type_traits>


using namespace std;
using namespace PowerFake;

struct Tag {};
using TestMemberFuncType = void (Tag::*)(int);

namespace PowerFake
{
struct NsTag {};
}  // namespace PowerFake


BOOST_AUTO_TEST_CASE(PrototypeExtractorFunctionTest)
{
    BOOST_TEST((std::is_same<PrototypeExtractor<void (*)(int)>::FakeType,
            function<void (int)>>::value));

    auto proto_normal = PrototypeExtractor<void (*)(int)>::Extract("folan");
    BOOST_TEST(proto_normal.return_type == "void");
    BOOST_TEST(proto_normal.name == "folan");
    BOOST_TEST(proto_normal.params == "(int)");
}

BOOST_AUTO_TEST_CASE(PrototypeExtractorMemberFunctionTest)
{
    BOOST_TEST((is_same<PrototypeExtractor<TestMemberFuncType>::FakeType,
            function<void (Tag *, int)>>::value));

    auto proto_mfn = PrototypeExtractor<TestMemberFuncType>::Extract(
        "Tag::folan");
    BOOST_TEST(proto_mfn.return_type == "void");
    BOOST_TEST(proto_mfn.name == "Tag::folan");
    BOOST_TEST(proto_mfn.params == "(int)");
}

BOOST_AUTO_TEST_CASE(PrototypeExtractorNamespaceMemberFunctionTest)
{
    auto proto_ns_mfn = PrototypeExtractor<void (NsTag::*)(int)>::Extract(
        "NsTag::folan");
    BOOST_TEST(proto_ns_mfn.return_type == "void");
    BOOST_TEST(proto_ns_mfn.name == "PowerFake::NsTag::folan");
    BOOST_TEST(proto_ns_mfn.params == "(int)");

}

#ifdef BIND_FAKES // AddFunction() only works when BIND_FAKES is defined
BOOST_AUTO_TEST_CASE(WrapperBaseTest)
{
    FunctionPrototype fproto("void", "func_name", "(int, char)");
    WrapperBase wb("WrapperBaseTest_alias", fproto);

    bool found = false;
    for (const auto &test_proto: WrapperBase::WrappedFunctions())
    {
        if (test_proto.alias == fproto.alias &&
                test_proto.return_type == fproto.return_type &&
                test_proto.name == fproto.name &&
                test_proto.params == fproto.params)
            found = true;
    }
    BOOST_TEST(found);
}
#endif

BOOST_AUTO_TEST_CASE(WrapperTest)
{
    Wrapper<void (NsTag::*)(int)> sample("sample",
        FunctionPrototype("", "", ""));

    BOOST_TEST(!sample.Callable());

    bool called_ok;
    auto myfake = MakeFake(sample, [&called_ok](int a){ called_ok = (a == 4); });
    BOOST_TEST(sample.Callable());

    sample.Call(nullptr, 4);

    BOOST_TEST(sample.Callable());
    BOOST_TEST(called_ok);
}

BOOST_AUTO_TEST_CASE(FunctionFakeTest)
{
    Wrapper<void (*)(int)> folan("folan", FunctionPrototype("", "", ""));

    {
        bool called_ok = false;
        auto myfake = MakeFake(folan, [&called_ok](int a){ called_ok = (a == 4); });
        BOOST_TEST(folan.Callable());

        folan.Call(4);
        BOOST_TEST(called_ok);
    }
    BOOST_TEST(!folan.Callable());
}

BOOST_AUTO_TEST_CASE(MemberFunctionSimpleFakeTest)
{
    Wrapper<TestMemberFuncType> folan("folan", FunctionPrototype("", "", ""));

    {
        bool called_ok = false;
        auto myfake = MakeFake(folan, [&called_ok](int a){ called_ok = (a == 4); });
        BOOST_TEST(folan.Callable());

        folan.Call(nullptr, 4);
        BOOST_TEST(called_ok);
    }
    BOOST_TEST(!folan.Callable());
}

BOOST_AUTO_TEST_CASE(MemberFunctionFullFakeTest)
{
    Wrapper<TestMemberFuncType> folan("folan", FunctionPrototype("", "", ""));

    {
        bool called_ok = false;
        auto myfake = MakeFake(folan, [&called_ok](Tag *ptr, int a) {
            called_ok = (ptr == nullptr && a == 4); });
        BOOST_TEST(folan.Callable());

        folan.Call(nullptr, 4);
        BOOST_TEST(called_ok);
    }
    BOOST_TEST(!folan.Callable());
}

BOOST_AUTO_TEST_CASE(PipeReadTest)
{
    PipeRead pr("echo 'hi\nhoy\nhey'");

    const char *line = pr.ReadLine();
    BOOST_TEST(line);
    BOOST_TEST(line == "hi"s);

    line = pr.ReadLine();
    BOOST_TEST(line);
    BOOST_TEST(line == "hoy"s);

    line = pr.ReadLine();
    BOOST_TEST(line);
    BOOST_TEST(line == "hey"s);

    line = pr.ReadLine();
    BOOST_TEST(!line);
}
