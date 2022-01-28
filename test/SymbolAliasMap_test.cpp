/*
 * SymbolAliasMap_test.cpp
 *
 *  Created on: ۲۸ ژانویهٔ ۲۰۲۲
 *      Author: Hedayat Vatankhah <hedayat.fwd@gmail.com>
 */


#include "SymbolAliasMap.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/framework.hpp>

using namespace std;
using namespace PowerFake;
using namespace PowerFake::internal;


TAG_PRIVATE(FindWrappedSymbolFunc, SymbolAliasMap::FindWrappedSymbol);
TAG_PRIVATE(GetNumMatchingTypesFunc, SymbolAliasMap::GetNumMatchingTypes);

BOOST_AUTO_TEST_CASE(FindWrappedSymbolTest)
{
    WrapperBase::Functions protos;
    protos.push_back( { FunctionPrototype("char", "folan<char>", "(int)",
        internal::Qualifiers::NO_QUAL, "alias1") } );
    protos.push_back( { FunctionPrototype("int", "test_function2", "()",
        internal::Qualifiers::NO_QUAL, "alias2") } );
    protos.push_back( { FunctionPrototype("int", "test_function", "()",
        internal::Qualifiers::NO_QUAL, "alias3") } );
    protos.push_back( { FunctionPrototype("void", "A::folani", "(int)",
        internal::Qualifiers::NO_QUAL, "alias4") } );
    protos.push_back( { FunctionPrototype(
        "std::unique_ptr<int, std::default_delete<int> >", "non_copyable_ref",
        "()", internal::Qualifiers::NO_QUAL, "some_alias") } );

    SymbolAliasMap sm(protos, true);
    FindWrappedSymbolFunc::Call(sm, "char folan<char>(int)", "symbol_for_alias1");
    FindWrappedSymbolFunc::Call(sm, "test_function2()", "symbol_for_alias2");
    FindWrappedSymbolFunc::Call(sm, "test_function()", "symbol_for_alias3");
    FindWrappedSymbolFunc::Call(sm, "A::folani(int)", "symbol_for_alias4");
    FindWrappedSymbolFunc::Call(sm, "some_nonexistent_function()",
        "symbol_for_non_wrapped");

    // test for ignoring static variables inside functions
    FindWrappedSymbolFunc::Call(sm, "non_copyable_ref()", "_Z16non_copyable_refv");
    FindWrappedSymbolFunc::Call(sm, "non_copyable_ref()::felfel",
        "_ZZ16non_copyable_refvE6felfel");
    FindWrappedSymbolFunc::Call(sm, "non_copyable_ref() const::felfel",
        "_ZZ16non_copyable_refvE6felfel2");

    for (int i = 0; i < 4; ++i)
    {
        string no = to_string(i+1);
        BOOST_TEST_MESSAGE("Checking alias no. " << no);
        auto a = std::find_if(protos.begin(), protos.end(), [no](const FunctionInfo &fi) { return fi.prototype.alias == "alias" + no; } );
        BOOST_TEST(
            (a != protos.end() && a->symbol == "symbol_for_alias" + no));
    }
}

BOOST_AUTO_TEST_CASE(GetNumMatchingTypesTest)
{
    WrapperBase::Functions protos;
    SymbolAliasMap sm(protos, true);
    FunctionPrototype proto;
    ExtendedPrototype ex_proto("", "", "(char)", 0);

    proto.params = "(char)";
    ex_proto.expanded_params = {"char"};
    BOOST_TEST(GetNumMatchingTypesFunc::Call(sm, proto, ex_proto) == vector{ 1 });

    proto.params = "(std::vector<int>)";
    ex_proto.expanded_params = {"std::vector<bool>"};
    vector vv { 0, 1 };
    BOOST_TEST(GetNumMatchingTypesFunc::Call(sm, proto, ex_proto) == vv);

    proto.params = "(int, std::mtpl<int, bool>)";
    ex_proto.expanded_params = {"int", "std::mtpl<int, char>"};
    vv = { 1, 1 };
    BOOST_TEST(GetNumMatchingTypesFunc::Call(sm, proto, ex_proto) == vv);
}
