module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <magic_enum.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

export module Goom.Utils.EnumUtils;

import Goom.Lib.AssertUtils;

export namespace GOOM::UTILS
{

template<class E>
concept EnumType = std::is_enum_v<E>;

template<class T>
concept KeyValueArrayType = requires(T a) {
  a[0]; // Accessing elements through [] works.
  a.at(0); // Accessing elements through [] works.
  a.size(); // a must have size() method.
};

template<EnumType E>
inline constexpr auto NUM = static_cast<uint32_t>(magic_enum::enum_count<E>());

template<EnumType E>
auto EnumToString(E value) -> std::string;

template<EnumType E>
auto StringToEnum(const std::string& eStr) -> E;

template<EnumType E, typename T>
class EnumMap
{
public:
  struct KeyValue
  {
    E key; // NOLINT(misc-non-private-member-variables-in-classes)
    T value; // NOLINT(misc-non-private-member-variables-in-classes)
    // NOLINTBEGIN(readability-identifier-length)
    constexpr KeyValue(E k, const T& v) : key{k}, value{v} {}
    constexpr KeyValue(E k, T&& v) : key{k}, value{std::move(v)} {}
    // NOLINTEND(readability-identifier-length)
    KeyValue() = delete;
  };

  constexpr EnumMap() noexcept = default;
  explicit constexpr EnumMap(std::array<KeyValue, NUM<E>>&& keyValues) noexcept;
  static constexpr auto Make(std::vector<KeyValue>&& keyValues) noexcept -> EnumMap<E, T>;

  [[nodiscard]] constexpr auto size() const noexcept -> size_t;
  [[nodiscard]] constexpr auto operator[](E key) const noexcept -> const T&;
  [[nodiscard]] constexpr auto operator[](E key) noexcept -> T&;

  [[nodiscard]] constexpr auto keys() -> std::array<E, NUM<E>>;
  [[nodiscard]] constexpr auto begin() const { return m_keyValues.begin(); }
  [[nodiscard]] constexpr auto begin() { return m_keyValues.begin(); }
  [[nodiscard]] constexpr auto end() const { return m_keyValues.end(); }
  [[nodiscard]] constexpr auto end() { return m_keyValues.end(); }

private:
  explicit constexpr EnumMap(std::array<T, NUM<E>>&& keyValues) noexcept;
  std::array<T, NUM<E>> m_keyValues;
  template<typename V>
  [[nodiscard]] static constexpr auto GetSortedValuesArray(V&& keyValues) noexcept
      -> std::array<T, NUM<E>>;
};

template<EnumType E, typename T>
[[nodiscard]] constexpr auto GetFilledEnumMap(const T& value) -> EnumMap<E, T>;

template<EnumType E, typename T>
class RuntimeEnumMap
{
public:
  using KeyValue = typename EnumMap<E, T>::KeyValue;

  RuntimeEnumMap() = delete;
  explicit RuntimeEnumMap(std::array<KeyValue, NUM<E>>&& keyValues) noexcept;
  static auto Make(std::vector<KeyValue>&& keyValues) noexcept -> RuntimeEnumMap<E, T>;

  [[nodiscard]] auto size() const noexcept -> size_t; // NOLINT(readability-identifier-naming)
  [[nodiscard]] auto operator[](E key) const noexcept -> const T&;
  [[nodiscard]] auto operator[](E key) noexcept -> T&;

private:
  explicit RuntimeEnumMap(std::vector<T>&& keyValues) noexcept;
  std::vector<T> m_keyValues;
  template<KeyValueArrayType V>
  [[nodiscard]] static auto GetSortedValuesArray(const V& keyValues) noexcept -> std::vector<T>;
};

} // namespace GOOM::UTILS

