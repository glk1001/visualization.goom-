#pragma once

#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/point2d.h"

#include <cstdint>

namespace GOOM::VISUAL_FX::IFS
{

using Dbl = float;
using Flt = int32_t;

struct FltPoint
{
  Flt x = 0;
  Flt y = 0;
};

inline constexpr auto FIX = 12U;

class Similitude;

class IfsPoint
{
public:
  IfsPoint(const Point2dInt& point, uint32_t count) noexcept;

  [[nodiscard]] auto GetX() const -> uint32_t { return static_cast<uint32_t>(m_x); }
  [[nodiscard]] auto GetY() const -> uint32_t { return static_cast<uint32_t>(m_y); }
  [[nodiscard]] auto GetColor() const -> Pixel { return m_color; }
  void SetColor(const Pixel& val) { m_color = val; }
  [[nodiscard]] auto GetCount() const -> uint32_t { return m_count; }
  void SetCount(const uint32_t val) { m_count = val; }
  [[nodiscard]] auto GetSimi() const -> const Similitude* { return m_simi; }
  void SetSimi(const Similitude* const simi) { m_simi = simi; }

private:
  int32_t m_x;
  int32_t m_y;
  uint32_t m_count;
  Pixel m_color            = BLACK_PIXEL;
  const Similitude* m_simi = nullptr;
};

[[nodiscard]] inline auto MultByUnit(const Dbl x) -> Flt
{
  static constexpr auto UNIT = 1U << FIX;
  return static_cast<Flt>(static_cast<Dbl>(UNIT) * x);
}

[[nodiscard]] inline auto DivByUnit(const Flt x) -> Flt
{
  return x >> FIX; // NOLINT(hicpp-signed-bitwise)
}

[[nodiscard]] inline auto DivBy2Units(const Flt x) -> Flt
{
  return x >> (FIX + 1); // NOLINT(hicpp-signed-bitwise)
}

inline IfsPoint::IfsPoint(const Point2dInt& point, const uint32_t count) noexcept
  : m_x{point.x}, m_y{point.y}, m_count{count}
{
  Expects(point.x >= 0);
  Expects(point.y >= 0);
}

} // namespace GOOM::VISUAL_FX::IFS
