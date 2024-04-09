/*
 * ParseUtils_test.cpp
 *
 *  Created on: ۱۵ ژانویهٔ ۲۰۲۲
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2022.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#include <ParseUtils.h>

#include <string_view>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace PowerFake::internal;


BOOST_AUTO_TEST_CASE(GetFunctionInfoTest)
{
    auto finfo_opt = GetFunctionInfo("PFKPrototypeStart: WRAPPED | "
            "SampleClass::CallThis | Folan__pfkalias__43 | "
            "void (FakeTest::SampleClass::*)() const noexcept | PFKPrototypeEnd");
    BOOST_TEST(finfo_opt.has_value());
    auto finfo = *finfo_opt;
    BOOST_TEST((finfo.fake_type == FakeType::WRAPPED));
    BOOST_TEST(finfo.prototype.name == "FakeTest::SampleClass::CallThis");
    BOOST_TEST(finfo.prototype.alias == "Folan__pfkalias__43");
    BOOST_TEST(finfo.prototype.params == "()");
    BOOST_TEST(finfo.prototype.qual == (Qualifiers::CONST | Qualifiers::NOEXCEPT));
    BOOST_TEST(finfo.prototype.return_type == "void");

    finfo_opt = GetFunctionInfo("PFKPrototypeStart: NWRAPPED | "
            "SampleClass::CallThis | Folan__pfkalias__43 | "
            "void (FakeTest::SampleClass::*)() const | PFKPrototypeEnd");
    BOOST_TEST(!finfo_opt.has_value());

    finfo_opt = GetFunctionInfo("PFKPrototypeStart: WRAPPED | "
            "SampleClass::StaticFunc | Folan__pfkalias__40 | "
            "const std::unique_ptr<int>& (*)(std::unique_ptr<int>&, float) | "
            "FakeTest::SampleClass | PFKPrototypeEnd");
    BOOST_TEST(finfo_opt.has_value());
    finfo = *finfo_opt;
    BOOST_TEST((finfo.fake_type == FakeType::WRAPPED));
    BOOST_TEST(finfo.prototype.name == "FakeTest::SampleClass::StaticFunc");
    BOOST_TEST(finfo.prototype.alias == "Folan__pfkalias__40");
    BOOST_TEST(finfo.prototype.params == "(std::unique_ptr<int>&, float)");
    BOOST_TEST(finfo.prototype.qual == Qualifiers::NO_QUAL);
    BOOST_TEST(finfo.prototype.return_type == "std::unique_ptr<int> const&");

    finfo_opt = GetFunctionInfo("PFKPrototypeStart: HIDDEN | "
            "called_by_normal_func | Folan__pfkalias__37 | void (*)(int) | "
            "PFKPrototypeEnd");
    BOOST_TEST(finfo_opt.has_value());
    finfo = *finfo_opt;
    BOOST_TEST((finfo.fake_type == FakeType::HIDDEN));
    BOOST_TEST(finfo.prototype.name == "called_by_normal_func");
    BOOST_TEST(finfo.prototype.alias == "Folan__pfkalias__37");
    BOOST_TEST(finfo.prototype.params == "(int)");
    BOOST_TEST(finfo.prototype.qual == Qualifiers::NO_QUAL);
    BOOST_TEST(finfo.prototype.return_type == "void");

    finfo_opt = GetFunctionInfo("PFKPrototypeStart: WRAPPED | "
            "date::current_zone | PowerFakeWrap__pfkalias__29 | "
            "const date::time_zone* (*)() | PFKPrototypeEnd");
    BOOST_TEST(finfo_opt.has_value());
    finfo = *finfo_opt;
    BOOST_TEST((finfo.fake_type == FakeType::WRAPPED));
    BOOST_TEST(finfo.prototype.name == "date::current_zone");
    BOOST_TEST(finfo.prototype.alias == "PowerFakeWrap__pfkalias__29");
    BOOST_TEST(finfo.prototype.params == "()");
    BOOST_TEST(finfo.prototype.qual == Qualifiers::NO_QUAL);
    BOOST_TEST(finfo.prototype.return_type == "date::time_zone const*");

    finfo_opt = GetFunctionInfo("PFKPrototypeStart: WRAPPED | ConnectDB | "
            "PowerFakeWrap__pfkalias__21 | "
            "std::unique_ptr<pg_conn, PGConnectionDeleter> (*)(const DBInfo&) | "
            "PFKPrototypeEnd");
    BOOST_TEST(finfo_opt.has_value());
    finfo = *finfo_opt;
    BOOST_TEST((finfo.fake_type == FakeType::WRAPPED));
    BOOST_TEST(finfo.prototype.name == "ConnectDB");
    BOOST_TEST(finfo.prototype.alias == "PowerFakeWrap__pfkalias__21");
    BOOST_TEST(finfo.prototype.params == "(DBInfo const&)");
    BOOST_TEST(finfo.prototype.qual == Qualifiers::NO_QUAL);
    BOOST_TEST(finfo.prototype.return_type == "std::unique_ptr<pg_conn, PGConnectionDeleter>");

    finfo_opt = GetFunctionInfo("PFKPrototypeStart: HIDDEN | "
            "rpc::FlowReceiver::Stub::Hello | PowerFakeWrap__pfkalias__30 | "
            "grpc::Status (rpc::FlowReceiver::Stub::*)(grpc::ClientContext*, "
            "const rpc::HelloRequest&, rpc::HelloResponse*) | PFKPrototypeEnd");
    BOOST_TEST(finfo_opt.has_value());
    finfo = *finfo_opt;
    BOOST_TEST((finfo.fake_type == FakeType::HIDDEN));
    BOOST_TEST(finfo.prototype.name == "rpc::FlowReceiver::Stub::Hello");
    BOOST_TEST(finfo.prototype.alias == "PowerFakeWrap__pfkalias__30");
    BOOST_TEST(finfo.prototype.params == "(grpc::ClientContext*, "
            "rpc::HelloRequest const&, rpc::HelloResponse*)");
    BOOST_TEST(finfo.prototype.qual == Qualifiers::NO_QUAL);
    BOOST_TEST(finfo.prototype.return_type == "grpc::Status");

    finfo_opt = GetFunctionInfo("PFKPrototypeStart: WRAPPED | SQLQuery::Exec | "
            "PowerFakeWrap__pfkalias__18 | std::unique_ptr<pg_result, "
            "PGResultDeleter> (SQLQuery::*)(std::__cxx11::basic_string<char>, "
            "bool) | PFKPrototypeEnd");
    BOOST_TEST(finfo_opt.has_value());
    finfo = *finfo_opt;
    BOOST_TEST((finfo.fake_type == FakeType::WRAPPED));
    BOOST_TEST(finfo.prototype.name == "SQLQuery::Exec");
    BOOST_TEST(finfo.prototype.alias == "PowerFakeWrap__pfkalias__18");
    BOOST_TEST(finfo.prototype.params == "(std::__cxx11::basic_string<char>, bool)");
    BOOST_TEST(finfo.prototype.qual == Qualifiers::NO_QUAL);
    BOOST_TEST(finfo.prototype.return_type == "std::unique_ptr<pg_result, PGResultDeleter>");

    finfo_opt = GetFunctionInfo("PFKPrototypeStart: WRAPPED | normal_func | "
            "Folan__pfkalias__34 | void (*)(int, "
            "const std::__cxx11::basic_string<char>&, "
            "std::__cxx11::basic_string<char>, "
            "const std::__cxx11::basic_string<char>*, const char** const*, "
            "std::vector<const char*>, const char* (*)(const char*)) | "
            "PFKPrototypeEnd");
    BOOST_TEST(finfo_opt.has_value());
    finfo = *finfo_opt;
    BOOST_TEST((finfo.fake_type == FakeType::WRAPPED));
    BOOST_TEST(finfo.prototype.name == "normal_func");
    BOOST_TEST(finfo.prototype.alias == "Folan__pfkalias__34");
    BOOST_TEST(finfo.prototype.params == "(int, std::__cxx11::basic_string<char> const&, "
            "std::__cxx11::basic_string<char>, std::__cxx11::basic_string<char> const*, "
            "char const** const*, std::vector<char const*>, char const* (*)(char const*))");
    BOOST_TEST(finfo.prototype.qual == Qualifiers::NO_QUAL);
    BOOST_TEST(finfo.prototype.return_type == "void");

}

BOOST_AUTO_TEST_CASE(ParseDemangledFunctionTest)
{
    std::string demangled = "std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > std::operator+<char, std::char_traits<char>, "
            "std::allocator<char> >(std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > const&, std::__cxx11::basic_string<char, "
            "std::char_traits<char>, std::allocator<char> > const&)";
    size_t name_start, name_end;
    FunctionName(demangled, name_start, name_end);
    auto proto = ParseDemangledFunction(demangled, name_start, name_end);
    BOOST_TEST(proto.qual == Qualifiers::NO_QUAL);
    BOOST_TEST(proto.return_type == "std::__cxx11::basic_string<char, "
            "std::char_traits<char>, std::allocator<char> >");
    BOOST_TEST(proto.name == "std::operator+<char, std::char_traits<char>, "
            "std::allocator<char> >");
    BOOST_TEST(proto.expanded_params.size() == 2);
    BOOST_TEST(proto.expanded_params[0] == proto.expanded_params[1]);
    BOOST_TEST(proto.expanded_params[0] == "std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > const&");

    demangled = "FakeTest::SampleClass::OverloadedCall(int) const";
    auto fname = FunctionName(demangled, name_start, name_end);
    BOOST_TEST(fname == "OverloadedCall");
    proto = ParseDemangledFunction(demangled, name_start, name_end);
    BOOST_TEST(proto.qual == Qualifiers::CONST);
    BOOST_TEST(proto.return_type.empty());
    BOOST_TEST(proto.name == "FakeTest::SampleClass::OverloadedCall");
    BOOST_TEST(proto.expanded_params.size() == 1);
    BOOST_TEST(proto.expanded_params[0] == "int");
}

BOOST_AUTO_TEST_CASE(FunctionNameTest)
{
    size_t name_start, name_end;
    auto res = FunctionName("char folan<char>(int)", name_start, name_end);
    BOOST_TEST(res == "folan<char>");

    res = FunctionName("std::function<std::unique_ptr<int, "
            "std::default_delete<int> >& (FakeTest::SampleClass*)>::operator bool() const",
            name_start, name_end);
    BOOST_TEST(res == "operator bool");

    res = FunctionName("std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > std::operator+<char, std::char_traits<char>, "
            "std::allocator<char> >(std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > const&, std::__cxx11::basic_string<char, "
            "std::char_traits<char>, std::allocator<char> > const&)", name_start,
            name_end);
    BOOST_TEST(res == "operator+<char, std::char_traits<char>, "
            "std::allocator<char> >");

    res = FunctionName("std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > operator bool<char, std::char_traits<char>, "
            "std::allocator<char> >(std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > const&, std::__cxx11::basic_string<char, "
            "std::char_traits<char>, std::allocator<char> > const&)", name_start,
            name_end);
    BOOST_TEST(res == "operator bool<char, std::char_traits<char>, "
            "std::allocator<char> >");

    res = FunctionName("std::ostream::operator>>(int)", name_start, name_end);
    BOOST_TEST(res == "operator>>");

    res = FunctionName("std::ostream::operator<<(int)", name_start, name_end);
    BOOST_TEST(res == "operator<<");

    res = FunctionName("Test::operator>(int)", name_start, name_end);
    BOOST_TEST(res == "operator>");

    res = FunctionName("Test::operator<(int)", name_start, name_end);
    BOOST_TEST(res == "operator<");

    res = FunctionName("Folan<int>()", name_start, name_end);
    BOOST_TEST(res == "Folan<int>");

    res = FunctionName("Folan<int>", name_start, name_end);
    BOOST_TEST(res == "Folan<int>");

    res = FunctionName("std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> >::operator[](unsigned long) const",
            name_start, name_end);
    BOOST_TEST(res == "operator[]");

    res = FunctionName("bool operator><char>("
            "std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > const&, "
            "std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > const&)", name_start, name_end);
    BOOST_TEST(res == "operator><char>");

    res = FunctionName("bool operator< <char>("
            "std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > const&, "
            "std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > const&)", name_start, name_end);
    BOOST_TEST(res == "operator< <char>");

    res = FunctionName("bool Class::operator><char>("
            "std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > const&, "
            "std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > const&)", name_start, name_end);
    BOOST_TEST(res == "operator><char>");

    res = FunctionName("bool Class::operator< <char>("
            "std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > const&, "
            "std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > const&)", name_start, name_end);
    BOOST_TEST(res == "operator< <char>");

    res = FunctionName("std::basic_istream<char, std::char_traits<char> >& "
            "std::operator>><char, std::char_traits<char>, std::allocator<char> >("
            "std::basic_istream<char, std::char_traits<char> >&, "
            "std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> >&)", name_start, name_end);
    BOOST_TEST(res == "operator>><char, std::char_traits<char>, "
            "std::allocator<char> >");

    res = FunctionName("std::basic_ostream<char, std::char_traits<char> >& "
            "std::operator<< <std::char_traits<char> >("
            "std::basic_ostream<char, std::char_traits<char> >&, char const*)",
            name_start, name_end);
    BOOST_TEST(res == "operator<< <std::char_traits<char> >");

    res = FunctionName("std::basic_ostream<char, std::char_traits<char> >& "
            "std::operator<< <char, std::char_traits<char>, std::allocator<char> >("
            "std::basic_ostream<char, std::char_traits<char> >&, "
            "std::__cxx11::basic_string<char, std::char_traits<char>, "
            "std::allocator<char> > const&)", name_start, name_end);
    BOOST_TEST(res == "operator<< <char, std::char_traits<char>, std::allocator<char> >");
}

BOOST_AUTO_TEST_CASE(SplitParamsTest)
{
    auto res = SplitParams("()");
    BOOST_TEST(res.empty());

    res = SplitParams("");
    BOOST_TEST(res.empty());

    res = SplitParams("bool, int");
    BOOST_TEST(res[0] == "bool");
    BOOST_TEST(res[1] == "int");

    res = SplitParams("(pg_conn*, "
            "const std::__cxx11::basic_string<char>&)");
    BOOST_TEST(res[0] == "pg_conn*");
    BOOST_TEST(res[1] == "const std::__cxx11::basic_string<char>&");

    res = SplitParams("(std::__cxx11::basic_string<char, "
            "std::char_traits<char>, std::allocator<char> >, bool)");
    BOOST_TEST(res[0] == "std::__cxx11::basic_string<char, "
            "std::char_traits<char>, std::allocator<char> >");
    BOOST_TEST(res[1] == "bool");

    res = SplitParams("(pg_conn*, "
            "const std::__cxx11::basic_string<char>& (*)(const int*, bool))");
    BOOST_TEST(res[0] == "pg_conn*");
    BOOST_TEST(res[1] == "const std::__cxx11::basic_string<char>& (*)(const int*, bool)");
}

BOOST_AUTO_TEST_CASE(NormalizeParametersTest)
{
    auto res = NormalizeParameters("(int, "
            "const std::__cxx11::basic_string<char>&, "
            "std::__cxx11::basic_string<char>, "
            "const std::__cxx11::basic_string<char>*, const char** const*, "
            "std::vector<const char*>, const char* (*)(const char*))");
    BOOST_TEST(res == "(int, std::__cxx11::basic_string<char> const&, "
            "std::__cxx11::basic_string<char>, std::__cxx11::basic_string<char> const*, "
            "char const** const*, std::vector<char const*>, char const* (*)(char const*))");
}

BOOST_AUTO_TEST_CASE(NormalizeTypeTest)
{
    // simple types
    auto res = NormalizeType("const std::__cxx11::basic_string<char>&");
    BOOST_TEST(res == "std::__cxx11::basic_string<char> const&");

    res = NormalizeType("const rpc::HelloRequest&");
    BOOST_TEST(res == "rpc::HelloRequest const&");

    res = NormalizeType("const rpc::HelloRequest*");
    BOOST_TEST(res == "rpc::HelloRequest const*");

    res = NormalizeType("const char* const*");
    BOOST_TEST(res == "char const* const*");

    // template types
    res = NormalizeType("std::vector<const char*, std::allocator<const char*> >");
    BOOST_TEST(res == "std::vector<char const*, std::allocator<char const*> >");

    // function types
    res = NormalizeType("const char* (*)(bool, const char* const*)");
    BOOST_TEST(res == "char const* (*)(bool, char const* const*)");

    res = NormalizeType("const char* (Folan::Bahman::*)(bool, const char* const*)");
    BOOST_TEST(res == "char const* (Folan::Bahman::*)(bool, char const* const*)");

    res = NormalizeType("char (*)(bool, const char* const*)");
    BOOST_TEST(res == "char (*)(bool, char const* const*)");

    res = NormalizeType("char (Folan::Bahman::*)(bool, const char* const*)");
    BOOST_TEST(res == "char (Folan::Bahman::*)(bool, char const* const*)");

    res = NormalizeType("const std::vector<char> &(Folan::Bahman::*)"
            "(std::vector<const bool&>, std::vector<const std::string &>, "
            "std::vector<const std::string &(*)(const char*)>)");
    BOOST_TEST(res == "std::vector<char> const& (Folan::Bahman::*)"
            "(std::vector<bool const&>, std::vector<std::string const&>, "
            "std::vector<std::string const& (*)(char const*)>)");

    // template & function types
    res = NormalizeType("std::vector<const char* (*)(bool, "
            "const char* const*)>");
    BOOST_TEST(res == "std::vector<char const* (*)(bool, "
            "char const* const*)>");

    res = NormalizeType("std::vector<const char* (Folan::Bahman::*)(bool, "
            "const char* const*)>");
    BOOST_TEST(res == "std::vector<char const* (Folan::Bahman::*)(bool, "
            "char const* const*)>");
}

BOOST_AUTO_TEST_CASE(NormalizeTypeWithTypeHintTest)
{
    // simple types
    auto res = NormalizeType("char", {{"char", "char_t"}});
    BOOST_TEST(res == "char_t");

    res = NormalizeType("const std::__cxx11::basic_string<char>&",
        {
            {"std::__cxx11::basic_string<char>",
            "std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >"}
        });
    BOOST_TEST(res == "std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&");

    res = NormalizeType("const char* const*", {{"char", "char_t"}});
    BOOST_TEST(res == "char_t const* const*");

    // template types
    res = NormalizeType("std::vector<const char*, std::allocator<const char*> >",
        {{"char", "char_t"}});
    BOOST_TEST(res == "std::vector<char_t const*, std::allocator<char_t const*> >");

    res = NormalizeType("std::vector<const char*, std::allocator<const char*> >",
        {{"std::vector", "std::vector_t"}});
    BOOST_TEST(res == "std::vector_t<char const*, std::allocator<char const*> >");

    res = NormalizeType("const std::string &",
            {{"std::string", "std::string_t"}});
    BOOST_TEST(res == "std::string_t const&");

    res = NormalizeType("std::vector<const std::string &>",
            {{"std::string", "std::string_t"}, {"std::vector", "std::vector_t"}});
    BOOST_TEST(res == "std::vector_t<std::string_t const&>");

    // function types
    res = NormalizeType("const char* (*)(bool, const char* const*)",
        {{"char", "char_t"}});
    BOOST_TEST(res == "char_t const* (*)(bool, char_t const* const*)");

    res = NormalizeType("const char* (Folan::Bahman::*)(bool, const char* const*)",
        {{"char", "char_t"}});
    BOOST_TEST(res == "char_t const* (Folan::Bahman::*)(bool, char_t const* const*)");

    res = NormalizeType("char (*)(bool, const char* const*)",
        {{"char", "char_t"}});
    BOOST_TEST(res == "char_t (*)(bool, char_t const* const*)");

    res = NormalizeType("const std::vector<char> &(Folan::Bahman::*)"
            "(std::vector<const bool&>, std::vector<const std::string &>, "
            "std::vector<const std::string &(*)(const char*)>)",
            {{"std::string", "std::string_t"}, {"std::vector", "std::vector_t"}});
    BOOST_TEST(res == "std::vector_t<char> const& (Folan::Bahman::*)"
            "(std::vector_t<bool const&>, std::vector_t<std::string_t const&>, "
            "std::vector_t<std::string_t const& (*)(char const*)>)");

    // template & function types
    res = NormalizeType("std::vector<const char* (*)(bool, "
            "const char* const*)>", {{"char", "char_t"}, {"bool", "bool_t"},
                    {"std::vector", "std::vector_t"}});
    BOOST_TEST(res == "std::vector_t<char_t const* (*)(bool_t, "
            "char_t const* const*)>");

    res = NormalizeType("std::vector<const char* (Folan::Bahman::*)(bool, "
            "const char* const*)>", {{"char", "char_t"}, {"bool", "bool_t"},
                    {"std::vector", "std::vector_t"}});
    BOOST_TEST(res == "std::vector_t<char_t const* (Folan::Bahman::*)(bool_t, "
            "char_t const* const*)>");
}

BOOST_AUTO_TEST_CASE(FixSpacesTest)
{
    auto res = FixSpaces("**const *");
    BOOST_TEST(res == "** const*");

    res = FixSpaces("* const *");
    BOOST_TEST(res == "* const*");
}
