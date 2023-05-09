#include "shape_paths.h"

#include "color/color_maps.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "utils/t_values.h"

namespace GOOM::VISUAL_FX::SHAPES
{

using COLOR::ColorMaps;
using COLOR::RandomColorMapsManager;
using DRAW::GetLowColor;
using DRAW::GetMainColor;
using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using UTILS::TValue;
using UTILS::MATH::IPath;

ShapePath::ShapePath(IGoomDraw& draw,
                     const std::shared_ptr<IPath>& path,
                     RandomColorMapsManager& colorMapsManager,
                     const ColorInfo colorInfo) noexcept
  : m_circleDrawer{draw},
    m_path{path},
    m_colorMapsManager{&colorMapsManager},
    m_colorInfo{colorInfo}
{
}

auto ShapePath::Draw(const DrawParams& drawParams) noexcept -> void
{
  const auto point = GetNextPoint();

  const auto shapeColors    = GetCurrentShapeColors();
  const auto& innerColorMap = m_colorMapsManager->GetColorMap(m_colorInfo.innerColorMapId);

  auto innerColorT = TValue{
      {TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(drawParams.maxRadius - 1)}
  };

  // clang-format off
  static constexpr auto MIN_BRIGHTNESS = 0.5F;
  static constexpr auto MAX_BRIGHTNESS = 4.0F;
  auto brightnessT = TValue{
      {TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(drawParams.maxRadius)}};
  // clang-format on

  m_colorAdjust.SetChromaFactor(m_colorInfo.chromaFactor);
  const auto innerColorCutoffRadius = GetInnerColorCutoffRadius(drawParams.maxRadius);

  for (auto radius = drawParams.maxRadius; radius > 1; --radius)
  {
    const auto brightness = drawParams.brightnessAttenuation *
                            STD20::lerp(MIN_BRIGHTNESS, MAX_BRIGHTNESS, brightnessT());
    const auto innerColor = innerColorMap.GetColor(innerColorT());
    const auto colors =
        GetColors(drawParams, radius, brightness, shapeColors, innerColorCutoffRadius, innerColor);

    m_circleDrawer.DrawCircle(point, radius, colors);

    brightnessT.Increment();
    innerColorT.Increment();
  }
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
      m_colorMapsManager->GetColorMap(m_colorInfo.mainColorMapId).GetColor(GetCurrentT()),
      m_colorMapsManager->GetColorMap(m_colorInfo.lowColorMapId).GetColor(GetCurrentT()),
  };
}

auto ShapePath::GetColors(const DrawParams& drawParams,
                          const int32_t radius,
                          const float brightness,
                          const MultiplePixels& shapeColors,
                          const int32_t innerColorCutoffRadius,
                          const Pixel& innerColor) const noexcept -> MultiplePixels
{
  if (drawParams.firstShapePathAtMeetingPoint)
  {
    return GetFinalMeetingPointColors(drawParams.meetingPointColors, brightness);
  }

  return radius <= innerColorCutoffRadius
             ? GetColorsWithoutInner(brightness, shapeColors)
             : GetColorsWithInner(brightness, shapeColors, innerColor, drawParams.innerColorMix);
}

static constexpr auto MAIN_COLOR_BRIGHTNESS_FACTOR              = 0.5F;
static constexpr auto LOW_COLOR_BRIGHTNESS_FACTOR               = 0.5F;
static constexpr auto LOW_COLOR_BRIGHTNESS_MEETING_POINT_FACTOR = 7.0F;

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
  return {m_colorAdjust.GetAdjustment(brightness, GetMainColor(meetingPointColors)),
          m_colorAdjust.GetAdjustment(LOW_COLOR_BRIGHTNESS_MEETING_POINT_FACTOR * brightness,
                                      GetLowColor(meetingPointColors))};
}

} // namespace GOOM::VISUAL_FX::SHAPES
