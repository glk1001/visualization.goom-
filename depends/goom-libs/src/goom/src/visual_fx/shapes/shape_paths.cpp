#include "shape_paths.h"

#include "../fx_helper.h"
#include "color/color_maps.h"
#include "draw/goom_draw.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "utils/graphics/blend2d_utils.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"

#include <algorithm>
#include <cstdint>
#include <memory>

namespace GOOM::VISUAL_FX::SHAPES
{

using COLOR::ColorMaps;
using DRAW::GetLowColor;
using DRAW::GetMainColor;
using DRAW::MultiplePixels;
using UTILS::GRAPHICS::FillCircleWithGradient;
using UTILS::MATH::IPath;

ShapePath::ShapePath(FxHelper& fxHelper,
                     const std::shared_ptr<IPath>& path,
                     const ColorInfo& colorInfo) noexcept
  : m_fxHelper{&fxHelper}, m_circleDrawer{fxHelper.GetDraw()}, m_path{path}, m_colorInfo{colorInfo}
{
}

auto ShapePath::Draw(const DrawParams& drawParams) noexcept -> void
{
  const auto point = GetNextPoint();

  const auto shapeColors = GetCurrentShapeColors();

  static constexpr auto MAX_BRIGHTNESS = 2.0F;

  const auto radius     = drawParams.maxRadius;
  const auto brightness = drawParams.brightnessAttenuation * MAX_BRIGHTNESS;
  const auto colors     = GetColors(drawParams, brightness, shapeColors);

  FillCircleWithGradient(m_fxHelper->GetBlend2dContexts(), colors, 1.0F, point, radius);

  const auto innerColorCutoffRadius = GetInnerColorCutoffRadius(drawParams.maxRadius);
  const auto& innerColorMap         = m_colorInfo.innerColorMapPtr;
  const auto innerColor             = innerColorMap->GetColor(m_innerColorT());
  const auto innerColorMixed =
      GetColorsWithInner(brightness, shapeColors, innerColor, drawParams.innerColorMix);

  FillCircleWithGradient(
      m_fxHelper->GetBlend2dContexts(), innerColorMixed, 1.0F, point, innerColorCutoffRadius);

  m_innerColorT.Increment();
}

inline auto ShapePath::GetInnerColorCutoffRadius(const int32_t maxRadius) noexcept -> int32_t
{
  static constexpr auto RADIUS_FRAC = 3;
  static constexpr auto MIN_CUTOFF  = 5;
  return std::max(MIN_CUTOFF, maxRadius / RADIUS_FRAC);
}

inline auto ShapePath::GetCurrentShapeColors() const noexcept -> MultiplePixels
{
  return {
      m_colorInfo.mainColorMapPtr->GetColor(GetCurrentT()),
      m_colorInfo.lowColorMapPtr->GetColor(GetCurrentT()),
  };
}

auto ShapePath::GetColors(const DrawParams& drawParams,
                          const float brightness,
                          const MultiplePixels& shapeColors) const noexcept -> MultiplePixels
{
  if (drawParams.firstShapePathAtMeetingPoint)
  {
    return GetFinalMeetingPointColors(drawParams.meetingPointColors, brightness);
  }

  return GetColorsWithoutInner(brightness, shapeColors);
}

static constexpr auto MAIN_COLOR_BRIGHTNESS_FACTOR               = 0.015F;
static constexpr auto LOW_COLOR_BRIGHTNESS_FACTOR                = 1.0F;
static constexpr auto MAIN_COLOR_BRIGHTNESS_MEETING_POINT_FACTOR = 0.1F;
static constexpr auto LOW_COLOR_BRIGHTNESS_MEETING_POINT_FACTOR  = 0.1F;

inline auto ShapePath::GetColorsWithoutInner(const float brightness,
                                             const MultiplePixels& shapeColors) const noexcept
    -> MultiplePixels
{
  return {
      m_colorAdjust.GetAdjustment(MAIN_COLOR_BRIGHTNESS_FACTOR * brightness,
                                  GetMainColor(shapeColors)),
      m_colorAdjust.GetAdjustment(LOW_COLOR_BRIGHTNESS_FACTOR * brightness,
                                  GetLowColor(shapeColors)),
  };
}

inline auto ShapePath::GetColorsWithInner(const float brightness,
                                          const MultiplePixels& shapeColors,
                                          const Pixel& innerColor,
                                          const float innerColorMix) const noexcept
    -> MultiplePixels
{
  return {
      m_colorAdjust.GetAdjustment(
          MAIN_COLOR_BRIGHTNESS_FACTOR * brightness,
          ColorMaps::GetColorMix(GetMainColor(shapeColors), innerColor, innerColorMix)),
      m_colorAdjust.GetAdjustment(
          LOW_COLOR_BRIGHTNESS_FACTOR * brightness,
          ColorMaps::GetColorMix(GetLowColor(shapeColors), innerColor, innerColorMix)),
  };
}

inline auto ShapePath::GetFinalMeetingPointColors(const MultiplePixels& meetingPointColors,
                                                  const float brightness) const noexcept
    -> MultiplePixels
{
  return {m_colorAdjust.GetAdjustment(MAIN_COLOR_BRIGHTNESS_MEETING_POINT_FACTOR * brightness,
                                      GetMainColor(meetingPointColors)),
          m_colorAdjust.GetAdjustment(LOW_COLOR_BRIGHTNESS_MEETING_POINT_FACTOR * brightness,
                                      GetLowColor(meetingPointColors))};
}

} // namespace GOOM::VISUAL_FX::SHAPES
