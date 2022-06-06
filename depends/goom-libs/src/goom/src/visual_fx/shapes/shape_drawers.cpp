#include "shape_drawers.h"

#include "color/color_adjustment.h"
#include "color/colormaps.h"
#include "draw/goom_draw.h"
#include "shape_parts.h"
#include "shape_paths.h"
#include "shapes.h"
#include "utils/math/goom_rand_base.h"

namespace GOOM::VISUAL_FX::SHAPES
{

using COLOR::ColorAdjustment;
using COLOR::IColorMap;
using COLOR::RandomColorMapsManager;
using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using UTILS::TValue;
using UTILS::MATH::IGoomRand;

class ShapePartDrawer
{
public:
  struct Params
  {
    float brightnessAttenuation;
    bool firstShapePathAtMeetingPoint;
    bool varyDotRadius;
    bool doDotJitter;
    ShapePathColors meetingPointColors;
  };

  ShapePartDrawer(IGoomDraw& draw, const IGoomRand& goomRand, const Params& params) noexcept;

  auto DrawShapePaths(const ShapePart& shapePart) noexcept -> void;

private:
  IGoomDraw& m_draw;
  const IGoomRand& m_goomRand;
  const Params& m_params;

  [[nodiscard]] auto GetMaxDotRadius(const ShapePart& shapePart) const noexcept -> int32_t;
};

class ShapePathDrawer
{
public:
  struct Params
  {
    float brightnessAttenuation;
    bool firstShapePathAtMeetingPoint;
    int32_t maxRadius;
    float innerColorMix;
    ShapePathColors meetingPointColors;
  };

  ShapePathDrawer(IGoomDraw& draw, const ShapePart& parentShapePart, const Params& params) noexcept;

  auto DrawNextShapePathPoint(const ShapePath& shapePath) noexcept -> void;

private:
  IGoomDraw& m_draw;
  const ShapePart& m_parentShapePart;
  const Params& m_params;
  const int32_t m_innerColorCutoffRadius = GetInnerColorCutoffRadius(m_params.maxRadius);
  [[nodiscard]] static auto GetInnerColorCutoffRadius(int32_t maxRadius) noexcept -> int32_t;

  auto DrawShapePathDot(const Point2dInt& centre,
                        const ShapePathColors& shapeColors,
                        const IColorMap& innerColorMap) noexcept -> void;

  [[nodiscard]] auto GetCurrentShapeColors(
      const ShapePath::ColorInfo& shapePathColorInfo) const noexcept -> ShapePathColors;
  [[nodiscard]] auto GetColors(int32_t radius,
                               float brightness,
                               const ShapePathColors& shapeColors,
                               const Pixel& innerColor) const noexcept -> MultiplePixels;
  [[nodiscard]] auto GetColorsWithoutInner(float brightness,
                                           const ShapePathColors& shapeColors) const noexcept
      -> MultiplePixels;
  [[nodiscard]] auto GetColorsWithInner(float brightness,
                                        const ShapePathColors& shapeColors,
                                        const Pixel& innerColor) const noexcept -> MultiplePixels;
  [[nodiscard]] auto GetFinalMeetingPointColors(float brightness) const noexcept -> MultiplePixels;

