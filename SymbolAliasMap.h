/*
 * SymbolAliasMap.h
 *
 *  Created on: ۱۸ مهر ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef SYMBOLALIASMAP_H_
#define SYMBOLALIASMAP_H_

#include "powerfake.h"
#include <map>
#include <string>
#include <string_view>
#include <vector>
#include "ParseUtils.h"

using PowerFake::internal::WrapperBase;

class SymbolAliasMap
{
    public:
        using Functions = WrapperBase::Functions;
        using FunctionNames = std::multimap<std::string_view, Functions::iterator>;
        using CandidateFunctions = std::map<std::string, std::vector<ExtendedPrototype>>;

    public:
        SymbolAliasMap(Functions &functions, bool approximate_matching,
            bool verbose = false, bool verify_mode = false);

        void Load(std::string_view filename);
        void Save(std::string_view filename);
        void AddSymbol(const char *symbol_name);
        bool FoundAllWrappedSymbols() const;
        void PrintUnresolvedSymbols();
        void ApplyApproximateMatching();

    private:
        const Functions &functions;
        const bool approximate_matching;
        const bool verbose;
        const bool verify_mode;
        FunctionNames functions_map;
        FunctionNames unresolved_functions;
        CandidateFunctions candidates;

        void CreateFunctionMap(Functions &functions);
        std::string_view GetSimpleName(
            const PowerFake::internal::FunctionPrototype &prototype);
        void FindWrappedSymbol(const std::string &demangled,
            const char *symbol_name);
        bool IsFunction(const char *symbol_name, const std::string &demangled);
        bool IsSameFunction(const std::string &demangled,
            const PowerFake::internal::FunctionPrototype &proto);
        bool IsApproximate(const PowerFake::internal::FunctionPrototype &proto,
            const ExtendedPrototype &ex_proto);
        int GetNumMatchingTypes(
            const PowerFake::internal::FunctionPrototype &proto,
            const ExtendedPrototype &ex_proto);
};

#endif /* SYMBOLALIASMAP_H_ */
