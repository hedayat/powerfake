/*
 * powerfake_test.cpp
 *
 *  Created on: ۱۱ مهر ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017, 2018.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#include "powerfake.h"
#include "Reader.h"
#include "NMSymbolReader.h"

#include <type_traits>
#include <string>
#include <boost/test/unit_test.hpp>
#include <boost/test/framework.hpp>


// A hack since we cannot test AddSymbol() directly
#define private public
#include "SymbolAliasMap.h"
#undef private


using namespace std;
using namespace PowerFake;
using namespace PowerFake::internal;

struct Tag {};
using TestMemberFuncType = void (Tag::*)(int);

namespace PowerFake
{
struct NsTag {};
}  // namespace PowerFake

struct SampleLibConfig
{
        SampleLibConfig()
        {
            auto &master_suite = boost::unit_test::framework::master_test_suite();
            for (int i = 0; i < master_suite.argc; ++i)
            {
                std::string arg = master_suite.argv[i];
                if (arg == "--sample-lib" && i + 1 < master_suite.argc)
                    sample_lib = master_suite.argv[i+1];
            }
        }

        std::string sample_lib;
};


BOOST_AUTO_TEST_CASE(PrototypeExtractorFunctionTest)
{
    BOOST_TEST((std::is_same<PrototypeExtractor<void (*)(int)>::FakeFunction,
            function<void (int)>>::value));

    auto proto_normal = PrototypeExtractor<void (*)(int)>::Extract("folan");
    BOOST_TEST(proto_normal.return_type == "void");
    BOOST_TEST(proto_normal.name == "folan");
    BOOST_TEST(proto_normal.params == "(int)");
}

BOOST_AUTO_TEST_CASE(PrototypeExtractorMemberFunctionTest)
{
    BOOST_TEST((is_same<PrototypeExtractor<TestMemberFuncType>::FakeFunction,
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
    Wrapper<void (NsTag::*)(int)> sample("sample", nullptr,
        internal::Qualifiers::NO_QUAL, "NsTag::folan");

    BOOST_TEST(!sample.Callable());

    bool called_ok;
    auto myfake = MakeFake((void (NsTag::*)(int))nullptr,
        [&called_ok](int a){ called_ok = (a == 4); });
    BOOST_TEST(sample.Callable());

    sample.Call(nullptr, 4);

    BOOST_TEST(sample.Callable());
    BOOST_TEST(called_ok);
}

BOOST_AUTO_TEST_CASE(FunctionFakeTest)
{
    Wrapper<void (*)(int)> folan("folan", nullptr,
        internal::Qualifiers::NO_QUAL, "");

    {
        bool called_ok = false;
        auto myfake = MakeFake((void (*)(int))nullptr,
            [&called_ok](int a){ called_ok = (a == 4); });
        BOOST_TEST(folan.Callable());

        folan.Call(4);
        BOOST_TEST(called_ok);
    }
    BOOST_TEST(!folan.Callable());
}

BOOST_AUTO_TEST_CASE(MemberFunctionSimpleFakeTest)
{
    Wrapper<TestMemberFuncType> folan("folan", nullptr,
        internal::Qualifiers::NO_QUAL, "Tag::function");

    {
        bool called_ok = false;
        auto myfake = MakeFake((TestMemberFuncType)nullptr,
            [&called_ok](int a){ called_ok = (a == 4); });
        BOOST_TEST(folan.Callable());

        folan.Call(nullptr, 4);
        BOOST_TEST(called_ok);
    }
    BOOST_TEST(!folan.Callable());
}

BOOST_AUTO_TEST_CASE(MemberFunctionFullFakeTest)
{
    Wrapper<TestMemberFuncType> folan("folan", nullptr,
        internal::Qualifiers::NO_QUAL, "Tag::function");

    {
        bool called_ok = false;
        auto myfake = MakeFake((TestMemberFuncType)nullptr,
            [&called_ok](Tag *ptr, int a) {
                called_ok = (ptr == nullptr && a == 4); });
        BOOST_TEST(folan.Callable());

        folan.Call(nullptr, 4);
        BOOST_TEST(called_ok);
    }
    BOOST_TEST(!folan.Callable());
}

BOOST_AUTO_TEST_CASE(PipeReadTest)
{
    PipeReader pr("echo 'hi\nhoy\nhey'");

    const char *line = pr.ReadLine();
    BOOST_TEST(line);
    BOOST_TEST(line == "hi");

    line = pr.ReadLine();
    BOOST_TEST(line);
    BOOST_TEST(line == "hoy");

    line = pr.ReadLine();
    BOOST_TEST(line);
    BOOST_TEST(line == "hey");

    line = pr.ReadLine();
    BOOST_TEST(!line);
}

BOOST_FIXTURE_TEST_CASE(NMReaderTest, SampleLibConfig)
{
    PipeReader pipe("nm -o " + sample_lib);
    NMSymbolReader nr(&pipe);

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
    WrapperBase::Functions protos;
    protos.insert( { "folan<char>", { FunctionPrototype("char", "folan<char>", "(int)",
        internal::Qualifiers::NO_QUAL, "alias1") } } );
    protos.insert( { "test_function2", { FunctionPrototype("int", "test_function2", "()",
        internal::Qualifiers::NO_QUAL, "alias2") } } );
    protos.insert( { "test_function", { FunctionPrototype("int", "test_function", "()",
        internal::Qualifiers::NO_QUAL, "alias3") } } );
    protos.insert( { "A::folani", { FunctionPrototype("void", "A::folani", "(int)",
        internal::Qualifiers::NO_QUAL, "alias4") } } );
    protos.insert( { "non_copyable_ref", { FunctionPrototype(
        "std::unique_ptr<int, std::default_delete<int> >", "non_copyable_ref",
        "()", internal::Qualifiers::NO_QUAL, "some_alias") } } );

    SymbolAliasMap sm;
    sm.FindWrappedSymbol(protos, "char folan<char>(int)", "symbol_for_alias1");
    sm.FindWrappedSymbol(protos, "test_function2()", "symbol_for_alias2");
    sm.FindWrappedSymbol(protos, "test_function", "symbol_for_alias3");
    sm.FindWrappedSymbol(protos, "A::folani", "symbol_for_alias4");
    sm.FindWrappedSymbol(protos, "some_nonexistent_function",
        "symbol_for_non_wrapped");

    // test for ignoring static variables inside functions
    sm.FindWrappedSymbol(protos, "non_copyable_ref()", "_Z16non_copyable_refv");
    sm.FindWrappedSymbol(protos, "non_copyable_ref()::felfel",
        "_ZZ16non_copyable_refvE6felfel");
    sm.FindWrappedSymbol(protos, "non_copyable_ref() const::felfel",
        "_ZZ16non_copyable_refvE6felfel2");

    for (int i = 0; i < 4; ++i)
    {
        string no = to_string(i+1);
        BOOST_TEST_MESSAGE("Checking alias no. " << no);
        auto a = sm.Map().find("alias" + no);
        BOOST_TEST(
            (a != sm.Map().end() && a->second->second.symbol == "symbol_for_alias" + no));
    }
}