  static constexpr float GAMMA = 1.3F;
  const ColorAdjustment m_colorAdjust{GAMMA, COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR};
};

ShapeDrawer::ShapeDrawer(IGoomDraw& draw,
                         const IGoomRand& goomRand,
                         RandomColorMapsManager& colorMapsManager) noexcept
  : m_draw{draw}, m_goomRand{goomRand}, m_colorMapsManager{colorMapsManager}
{
}

inline auto ShapeDrawer::GetCurrentMeetingPointColors() const noexcept -> ShapePathColors
{
  return {
      m_colorMapsManager.GetColorMap(m_meetingPointMainColorId).GetColor(m_meetingPointColorsT()),
      m_colorMapsManager.GetColorMap(m_meetingPointLowColorId).GetColor(m_meetingPointColorsT()),
  };
}

auto ShapeDrawer::DrawShapeParts(const Shape& shape) noexcept -> void
{
  const ShapePartDrawer::Params shapePartParams{
      GetBrightnessAttenuation(shape),
      shape.FirstShapePathAtMeetingPoint(),
      m_varyDotRadius,
      m_doDotJitter,
      GetCurrentMeetingPointColors(),
  };
  ShapePartDrawer shapePartDrawer{m_draw, m_goomRand, shapePartParams};

  shape.IterateAllShapeParts([&shapePartDrawer](const ShapePart& shapePart)
                             { shapePartDrawer.DrawShapePaths(shapePart); });

  if (shape.FirstShapePathAtMeetingPoint())
  {
    m_meetingPointColorsT.Increment();
  }
}

inline auto ShapeDrawer::GetBrightnessAttenuation(const Shape& shape) noexcept -> float
{
  if (not shape.FirstShapePathsCloseToMeeting())
  {
    return 1.0F;
  }

  const float distanceFromOne =
      1.0F - shape.GetShapePart(0).GetFirstShapePathTDistanceFromClosestBoundary();

  const float minBrightness = 2.0F / static_cast<float>(shape.GetTotalNumShapePaths());
  static constexpr float EXPONENT = 25.0F;
  return STD20::lerp(1.0F, minBrightness, std::pow(distanceFromOne, EXPONENT));
}

inline ShapePartDrawer::ShapePartDrawer(IGoomDraw& draw,
                                        const IGoomRand& goomRand,
                                        const Params& params) noexcept
  : m_draw{draw}, m_goomRand{goomRand}, m_params{params}
{
}


auto ShapePartDrawer::DrawShapePaths(const ShapePart& shapePart) noexcept -> void
{
  const ShapePathDrawer::Params shapePathParams{
      m_params.brightnessAttenuation, m_params.firstShapePathAtMeetingPoint,
      GetMaxDotRadius(shapePart),     shapePart.GetInnerColorMix(),
      m_params.meetingPointColors,
  };
  ShapePathDrawer shapePathDrawer{m_draw, shapePart, shapePathParams};

  const size_t numShapePaths = shapePart.GetNumShapePaths();
  for (size_t shapePathNum = 0; shapePathNum < numShapePaths; ++shapePathNum)
  {
    shapePathDrawer.DrawNextShapePathPoint(shapePart.GetShapePath(shapePathNum));
  }
}

inline auto ShapePartDrawer::GetMaxDotRadius(const ShapePart& shapePart) const noexcept -> int32_t
{
  int32_t maxRadius = shapePart.GetCurrentShapeDotRadius(m_params.varyDotRadius);

  if (m_params.doDotJitter)
  {
    static constexpr int32_t MAX_RADIUS_JITTER = 3;
    maxRadius += m_goomRand.GetRandInRange(0, MAX_RADIUS_JITTER + 1);
  }

  if (shapePart.AreShapePathsCloseToMeeting())
  {
    const float tDistanceFromOne = shapePart.GetFirstShapePathTDistanceFromClosestBoundary();
    static constexpr float EXTRA_RADIUS = 10.0F;
    static constexpr float EXPONENT = 10.0F;
    maxRadius += static_cast<int32_t>(std::pow(tDistanceFromOne, EXPONENT) * EXTRA_RADIUS);
  }

  return maxRadius;
}

inline ShapePathDrawer::ShapePathDrawer(IGoomDraw& draw,
                                        const ShapePart& parentShapePart,
                                        const Params& params) noexcept
  : m_draw{draw}, m_parentShapePart{parentShapePart}, m_params{params}
{
}

inline auto ShapePathDrawer::GetInnerColorCutoffRadius(const int32_t maxRadius) noexcept -> int32_t
{
  static constexpr int32_t RADIUS_FRAC = 3;
  static constexpr int32_t MIN_CUTOFF = 5;
  return std::max(MIN_CUTOFF, maxRadius / RADIUS_FRAC);
}

inline auto ShapePathDrawer::DrawNextShapePathPoint(const ShapePath& shapePath) noexcept -> void
{
  const Point2dInt point = shapePath.GetNextPoint();
  const ShapePath::ColorInfo& shapePathColorInfo = shapePath.GetColorInfo();

  const ShapePathColors shapeColors = GetCurrentShapeColors(shapePathColorInfo);
  const IColorMap& innerColorMap =
      m_parentShapePart.GetColorMap(shapePathColorInfo.innerColorMapId);

  DrawShapePathDot(point, shapeColors, innerColorMap);
}

inline auto ShapePathDrawer::GetCurrentShapeColors(
    const ShapePath::ColorInfo& shapePathColorInfo) const noexcept -> ShapePathColors
{
  return {m_parentShapePart.GetCurrentColor(shapePathColorInfo.mainColorMapId),
          m_parentShapePart.GetCurrentColor(shapePathColorInfo.lowColorMapId)};
}

auto ShapePathDrawer::DrawShapePathDot(const Point2dInt& centre,
                                       const ShapePathColors& shapeColors,
                                       const IColorMap& innerColorMap) noexcept -> void
{
  TValue innerColorT{TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(m_params.maxRadius - 1)};

  static constexpr float MIN_BRIGHTNESS = 0.5F;
  static constexpr float MAX_BRIGHTNESS = 3.0F;
  TValue brightnessT{TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(m_params.maxRadius)};

  for (int32_t radius = m_params.maxRadius; radius > 1; --radius)
  {
    const float brightness =
        m_params.brightnessAttenuation * STD20::lerp(MIN_BRIGHTNESS, MAX_BRIGHTNESS, brightnessT());
    const Pixel innerColor = innerColorMap.GetColor(innerColorT());
    const MultiplePixels colors = GetColors(radius, brightness, shapeColors, innerColor);

    m_draw.Circle(centre, radius, colors);

    brightnessT.Increment();
    innerColorT.Increment();
  }
}

auto ShapePathDrawer::GetColors(const int32_t radius,
                                const float brightness,
                                const ShapePathColors& shapeColors,
                                const Pixel& innerColor) const noexcept -> MultiplePixels
{
  if (m_params.firstShapePathAtMeetingPoint)
  {
    return GetFinalMeetingPointColors(brightness);
  }

  return radius <= m_innerColorCutoffRadius
             ? GetColorsWithoutInner(brightness, shapeColors)
             : GetColorsWithInner(brightness, shapeColors, innerColor);
}

static constexpr float MAIN_COLOR_BRIGHTNESS_FACTOR = 0.5F;
static constexpr float LOW_COLOR_BRIGHTNESS_FACTOR = 0.5F;

inline auto ShapePathDrawer::GetColorsWithoutInner(
    const float brightness, const ShapePathColors& shapeColors) const noexcept -> MultiplePixels
{
  const Pixel mainColor =
      m_colorAdjust.GetAdjustment(MAIN_COLOR_BRIGHTNESS_FACTOR * brightness, shapeColors.mainColor);
  const Pixel lowColor =
      m_colorAdjust.GetAdjustment(LOW_COLOR_BRIGHTNESS_FACTOR * brightness, shapeColors.lowColor);

  return {mainColor, lowColor};
}

inline auto ShapePathDrawer::GetColorsWithInner(const float brightness,
                                                const ShapePathColors& shapeColors,
                                                const Pixel& innerColor) const noexcept
    -> MultiplePixels
{
  const Pixel mainColor = m_colorAdjust.GetAdjustment(
      MAIN_COLOR_BRIGHTNESS_FACTOR * brightness,
      IColorMap::GetColorMix(shapeColors.mainColor, innerColor, m_params.innerColorMix));
  const Pixel lowColor = m_colorAdjust.GetAdjustment(
      LOW_COLOR_BRIGHTNESS_FACTOR * brightness,
      IColorMap::GetColorMix(shapeColors.lowColor, innerColor, m_params.innerColorMix));

  return {mainColor, lowColor};
}

inline auto ShapePathDrawer::GetFinalMeetingPointColors(const float brightness) const noexcept
    -> MultiplePixels
{
  static constexpr float BRIGHTNESS_FACTOR = 7.0F;
  return {m_colorAdjust.GetAdjustment(brightness, m_params.meetingPointColors.mainColor),
          m_colorAdjust.GetAdjustment(BRIGHTNESS_FACTOR * brightness,
                                      m_params.meetingPointColors.lowColor)};
}

} // namespace GOOM::VISUAL_FX::SHAPES
