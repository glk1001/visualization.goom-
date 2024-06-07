module;

#include <algorithm>
#include <cstdint>
#include <memory>
#include <set>

module Goom.VisualFx.ShapesFx:ShapePaths;

import Goom.Color.ColorAdjustment;
import Goom.Color.ColorMaps;
import Goom.Color.RandomColorMaps;
import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.CircleDrawer;
import Goom.Utils.Graphics.Blend2dUtils;
import Goom.Utils.Math.Paths;
import Goom.Utils.Math.TValues;
import Goom.VisualFx.FxHelper;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;

namespace GOOM::VISUAL_FX::SHAPES
{

class ShapePath
{
public:
  struct ColorInfo
  {
    COLOR::ConstColorMapSharedPtr mainColorMapPtr  = nullptr;
    COLOR::ConstColorMapSharedPtr lowColorMapPtr   = nullptr;
    COLOR::ConstColorMapSharedPtr innerColorMapPtr = nullptr;
  };
  ShapePath(FxHelper& fxHelper,
            const std::shared_ptr<UTILS::MATH::IPath>& path,
            const ColorInfo& colorInfo) noexcept;

  auto UpdateMainColorInfo(const COLOR::WeightedRandomColorMaps& mainColorMaps) noexcept -> void;
  auto UpdateLowColorInfo(const COLOR::WeightedRandomColorMaps& lowColorMaps) noexcept -> void;
  auto UpdateInnerColorInfo(const COLOR::WeightedRandomColorMaps& innerColorMaps) noexcept -> void;

  auto SetNumSteps(uint32_t val) noexcept -> void;
  auto IncrementT() noexcept -> void;
  auto ResetT(float val) noexcept -> void;
  [[nodiscard]] auto HasJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasJustHitEndBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasJustHitAnyBoundary() const noexcept -> bool;
  [[nodiscard]] auto GetNextPoint() const noexcept -> Point2dInt;
  [[nodiscard]] auto GetColorInfo() const noexcept -> const ColorInfo&;
  [[nodiscard]] auto GetCurrentT() const noexcept -> float;

  [[nodiscard]] auto GetIPath() const noexcept -> const UTILS::MATH::IPath&;
  [[nodiscard]] auto GetIPath() noexcept -> UTILS::MATH::IPath&;

  struct DrawParams
  {
    float brightnessAttenuation{};
    bool firstShapePathAtMeetingPoint{};
    int32_t maxRadius{};
    float innerColorMix{};
    DRAW::MultiplePixels meetingPointColors;
  };
  auto Draw(const DrawParams& drawParams) noexcept -> void;

private:
  FxHelper* m_fxHelper;
  DRAW::SHAPE_DRAWERS::CircleDrawer m_circleDrawer;
  std::shared_ptr<UTILS::MATH::IPath> m_path;

  ColorInfo m_colorInfo;
  [[nodiscard]] static auto GetColorMapTypes() noexcept
      -> const std::set<COLOR::RandomColorMaps::ColorMapTypes>&;
  static constexpr auto NUM_COLOR_STEPS = 200U;
  UTILS::MATH::TValue m_innerColorT{
      {UTILS::MATH::TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_COLOR_STEPS}
  };

  [[nodiscard]] static auto GetInnerColorCutoffRadius(int32_t maxRadius) noexcept -> int32_t;
  [[nodiscard]] auto GetCurrentShapeColors() const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetColors(const DrawParams& drawParams,
                               float brightness,
                               const DRAW::MultiplePixels& shapeColors) const noexcept
      -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetColorsWithoutInner(float brightness,
                                           const DRAW::MultiplePixels& shapeColors) const noexcept
      -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetColorsWithInner(float brightness,
                                        const DRAW::MultiplePixels& shapeColors,
                                        const Pixel& innerColor,
                                        float innerColorMix) const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetFinalMeetingPointColors(const DRAW::MultiplePixels& meetingPointColors,
                                                float brightness) const noexcept
      -> DRAW::MultiplePixels;

  static constexpr float GAMMA = 1.3F;
  COLOR::ColorAdjustment m_colorAdjust{
      {GAMMA, COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };
};

} // namespace GOOM::VISUAL_FX::SHAPES

namespace GOOM::VISUAL_FX::SHAPES
{

inline auto ShapePath::SetNumSteps(const uint32_t val) noexcept -> void
{
  m_path->SetNumSteps(val);
}

inline auto ShapePath::IncrementT() noexcept -> void
{
  m_path->IncrementT();
}

inline auto ShapePath::ResetT(const float val) noexcept -> void
{
  m_path->Reset(val);
}

inline auto ShapePath::HasJustHitStartBoundary() const noexcept -> bool
{
  return m_path->GetPositionT().HasJustHitStartBoundary();
}

inline auto ShapePath::HasJustHitEndBoundary() const noexcept -> bool
{
  return m_path->GetPositionT().HasJustHitEndBoundary();
}

inline auto ShapePath::HasJustHitAnyBoundary() const noexcept -> bool
{
  return HasJustHitStartBoundary() || HasJustHitEndBoundary();
}

inline auto ShapePath::GetNextPoint() const noexcept -> Point2dInt
{
  return m_path->GetNextPoint();
}

inline auto ShapePath::GetCurrentT() const noexcept -> float
{
  return m_path->GetPositionT()();
}

inline auto ShapePath::GetIPath() const noexcept -> const UTILS::MATH::IPath&
{
  return *m_path;
}

inline auto ShapePath::GetIPath() noexcept -> UTILS::MATH::IPath&
{
  return *m_path;
}

inline auto ShapePath::GetColorInfo() const noexcept -> const ColorInfo&
{
  return m_colorInfo;
}

inline auto ShapePath::GetColorMapTypes() noexcept
    -> const std::set<COLOR::RandomColorMaps::ColorMapTypes>&
{
  return COLOR::RandomColorMaps::GetAllColorMapsTypes();
}

inline auto ShapePath::UpdateMainColorInfo(
    const COLOR::WeightedRandomColorMaps& mainColorMaps) noexcept -> void
{
  m_colorInfo.mainColorMapPtr = mainColorMaps.GetRandomColorMapSharedPtr(GetColorMapTypes());
}

inline auto ShapePath::UpdateLowColorInfo(
    const COLOR::WeightedRandomColorMaps& lowColorMaps) noexcept -> void
{
  m_colorInfo.lowColorMapPtr = lowColorMaps.GetRandomColorMapSharedPtr(GetColorMapTypes());
}

inline auto ShapePath::UpdateInnerColorInfo(
    const COLOR::WeightedRandomColorMaps& innerColorMaps) noexcept -> void
{
  m_colorInfo.innerColorMapPtr = innerColorMaps.GetRandomColorMapSharedPtr(GetColorMapTypes());
}

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
