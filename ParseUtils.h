/*
 * ParseUtils.h
 *
 *  Created on: ۱۵ ژانویهٔ ۲۰۲۲
 *      Author: Hedayat Vatankhah <hedayat.fwd@gmail.com>
 */

#ifndef PARSEUTILS_H_
#define PARSEUTILS_H_

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "powerfake.h"

using Functions = PowerFake::internal::WrapperBase::Functions;

struct ExtendedPrototype: public PowerFake::internal::FunctionPrototype
{
    ExtendedPrototype(std::string ret, std::string name, std::string params,
        uint32_t qual);
    std::vector<std::string_view> expanded_params;
    std::string symbol;
};

Functions ReadFunctionsList(std::vector<std::string> wrapper_files,
    bool verbose = false);
std::optional<PowerFake::internal::FunctionInfo> GetFunctionInfo(
    std::string_view function_str);
std::optional<std::pair<std::string, std::string>> GetTypeHint(
    std::string_view typehint_str);

ExtendedPrototype ParseDemangledFunction(std::string_view demangled,
    unsigned name_start, unsigned name_end);
std::string_view FunctionName(std::string_view demangled, unsigned &start_pos,
    unsigned &end_pos);
PowerFake::internal::Qualifiers QualifierFromStr(std::string_view qs);
std::string NormalizeType(std::string_view compile_type);
std::string NormalizeParameters(std::string_view params);
std::vector<std::string_view> SplitParams(std::string_view params);
std::string FixSpaces(std::string_view type_str);
std::pair<std::string_view::size_type, int> FindStrings(std::string_view prefix,
    std::vector<std::string_view> srch, std::string_view str,
    std::string_view::size_type start_pos = 0);

#endif /* PARSEUTILS_H_ */
