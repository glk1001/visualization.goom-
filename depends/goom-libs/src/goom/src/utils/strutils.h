#pragma once

#include <string>
#include <vector>

namespace GOOM::UTILS
{

[[nodiscard]] auto bool_to_string(bool value) -> std::string;

[[nodiscard]] auto StringSplit(const std::string& str, const std::string& delim)
    -> std::vector<std::string>;

[[nodiscard]] auto StringJoin(const std::vector<std::string>& strings, const std::string& delim)
    -> std::string;

} // namespace GOOM::UTILS
