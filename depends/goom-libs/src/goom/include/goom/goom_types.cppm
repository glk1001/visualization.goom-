module;

#include <algorithm>
#include <cstdint>
#include <numeric>

export module Goom.Lib.GoomTypes;

import Goom.Lib.AssertUtils;
import Goom.Lib.Point2d;

export namespace GOOM
{

using UnderlyingEnumType = std::int8_t;

enum class ShowSongTitleType : UnderlyingEnumType
{
  AT_START,
  ALWAYS,
  NEVER,
};

template<typename T>
struct MinMaxValues
{
  T minValue;
  T maxValue;
};

template<typename T>
[[nodiscard]] constexpr auto GetStartingMinMaxValues() noexcept -> MinMaxValues<T>;

template<typename T>
constexpr auto UpdateMinMaxValues(MinMaxValues<T>& minMaxValues, T value) noexcept -> void;

template<typename T>
[[nodiscard]] constexpr auto GetMidpointOfMinMax(const MinMaxValues<T>& minMaxValues) noexcept -> T;

struct Rectangle2dInt
{
  Point2dInt topLeft{};
  Point2dInt bottomRight{};
};
struct Rectangle2dFlt
{
  Point2dFlt topLeft{};
  Point2dFlt bottomRight{};
};
[[nodiscard]] constexpr auto ToRectangle2dFlt(const Rectangle2dInt& rectangle) noexcept
    -> Rectangle2dFlt;

class Dimensions
{
public:
  constexpr Dimensions(uint32_t width, uint32_t height) noexcept;

  [[nodiscard]] constexpr auto GetWidth() const noexcept -> uint32_t;
  [[nodiscard]] constexpr auto GetHeight() const noexcept -> uint32_t;
  [[nodiscard]] constexpr auto GetSize() const noexcept -> uint32_t;

  [[nodiscard]] constexpr auto GetIntWidth() const noexcept -> int32_t;
  [[nodiscard]] constexpr auto GetIntHeight() const noexcept -> int32_t;

  [[nodiscard]] constexpr auto GetFltWidth() const noexcept -> float;
  [[nodiscard]] constexpr auto GetFltHeight() const noexcept -> float;

  [[nodiscard]] constexpr auto GetCentrePoint() const noexcept -> Point2dInt;

private:
  uint32_t m_width;
  uint32_t m_height;
};

} // namespace GOOM

namespace GOOM
{

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
constexpr Dimensions::Dimensions(const uint32_t width, const uint32_t height) noexcept
  : m_width{width}, m_height{height}
{
  Expects(width > 0);
  Expects(height > 0);
}

constexpr auto Dimensions::GetWidth() const noexcept -> uint32_t
{
  return m_width;
}

constexpr auto Dimensions::GetHeight() const noexcept -> uint32_t
{
  return m_height;
}

constexpr auto Dimensions::GetSize() const noexcept -> uint32_t
{
  return m_width * m_height;
}

constexpr auto Dimensions::GetIntWidth() const noexcept -> int32_t
{
  return static_cast<int32_t>(m_width);
}

constexpr auto Dimensions::GetIntHeight() const noexcept -> int32_t
{
  return static_cast<int32_t>(m_height);
}

constexpr auto Dimensions::GetFltWidth() const noexcept -> float
{
  return static_cast<float>(m_width);
}

constexpr auto Dimensions::GetFltHeight() const noexcept -> float
{
  return static_cast<float>(m_height);
}

constexpr auto Dimensions::GetCentrePoint() const noexcept -> Point2dInt
{
  return MidpointFromOrigin({.x = GetIntWidth(), .y = GetIntHeight()});
}

template<typename T>
constexpr auto GetStartingMinMaxValues() noexcept -> MinMaxValues<T>
{
  return {
      .minValue = std::numeric_limits<T>::max(),
      .maxValue = std::numeric_limits<T>::min(),
  };
}

template<typename T>
constexpr auto UpdateMinMaxValues(MinMaxValues<T>& minMaxValues, const T value) noexcept -> void
{
  minMaxValues.minValue = std::min(minMaxValues.minValue, value);
  minMaxValues.maxValue = std::max(minMaxValues.maxValue, value);
}

template<typename T>
constexpr auto GetMidpointOfMinMax(const MinMaxValues<T>& minMaxValues) noexcept -> T
{
  return std::midpoint(minMaxValues.minValue, minMaxValues.maxValue);
}

constexpr auto ToRectangle2dFlt(const Rectangle2dInt& rectangle) noexcept -> Rectangle2dFlt
{
  return {.topLeft     = ToPoint2dFlt(rectangle.topLeft),
          .bottomRight = ToPoint2dFlt(rectangle.bottomRight)};
}

} // namespace GOOM
