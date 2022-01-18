/*
 * ParseUtils_test.cpp
 *
 *  Created on: ۱۵ ژانویهٔ ۲۰۲۲
 *      Author: Hedayat Vatankhah <hedayat.fwd@gmail.com>
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
    BOOST_TEST(finfo.prototype.qual == Qualifiers::CONST | Qualifiers::NOEXCEPT);
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
    BOOST_TEST(finfo.prototype.return_type == "const std::unique_ptr<int>&");

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
    BOOST_TEST(finfo.prototype.return_type == "const date::time_zone*");

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

    /*
Error: Cannot find symbol for function: std::unique_ptr<pg_result, PGResultDeleter> SQLQuery::Exec(std::__cxx11::basic_string<char>, bool)  (alias: PowerFakeWrap__pfkalias__18)
Error: Cannot find symbol for function: bool TableExists(pg_conn*, const std::__cxx11::basic_string<char>&)  (alias: PowerFakeWrap__pfkalias__22)
Error: Cannot find symbol for function: grpc::Status rpc::FlowReceiver::Stub::Hello(grpc::ClientContext*, const rpc::HelloRequest&, rpc::HelloResponse*)  (alias: PowerFakeWrap__pfkalias__30)
 *
 *
 *Found Prototype: PFKPrototypeStart: WRAPPED | SQLQuery::Exec | PowerFakeWrap__pfkalias__18 | std::unique_ptr<pg_result, PGResultDeleter> (SQLQuery::*)(std::__cxx11::basic_string<char>, bool) | PFKPrototypeEnd
Found Prototype: PFKPrototypeStart: WRAPPED | TableExists | PowerFakeWrap__pfkalias__22 | bool (*)(pg_conn*, const std::__cxx11::basic_string<char>&) | PFKPrototypeEnd
Found Prototype: PFKPrototypeStart: WRAPPED | rpc::FlowReceiver::Stub::Hello | PowerFakeWrap__pfkalias__30 | grpc::Status (rpc::FlowReceiver::Stub::*)(grpc::ClientContext*, const rpc::HelloRequest&, rpc::HelloResponse*) | PFKPrototypeEnd

 *
 */

}
