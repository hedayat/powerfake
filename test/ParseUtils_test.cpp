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
}
