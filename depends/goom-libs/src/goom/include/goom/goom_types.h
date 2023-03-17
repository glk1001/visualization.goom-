#pragma once

#include "goom_config.h"
#include "point2d.h"

#include <cstdint>

namespace GOOM
{

template<typename T>
struct MinMaxValues
{
  T minValue;
  T maxValue;
};

template<typename T>
constexpr auto GetAverage(const MinMaxValues<T> minMaxValues) noexcept -> T
{
  constexpr auto NUM_VALUES = 2;
  return (minMaxValues.minValue + minMaxValues.maxValue) / static_cast<T>(NUM_VALUES);
}

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
  return MidpointFromOrigin({GetIntWidth(), GetIntHeight()});
}

} // namespace GOOM
