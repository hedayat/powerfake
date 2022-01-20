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
#include <vector>

#include "powerfake.h"

using Functions = PowerFake::internal::WrapperBase::Functions;

Functions ReadFunctionsList(std::vector<std::string> wrapper_files,
    bool verbose = false);
std::optional<PowerFake::internal::FunctionInfo> GetFunctionInfo(
    std::string_view function_str);

std::string FixConstPlacement(std::string_view compile_type);
std::string FixParamsConstPlacement(std::string_view params);
std::vector<std::string_view> SplitParams(std::string_view params);
std::string FixSpaces(std::string_view type_str);

#endif /* PARSEUTILS_H_ */
