//#undef NO_LOGGING

#include "shape_parts.h"

#include "../fx_helper.h"
#include "color/random_color_maps.h"
#include "color/random_color_maps_groups.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "shape_paths.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/parametric_functions2d.h"
#include "utils/math/paths.h"
#include "utils/math/transform2d.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace GOOM::VISUAL_FX::SHAPES
{

using COLOR::GetUnweightedRandomColorMaps;
using COLOR::WeightedRandomColorMaps;
using UTILS::TValue;
using UTILS::MATH::AngleParams;
using UTILS::MATH::CircleFunction;
using UTILS::MATH::CirclePath;
using UTILS::MATH::IsEven;
using UTILS::MATH::IsOdd;
using UTILS::MATH::Transform2d;
using UTILS::MATH::TransformedPath;
using UTILS::MATH::TWO_PI;

ShapePart::ShapePart(FxHelper& fxHelper,
                     const Params& params,
                     const PixelChannelType defaultAlpha) noexcept
  : m_fxHelper{&fxHelper},
    m_defaultAlpha{defaultAlpha},
    m_currentTMinMaxLerp{params.tMinMaxLerp},
    m_shapePathsStepSpeed{
        {params.shapePathsMinNumSteps, params.shapePathsMaxNumSteps}, params.tMinMaxLerp},
    m_minShapeDotRadius{params.minShapeDotRadius},
    m_maxShapeDotRadius{params.maxShapeDotRadius},
    m_shapePartNum{params.shapePartNum},
    m_maxNumShapePaths{params.maxNumShapePaths},
    m_totalNumShapeParts{params.totalNumShapeParts},
    m_shapePathsTargetPoint{params.shapePathsTargetPoint},
    m_minRadiusFraction{params.minRadiusFraction},
    m_maxRadiusFraction{params.maxRadiusFraction}
{
  Expects(0 < params.totalNumShapeParts);
  Expects(params.shapePartNum < params.totalNumShapeParts);
  Expects(0.0F <= params.minRadiusFraction);
  Expects(params.minRadiusFraction < params.maxRadiusFraction);
  Expects(0.0F <= params.tMinMaxLerp);
  Expects(params.tMinMaxLerp <= 1.0F);
  Expects(1 <= params.minShapeDotRadius);
  Expects(params.minShapeDotRadius <= params.maxShapeDotRadius);
  Expects(MIN_NUM_SHAPE_PATHS <= params.maxNumShapePaths);
  Expects(0 < params.shapePathsMinNumSteps);
  Expects(params.shapePathsMinNumSteps < params.shapePathsMaxNumSteps);
}

auto ShapePart::GetInitialColorInfo() const noexcept -> ColorInfo
{
  return {GetUnweightedRandomColorMaps(m_fxHelper->GetGoomRand(), m_defaultAlpha),
          GetUnweightedRandomColorMaps(m_fxHelper->GetGoomRand(), m_defaultAlpha),
          GetUnweightedRandomColorMaps(m_fxHelper->GetGoomRand(), m_defaultAlpha),
          m_fxHelper->GetGoomRand().GetRandInRange(MIN_INNER_COLOR_MIX_T, MAX_INNER_COLOR_MIX_T)};
}

auto ShapePart::SetShapePathsTargetPoint(const Point2dInt& targetPoint) -> void
{
  if (m_shapePathsTargetPoint == targetPoint)
  {
    m_needToUpdateTargetPoint = false;
    return;
  }

  m_needToUpdateTargetPoint  = true;
  m_newShapePathsTargetPoint = targetPoint;
}

auto ShapePart::SetShapePathsMinMaxNumSteps(
    const MinMaxValues<uint32_t>& minMaxShapePathsNumSteps) noexcept -> void
{
  m_shapePathsStepSpeed.SetMinMaxNumSteps(minMaxShapePathsNumSteps);
}

auto ShapePart::UpdateShapePathTargets() noexcept -> void
{
  m_radiusFractionT.Increment();

  static constexpr auto MIN_MIN_RADIUS_FRACTION = 0.05F;
  static constexpr auto MAX_MIN_RADIUS_FRACTION = 0.2F;
  static constexpr auto MIN_MAX_RADIUS_FRACTION = 0.3F;
  static constexpr auto MAX_MAX_RADIUS_FRACTION = 0.5F;
  m_minRadiusFraction =
      std::lerp(MAX_MIN_RADIUS_FRACTION, MIN_MIN_RADIUS_FRACTION, m_radiusFractionT());
  m_maxRadiusFraction =
      std::lerp(MAX_MAX_RADIUS_FRACTION, MIN_MAX_RADIUS_FRACTION, m_radiusFractionT());

  if (not m_needToUpdateTargetPoint)
  {
    return;
  }
  if (not m_shapePaths.at(0).HasJustHitEndBoundary())
  {
    return;
  }

  m_shapePathsTargetPoint = m_newShapePathsTargetPoint;

  std::for_each(begin(m_shapePaths),
                end(m_shapePaths),
                [this](ShapePath& shapePath) { UpdateShapePathTransform(shapePath); });

  ResetTs(0.0F);

  m_needToUpdateTargetPoint = false;
}

inline auto ShapePart::UpdateShapePathTransform(ShapePath& shapePath) const noexcept -> void
{
  auto& basePath = dynamic_cast<TransformedPath&>(shapePath.GetIPath());

  auto newTransform = basePath.GetTransform();
  newTransform.SetTranslation(ToVec2dFlt(m_shapePathsTargetPoint));

  basePath.SetTransform(newTransform);
}

inline auto ShapePart::GetTransform2d(const Vec2dFlt& targetPoint,
                                      const float radius,
                                      const float scale,
                                      const float rotate) noexcept -> Transform2d
{
  const auto centre = Vec2dFlt{
      targetPoint.x - (scale * radius * std::cos(rotate)),
      targetPoint.y - (scale * radius * std::sin(rotate)),
  };
  return Transform2d{rotate, centre, scale};
}

auto ShapePart::GetRandomizedShapePaths() noexcept -> std::vector<ShapePath>
{
  const auto numShapePaths =
      m_fxHelper->GetGoomRand().GetRandInRange(MIN_NUM_SHAPE_PATHS, m_maxNumShapePaths + 1);

  static constexpr auto MIN_MIN_SCALE         = 0.9F;
  static constexpr auto MAX_MIN_SCALE         = 1.0F;
  static constexpr auto MIN_MAX_SCALE         = 1.0F + UTILS::MATH::SMALL_FLOAT;
  static constexpr auto MAX_MAX_SCALE         = 1.5F;
  static constexpr auto PROB_SCALE_EQUALS_ONE = 0.9F;

  const auto probScaleEqualsOne = m_fxHelper->GetGoomRand().ProbabilityOf(PROB_SCALE_EQUALS_ONE);
  const auto minScale =
      probScaleEqualsOne ? 1.0F
                         : m_fxHelper->GetGoomRand().GetRandInRange(MIN_MIN_SCALE, MAX_MIN_SCALE);
  const auto maxScale =
      probScaleEqualsOne ? 1.0F
                         : m_fxHelper->GetGoomRand().GetRandInRange(MIN_MAX_SCALE, MAX_MAX_SCALE);

  return GetShapePaths(numShapePaths, {minScale, maxScale});
}

auto ShapePart::GetShapePaths(const uint32_t numShapePaths,
                              const MinMaxValues<float>& minMaxValues) noexcept
    -> std::vector<ShapePath>
{
  const auto targetPointFlt = ToVec2dFlt(m_shapePathsTargetPoint);

  static constexpr auto MIN_ANGLE = 0.0F;
  static constexpr auto MAX_ANGLE = TWO_PI;
  auto stepFraction               = TValue{
                    {TValue::StepType::SINGLE_CYCLE, numShapePaths}
  };

  const auto radius    = GetCircleRadius();
  const auto direction = GetCircleDirection();
  const auto numSteps  = m_shapePathsStepSpeed.GetCurrentNumSteps();

  auto shapePaths = std::vector<ShapePath>{};

  for (auto i = 0U; i < numShapePaths; ++i)
  {
    const auto rotate     = std::lerp(MIN_ANGLE, MAX_ANGLE, stepFraction());
    const auto scale      = std::lerp(minMaxValues.minValue, minMaxValues.maxValue, stepFraction());
    const auto circlePath = GetCirclePath(radius, direction, numSteps);

    const auto newTransform = GetTransform2d(targetPointFlt, radius, scale, rotate);
    const auto basePath = std::make_shared<TransformedPath>(circlePath.GetClone(), newTransform);

    const auto colorInfo = GetShapePathColorInfo();

    shapePaths.emplace_back(*m_fxHelper, basePath, colorInfo);

    static constexpr auto CLOSE_ENOUGH = 4;
    if (SqDistance(shapePaths.at(i).GetIPath().GetStartPos(), m_shapePathsTargetPoint) >
        CLOSE_ENOUGH)
    {
      //      LogError(m_fxHelper->GetGoomLogger(),
      //               "shapePaths.at({}).GetIPath().GetStartPos() = {}, {}",
      //               i,
      //               shapePaths.at(i).GetIPath().GetStartPos().x,
      //               shapePaths.at(i).GetIPath().GetStartPos().y);
      //      LogError(m_fxHelper->GetGoomLogger(),
      //               "m_shapesTargetPoint = {}, {}",
      //               m_shapePathsTargetPoint.x,
      //               m_shapePathsTargetPoint.y);
      //      LogError(
      //          m_fxHelper->GetGoomLogger(),
      //          "targetPointFlt = {}, {}", targetPointFlt.x, targetPointFlt.y);
      //      LogError(m_fxHelper->GetGoomLogger(), "radius = {}", radius);
      //      LogError(m_fxHelper->GetGoomLogger(), "rotate = {}", rotate);
      //      LogError(m_fxHelper->GetGoomLogger(), "std::cos(rotate) = {}", std::cos(rotate));
      //      LogError(m_fxHelper->GetGoomLogger(), "std::sin(rotate) = {}", std::sin(rotate));
      //      LogError(m_fxHelper->GetGoomLogger(), "scale = {}", scale);
      //      LogError(m_fxHelper->GetGoomLogger(), "numSteps = {}", numSteps);
    }
    Ensures(SqDistance(shapePaths.at(i).GetIPath().GetStartPos(), m_shapePathsTargetPoint) <=
            CLOSE_ENOUGH);

    stepFraction.Increment();
  }

  return shapePaths;
}

inline auto ShapePart::GetShapePathColorInfo() const noexcept -> ShapePath::ColorInfo
{
  const auto& colorMapTypes = WeightedRandomColorMaps::GetAllColorMapsTypes();

  return ShapePath::ColorInfo{
      m_colorInfo.mainColorMaps.GetRandomColorMapSharedPtr(colorMapTypes),
      m_colorInfo.lowColorMaps.GetRandomColorMapSharedPtr(colorMapTypes),
      m_colorInfo.innerColorMaps.GetRandomColorMapSharedPtr(colorMapTypes),
  };
}

inline auto ShapePart::GetCircleRadius() const noexcept -> float
{
  const auto minDimension = std::min(m_fxHelper->GetDimensions().GetFltWidth(),
                                     m_fxHelper->GetDimensions().GetFltHeight());
  const auto minRadius    = m_minRadiusFraction * minDimension;
  const auto maxRadius    = m_maxRadiusFraction * minDimension;
  const auto t = static_cast<float>(m_shapePartNum) / static_cast<float>(m_totalNumShapeParts - 1);

  return std::lerp(minRadius, maxRadius, t);
}

inline auto ShapePart::GetCircleDirection() const noexcept -> CircleFunction::Direction
{
  if (m_useEvenShapePartNumsForDirection)
  {
    return IsEven(m_shapePartNum) ? CircleFunction::Direction::COUNTER_CLOCKWISE
                                  : CircleFunction::Direction::CLOCKWISE;
  }
  return IsOdd(m_shapePartNum) ? CircleFunction::Direction::COUNTER_CLOCKWISE
                               : CircleFunction::Direction::CLOCKWISE;
}

inline auto ShapePart::GetCirclePath(const float radius,
                                     const CircleFunction::Direction direction,
                                     const uint32_t numSteps) noexcept -> CirclePath
{
  auto positionT = std::make_unique<TValue>(
      TValue::NumStepsProperties{TValue::StepType::CONTINUOUS_REVERSIBLE, numSteps});

  const auto params = ShapeFunctionParams{radius, AngleParams{}, direction};

  return CirclePath{std::move(positionT), GetCircleFunction(params)};
}

inline auto ShapePart::GetCircleFunction(const ShapeFunctionParams& params) -> CircleFunction
{
  static constexpr auto CENTRE_POS = Vec2dFlt{0.0F, 0.0F};

  return {CENTRE_POS, params.radius, params.angleParams, params.direction};
}

auto ShapePart::SetWeightedMainColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept
    -> void
{
  m_colorInfo.mainColorMaps = weightedMaps;
  UpdateShapesMainColorMaps();
}

auto ShapePart::SetWeightedLowColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept
    -> void
{
  m_colorInfo.lowColorMaps = weightedMaps;
  UpdateShapesLowColorMaps();
}

auto ShapePart::SetWeightedInnerColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept
    -> void
{
  m_colorInfo.innerColorMix =
      m_fxHelper->GetGoomRand().GetRandInRange(MIN_INNER_COLOR_MIX_T, MAX_INNER_COLOR_MIX_T);

  m_colorInfo.innerColorMaps = weightedMaps;

  UpdateShapesInnerColorMaps();
}

auto ShapePart::UpdateShapesMainColorMaps() noexcept -> void
{
  std::for_each(begin(m_shapePaths),
                end(m_shapePaths),
                [this](ShapePath& shapePath)
                { shapePath.UpdateMainColorInfo(m_colorInfo.mainColorMaps); });
}

auto ShapePart::UpdateShapesLowColorMaps() noexcept -> void
{
  std::for_each(begin(m_shapePaths),
                end(m_shapePaths),
                [this](ShapePath& shapePath)
                { shapePath.UpdateLowColorInfo(m_colorInfo.lowColorMaps); });
}

auto ShapePart::UpdateShapesInnerColorMaps() noexcept -> void
{
  std::for_each(begin(m_shapePaths),
                end(m_shapePaths),
                [this](ShapePath& shapePath)
                { shapePath.UpdateInnerColorInfo(m_colorInfo.innerColorMaps); });
}

auto ShapePart::ChangeAllShapesColorMapsNow() noexcept -> void
{
  UpdateShapesMainColorMaps();
  UpdateShapesLowColorMaps();
  UpdateShapesInnerColorMaps();
}

auto ShapePart::Start() noexcept -> void
{
  SetRandomizedShapePaths();
  StartMegaColorChangeOnOffTimer();
}

auto ShapePart::GetCurrentShapeDotRadius(const bool varyRadius) const noexcept -> int32_t
{
  if (not varyRadius)
  {
    return m_minShapeDotRadius;
  }

  const auto maxShapeDotRadius =
      m_useExtremeMaxShapeDotRadius ? m_extremeMaxShapeDotRadius : m_maxShapeDotRadius;

  return static_cast<int32_t>(std::lerp(m_minShapeDotRadius, maxShapeDotRadius, m_dotRadiusT()));
}

auto ShapePart::GetFirstShapePathPositionT() const noexcept -> float
{
  if (0 == GetNumShapePaths())
  {
    return 1.0F;
  }

  return GetShapePath(0).GetCurrentT();
}

auto ShapePart::GetFirstShapePathTDistanceFromClosestBoundary() const noexcept -> float
{
  const auto positionT = GetFirstShapePathPositionT();

  if (positionT < UTILS::MATH::HALF)
  {
    return positionT;
  }

  return 1.0F - positionT;
}

auto ShapePart::AreShapePathsCloseToMeeting() const noexcept -> bool
{
  static constexpr auto T_MEETING_CUTOFF = 0.05F;
  const auto positionT                   = GetFirstShapePathPositionT();

  return (T_MEETING_CUTOFF > positionT) || (positionT > (1.0F - T_MEETING_CUTOFF));
}

auto ShapePart::UseFixedShapePathsNumSteps(const float tMinMaxLerp) noexcept -> void
{
  m_currentTMinMaxLerp = tMinMaxLerp;
  m_shapePathsStepSpeed.SetSpeed(m_currentTMinMaxLerp);
  m_dotRadiusStepSpeed.SetSpeed(m_currentTMinMaxLerp);
}

auto ShapePart::UseRandomShapePathsNumSteps() noexcept -> void
{
  m_currentTMinMaxLerp = GetNewRandomMinMaxLerpT(m_fxHelper->GetGoomRand(), m_currentTMinMaxLerp);
  m_shapePathsStepSpeed.SetSpeed(m_currentTMinMaxLerp);
  m_dotRadiusStepSpeed.SetSpeed(m_currentTMinMaxLerp);
}

auto ShapePart::SetShapePathsNumSteps() noexcept -> void
{
  std::for_each(begin(m_shapePaths),
                end(m_shapePaths),
                [this](ShapePath& path)
                { path.SetNumSteps(m_shapePathsStepSpeed.GetCurrentNumSteps()); });

  m_dotRadiusT.SetNumSteps(m_dotRadiusStepSpeed.GetCurrentNumSteps());
}

void ShapePart::DoRandomChanges() noexcept
{
  DoMegaColorChange();

  if (not m_shapePaths.at(0).HasJustHitAnyBoundary())
  {
    return;
  }

  SetRandomizedShapePaths();
  SetShapePathsNumSteps();
  ChangeAllColorMapsNow();
}

inline auto ShapePart::DoMegaColorChange() noexcept -> void
{
  if (not m_megaColorChangeMode)
  {
    return;
  }

  ChangeAllShapesColorMapsNow();
}

inline auto ShapePart::StartMegaColorChangeOnOffTimer() noexcept -> void
{
  m_megaColorChangeOnOffTimer.Reset();
  m_megaColorChangeOnOffTimer.SetActions(
      {[this]() { return SetMegaColorChangeOn(); }, [this]() { return SetMegaColorChangeOff(); }});
  m_megaColorChangeOnOffTimer.StartOffTimer();
}

inline auto ShapePart::SetMegaColorChangeOn() noexcept -> bool
{
  if (static constexpr auto PROB_MEGA_COLOR_CHANGE_ON = 0.1F;
      not m_fxHelper->GetGoomRand().ProbabilityOf(PROB_MEGA_COLOR_CHANGE_ON))
  {
    return false;
  }
  m_megaColorChangeMode = true;
  return true;
}

inline auto ShapePart::SetMegaColorChangeOff() noexcept -> bool
{
  if (static constexpr auto PROB_MEGA_COLOR_CHANGE_OFF = 0.9F;
      not m_fxHelper->GetGoomRand().ProbabilityOf(PROB_MEGA_COLOR_CHANGE_OFF))
  {
    return false;
  }
  m_megaColorChangeMode = false;
  return true;
}

inline auto ShapePart::ChangeAllColorMapsNow() noexcept -> void
{
  ChangeAllShapesColorMapsNow();

  static constexpr auto PROB_USE_EXTREME_MAX_DOT_RADIUS = 0.5F;
  m_useExtremeMaxShapeDotRadius =
      m_fxHelper->GetGoomRand().ProbabilityOf(PROB_USE_EXTREME_MAX_DOT_RADIUS);
}

auto ShapePart::Update() noexcept -> void
{
  IncrementTs();
  UpdateShapePathTargets();
}

inline auto ShapePart::IncrementTs() noexcept -> void
{
  std::for_each(begin(m_shapePaths), end(m_shapePaths), [](ShapePath& path) { path.IncrementT(); });

  m_dotRadiusT.Increment();
  m_megaColorChangeOnOffTimer.Update();
}

auto ShapePart::Draw(const DrawParams& drawParams) noexcept -> void
{
  const auto shapePathParams = ShapePath::DrawParams{
      drawParams.brightnessAttenuation,
      drawParams.firstShapePathAtMeetingPoint,
      GetMaxDotRadius(drawParams.varyDotRadius),
      m_colorInfo.innerColorMix,
      drawParams.meetingPointColors,
  };

  for (auto& shapePath : m_shapePaths)
  {
    shapePath.Draw(shapePathParams);
  }
}

inline auto ShapePart::GetMaxDotRadius(const bool varyRadius) const noexcept -> int32_t
{
  auto maxRadius = GetCurrentShapeDotRadius(varyRadius);

  if (AreShapePathsCloseToMeeting())
  {
    const auto tDistanceFromOne        = GetFirstShapePathTDistanceFromClosestBoundary();
    static constexpr auto EXTRA_RADIUS = 10.0F;
    static constexpr auto EXPONENT     = 2.0F;
    maxRadius += static_cast<int32_t>(std::pow(tDistanceFromOne, EXPONENT) * EXTRA_RADIUS);
  }

  return maxRadius;
}

} // namespace GOOM::VISUAL_FX::SHAPES
