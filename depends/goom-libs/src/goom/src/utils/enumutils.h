#pragma once

#include <cstdint>
#include <magic_enum.hpp>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace GOOM::UTILS
{
template<class E>
constexpr uint32_t NUM = static_cast<size_t>(E::_num);

template<typename E>
constexpr auto ToUType(E value) noexcept
{
  return static_cast<std::underlying_type_t<E>>(value);
}

template<class E>
auto EnumToString(const E value) -> std::string
{
  return std::string(magic_enum::enum_name(value));
}

template<class E>
auto StringToEnum(const std::string& eStr) -> E
{
  if (const auto val = magic_enum::enum_cast<E>(eStr); val)
  {
    return *val;
  }

  throw std::runtime_error("Unknown enum value \"" + eStr + "\".");
}

} // namespace GOOM::UTILS
