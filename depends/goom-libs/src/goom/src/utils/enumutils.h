#pragma once

//#define NO_MAGIC_ENUM_AVAILABLE

#include <cstdint>
#ifndef NO_MAGIC_ENUM_AVAILABLE
#include <magic_enum.hpp>
#include <stdexcept>
#endif
#include <string>
#include <type_traits>

namespace GOOM::UTILS
{
template<class E>
static constexpr uint32_t NUM = static_cast<size_t>(E::_num);

template<typename E>
constexpr auto ToUType(E value) noexcept
{
  return static_cast<std::underlying_type_t<E>>(value);
}

#ifdef NO_MAGIC_ENUM_AVAILABLE
template<class E>
auto EnumToString(const E value) -> std::string
{
  return std::to_string(static_cast<int>(value));
}
#else
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
#endif

} // namespace GOOM::UTILS
