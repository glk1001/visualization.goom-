module;

#include "goom/goom_config.h"

#include <cstdint>

module Goom.VisualFx.IfsDancersFx:IfsPoints;

import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;
import :IfsTypes;
import :Similitudes;

namespace GOOM::VISUAL_FX::IFS
{

class IfsPoint
{
public:
  IfsPoint(const Point2dInt& point, uint32_t count) noexcept;

  [[nodiscard]] auto GetX() const -> uint32_t { return static_cast<uint32_t>(m_x); }
  [[nodiscard]] auto GetY() const -> uint32_t { return static_cast<uint32_t>(m_y); }
  [[nodiscard]] auto GetColor() const -> Pixel { return m_color; }
  auto SetColor(const Pixel& val) -> void { m_color = val; }
  [[nodiscard]] auto GetCount() const -> uint32_t { return m_count; }
  auto SetCount(const uint32_t val) -> void { m_count = val; }
  [[nodiscard]] auto GetSimi() const -> const Similitude* { return m_simi; }
  auto SetSimi(const Similitude* const simi) -> void { m_simi = simi; }

private:
  int32_t m_x;
  int32_t m_y;
  uint32_t m_count;
  Pixel m_color            = BLACK_PIXEL;
  const Similitude* m_simi = nullptr;
};

} // namespace GOOM::VISUAL_FX::IFS

namespace GOOM::VISUAL_FX::IFS
{

inline IfsPoint::IfsPoint(const Point2dInt& point, const uint32_t count) noexcept
  : m_x{point.x}, m_y{point.y}, m_count{count}
{
  Expects(point.x >= 0);
  Expects(point.y >= 0);
}

} // namespace GOOM::VISUAL_FX::IFS
