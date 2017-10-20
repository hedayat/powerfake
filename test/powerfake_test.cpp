/*
 * powerfake_test.cpp
 *
 *  Created on: ۱۱ مهر ۱۳۹۶
 *      Author: Hedayat Vatankhah
 */

#include "powerfake.h"
#include "piperead.h"
#include "nmreader.h"

#include <boost/test/unit_test.hpp>
#include <type_traits>
#include "../TestDirConfig.h"

// A hack since we cannot test AddSymbol() directly
#define private public
#include "SymbolAliasMap.h"
#undef private


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

BOOST_FIXTURE_TEST_CASE(NMReaderTest, TestDirConfig)
{
    NMReader nr(test_dir + "/powerfake/libsample_lib.a");

    string symbols[4];
    for (int i = 0; i < 4; ++i)
    {
        symbols[i] = nr.NextSymbol();
        BOOST_TEST_REQUIRE(symbols[i].c_str());
    }
    BOOST_TEST(!nr.NextSymbol());

    sort(symbols, symbols+4);
    BOOST_TEST(boost::core::demangle(symbols[0].c_str()) == "test_function2()");
    BOOST_TEST(boost::core::demangle(symbols[1].c_str()) == "char folan<char>(int)");
    BOOST_TEST(boost::core::demangle(symbols[2].c_str()) == "A::folani(int)");
    BOOST_TEST(symbols[3] == "test_function");

}

BOOST_AUTO_TEST_CASE(FindWrappedSymbolTest)
{
    WrapperBase::Prototypes protos;
    protos.push_back(FunctionPrototype("char", "folan<char>", "(int)",
        "alias1"));
    protos.push_back(FunctionPrototype("int", "test_function2", "()",
        "alias2"));
    protos.push_back(FunctionPrototype("int", "test_function", "()",
        "alias3"));
    protos.push_back(FunctionPrototype("void", "A::folani", "(int)",
        "alias4"));

    SymbolAliasMap sm;
    sm.FindWrappedSymbol(protos, "char folan<char>(int)", "symbol_for_alias1");
    sm.FindWrappedSymbol(protos, "test_function2()", "symbol_for_alias2");
    sm.FindWrappedSymbol(protos, "test_function", "symbol_for_alias3");
    sm.FindWrappedSymbol(protos, "A::folani", "symbol_for_alias4");
    sm.FindWrappedSymbol(protos, "some_nonexistent_function",
        "symbol_for_non_wrapped");

    for (int i = 0; i < 4; ++i)
    {
        string no = to_string(i+1);
        auto a = sm.Map().find("alias" + no);
        BOOST_TEST(
            (a != sm.Map().end() && a->second == "symbol_for_alias" + no));
    }
}