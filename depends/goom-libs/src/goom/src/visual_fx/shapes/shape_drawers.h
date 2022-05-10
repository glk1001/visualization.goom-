#pragma once

#include "color/random_colormaps_manager.h"
#include "utils/t_values.h"

#include <cstdint>

namespace GOOM::DRAW
{
class IGoomDraw;
}
namespace GOOM::UTILS::MATH
{
class IGoomRand;
}

namespace GOOM::VISUAL_FX::SHAPES
{

class Shape;

struct ShapePathColors
{
  Pixel mainColor;
  Pixel lowColor;
};

class ShapeDrawer
{
public:
  ShapeDrawer(DRAW::IGoomDraw& draw,
              const UTILS::MATH::IGoomRand& goomRand,
              COLOR::RandomColorMapsManager& colorMapsManager) noexcept;

  auto SetVaryDotRadius(bool val) -> void;
  auto SetDoDotJitter(bool val) -> void;

  auto DrawShapeParts(const Shape& shape) noexcept -> void;

private:
  DRAW::IGoomDraw& m_draw;
  const UTILS::MATH::IGoomRand& m_goomRand;
  COLOR::RandomColorMapsManager& m_colorMapsManager;

  bool m_varyDotRadius = false;
  bool m_doDotJitter = false;
  [[nodiscard]] static auto GetBrightnessAttenuation(const Shape& shape) noexcept -> float;
  static constexpr uint32_t NUM_MEETING_POINT_COLOR_STEPS = 50;
  UTILS::TValue m_meetingPointColorsT{UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE,
                                      NUM_MEETING_POINT_COLOR_STEPS};

  COLOR::RandomColorMapsManager::ColorMapId m_meetingPointMainColorId{
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand)};
  COLOR::RandomColorMapsManager::ColorMapId m_meetingPointLowColorId{
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand)};
  [[nodiscard]] auto GetCurrentMeetingPointColors() const noexcept -> ShapePathColors;
};

inline auto ShapeDrawer::SetVaryDotRadius(const bool val) -> void
{
  m_varyDotRadius = val;
}

inline auto ShapeDrawer::SetDoDotJitter(const bool val) -> void
{
  m_doDotJitter = val;
}

} // namespace GOOM::VISUAL_FX::SHAPES