namespace GOOM::UTILS
{

template<EnumType E, typename T>
constexpr EnumMap<E, T>::EnumMap(std::array<KeyValue, NUM<E>>&& keyValues) noexcept
  : m_keyValues{GetSortedValuesArray(std::move(keyValues))}
{
}

template<EnumType E, typename T>
constexpr auto EnumMap<E, T>::Make(std::vector<KeyValue>&& keyValues) noexcept -> EnumMap<E, T>
{
  return EnumMap{GetSortedValuesArray(std::move(keyValues))};
}

template<EnumType E, typename T>
constexpr EnumMap<E, T>::EnumMap(std::array<T, NUM<E>>&& keyValues) noexcept
  : m_keyValues{std::move(keyValues)}
{
}

template<EnumType E, typename T>
constexpr auto EnumMap<E, T>::keys() -> std::array<E, NUM<E>>
{
  auto keys = std::array<E, NUM<E>>{};
  for (auto i = 0U; i < NUM<E>; ++i)
  {
    keys.at(i) = static_cast<E>(i);
  }
  return keys;
}

template<EnumType E, typename T>
constexpr auto EnumMap<E, T>::operator[](const E key) const noexcept -> const T&
{
  return m_keyValues.at(static_cast<size_t>(key));
}

template<EnumType E, typename T>
constexpr auto EnumMap<E, T>::operator[](const E key) noexcept -> T&
{
  return m_keyValues.at(static_cast<size_t>(key));
}

template<EnumType E, typename T>
constexpr auto EnumMap<E, T>::size() const noexcept -> size_t
{
  return m_keyValues.size();
}

template<EnumType E, typename T>
template<typename V>
constexpr auto EnumMap<E, T>::GetSortedValuesArray(V&& keyValues) noexcept -> std::array<T, NUM<E>>
{
  auto sortedValuesArray = std::array<T, NUM<E>>{};
  for (auto& keyValue : std::forward<V>(keyValues))
  {
    sortedValuesArray.at(static_cast<size_t>(keyValue.key)) = std::move(keyValue.value);
  }
  return sortedValuesArray;
}

template<EnumType E, typename T>
[[nodiscard]] constexpr auto GetFilledEnumMap(const T& value) -> EnumMap<E, T>
{
  auto enumMap = EnumMap<E, T>{};
  for (auto& enumValue : enumMap)
  {
    enumValue = value;
  }
  static_assert(enumMap.size() == NUM<E>);

  return enumMap;
}

template<EnumType E, typename T>
RuntimeEnumMap<E, T>::RuntimeEnumMap(std::array<KeyValue, NUM<E>>&& keyValues) noexcept
  : m_keyValues{GetSortedValuesArray(std::move(keyValues))}
{
}

template<EnumType E, typename T>
auto RuntimeEnumMap<E, T>::Make(std::vector<KeyValue>&& keyValues) noexcept -> RuntimeEnumMap<E, T>
{
  return RuntimeEnumMap{GetSortedValuesArray(std::move(keyValues))};
}

template<EnumType E, typename T>
RuntimeEnumMap<E, T>::RuntimeEnumMap(std::vector<T>&& keyValues) noexcept
  : m_keyValues{std::move(keyValues)}
{
}

template<EnumType E, typename T>
auto RuntimeEnumMap<E, T>::operator[](const E key) const noexcept -> const T&
{
  return m_keyValues.at(static_cast<size_t>(key));
}

template<EnumType E, typename T>
auto RuntimeEnumMap<E, T>::operator[](const E key) noexcept -> T&
{
  return m_keyValues.at(static_cast<size_t>(key));
}

template<EnumType E, typename T>
auto RuntimeEnumMap<E, T>::size() const noexcept -> size_t
{
  return m_keyValues.size();
}

template<EnumType E, typename T>
template<KeyValueArrayType V>
auto RuntimeEnumMap<E, T>::GetSortedValuesArray(const V& keyValues) noexcept -> std::vector<T>
{
  Expects(keyValues.size() == NUM<E>);

  auto keysArray = std::vector<uint32_t>(NUM<E>);
  for (auto i = 0U; i < NUM<E>; ++i)
  {
    keysArray.at(static_cast<size_t>(keyValues.at(i).key)) = i;
  }

  auto sortedValuesArray = std::vector<T>{};
  for (const auto& key : keysArray)
  {
    sortedValuesArray.emplace_back(keyValues.at(key).value);
  }
  return sortedValuesArray;
}

template<EnumType E>
auto EnumToString(const E value) -> std::string
{
  return std::string(magic_enum::enum_name(value));
}

template<EnumType E>
auto StringToEnum(const std::string& eStr) -> E
{
  if (const auto val = magic_enum::enum_cast<E>(eStr); val)
  {
    return *val;
  }

  std::unreachable();
}

} // namespace GOOM::UTILS
