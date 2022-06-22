#include "shape_parts.h"

//#undef NO_LOGGING

#include "color/random_color_maps.h"
#include "color/random_color_maps_groups.h"
#include "color/random_color_maps_manager.h"
#include "goom/logging.h"
#include "goom_config.h"
#include "point2d.h"
#include "shape_paths.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"
#include "utils/timer.h"

namespace GOOM::VISUAL_FX::SHAPES
{

using COLOR::RandomColorMapsGroups;
using COLOR::RandomColorMapsManager;
using COLOR::COLOR_DATA::ColorMapName;
using UTILS::Logging;
using UTILS::TValue;
using UTILS::MATH::AngleParams;
using UTILS::MATH::CircleFunction;
using UTILS::MATH::CirclePath;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::IsEven;
using UTILS::MATH::IsOdd;
using UTILS::MATH::Transform2d;
using UTILS::MATH::TransformedPath;
using UTILS::MATH::TWO_PI;

ShapePart::ShapePart(const IGoomRand& goomRand,
                     const PluginInfo& goomInfo,
                     RandomColorMapsManager& colorMapsManager,
                     const Params& params) noexcept
  : m_goomRand{goomRand},
    m_goomInfo{goomInfo},
    m_colorMapsManager{colorMapsManager},
    m_currentTMinMaxLerp{params.tMinMaxLerp},
    m_shapePathsStepSpeed{params.shapePathsMinNumSteps, params.shapePathsMaxNumSteps,
                          params.tMinMaxLerp},
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
  return {RandomColorMapsGroups::MakeSharedAllMapsUnweighted(m_goomRand),
          RandomColorMapsGroups::MakeSharedAllMapsUnweighted(m_goomRand),
          RandomColorMapsGroups::MakeSharedAllMapsUnweighted(m_goomRand),
          m_goomRand.GetRandInRange(MIN_INNER_COLOR_MIX_T, MAX_INNER_COLOR_MIX_T)};
}

auto ShapePart::SetShapePathsTargetPoint(const Point2dInt& targetPoint) -> void
{
  if (m_shapePathsTargetPoint == targetPoint)
  {
    m_needToUpdateTargetPoint = false;
    return;
  }

  m_needToUpdateTargetPoint = true;
  m_newShapePathsTargetPoint = targetPoint;
}

auto ShapePart::SetShapePathsMinMaxNumSteps(const uint32_t shapePathsMinNumSteps,
                                            const uint32_t shapePathsMaxNumSteps) noexcept -> void
{
  m_shapePathsStepSpeed.SetMinMaxNumSteps(shapePathsMinNumSteps, shapePathsMaxNumSteps);
}

auto ShapePart::UpdateShapePathTargets() noexcept -> void
{
  if (not m_needToUpdateTargetPoint)
  {
    return;
  }
  if (not m_shapePaths.at(0).HasJustHitEndBoundary())
  {
    return;
  }

  m_shapePathsTargetPoint = m_newShapePathsTargetPoint;

  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [this](ShapePath& shapePath) { UpdateShapePathTransform(shapePath); });

  ResetTs(0.0F);

  m_needToUpdateTargetPoint = false;
}

inline auto ShapePart::UpdateShapePathTransform(ShapePath& shapePath) const noexcept -> void
{
  auto& basePath = dynamic_cast<TransformedPath&>(shapePath.GetIPath());

  Transform2d newTransform = basePath.GetTransform();
  newTransform.SetTranslation(Vec2dFlt{m_shapePathsTargetPoint.ToFlt()});

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
  return Transform2d{rotate, scale, centre};
}

auto ShapePart::GetRandomizedShapePaths() noexcept -> std::vector<ShapePath>
{
  const uint32_t numShapePaths =
      m_goomRand.GetRandInRange(MIN_NUM_SHAPE_PATHS, m_maxNumShapePaths + 1);

  static constexpr float MIN_MIN_SCALE = 0.9F;
  static constexpr float MAX_MIN_SCALE = 1.0F;
  static constexpr float MIN_MAX_SCALE = 1.0F + UTILS::MATH::SMALL_FLOAT;
  static constexpr float MAX_MAX_SCALE = 1.5F;
  static constexpr float PROB_SCALE_EQUALS_ONE = 0.9F;
  const bool probScaleEqualsOne = m_goomRand.ProbabilityOf(PROB_SCALE_EQUALS_ONE);
  const auto minScale =
      float{probScaleEqualsOne ? 1.0F : m_goomRand.GetRandInRange(MIN_MIN_SCALE, MAX_MIN_SCALE)};
  const auto maxScale =
      float{probScaleEqualsOne ? 1.0F : m_goomRand.GetRandInRange(MIN_MAX_SCALE, MAX_MAX_SCALE)};

  return GetShapePaths(numShapePaths, minScale, maxScale);
}

auto ShapePart::GetShapePaths(const uint32_t numShapePaths,
                              const float minScale,
                              const float maxScale) noexcept -> std::vector<ShapePath>
{

  const Vec2dFlt targetPointFlt{m_shapePathsTargetPoint.ToFlt()};

  static constexpr float MIN_ANGLE = 0.0F;
  static constexpr float MAX_ANGLE = TWO_PI;
  auto stepFraction = TValue{TValue::StepType::SINGLE_CYCLE, numShapePaths};

  const auto radius = float{GetCircleRadius()};
  const auto direction = CircleFunction::Direction{GetCircleDirection()};
  const auto numSteps = uint32_t{m_shapePathsStepSpeed.GetCurrentNumSteps()};

  std::vector<ShapePath> shapePaths{};

  for (uint32_t i = 0; i < numShapePaths; ++i)
  {
    const auto rotate = float{STD20::lerp(MIN_ANGLE, MAX_ANGLE, stepFraction())};
    const auto scale = float{STD20::lerp(minScale, maxScale, stepFraction())};
    const auto circlePath = GetCirclePath(radius, direction, numSteps);

    const auto newTransform = GetTransform2d(targetPointFlt, radius, scale, rotate);
    const auto basePath = std::make_shared<TransformedPath>(circlePath.GetClone(), newTransform);

    const auto colorInfo = MakeShapePathColorInfo();

    shapePaths.emplace_back(basePath, colorInfo);

    static constexpr int32_t CLOSE_ENOUGH = 4;
    UNUSED_FOR_NDEBUG(CLOSE_ENOUGH);
    if (SqDistance(shapePaths.at(i).GetIPath().GetStartPos(), m_shapePathsTargetPoint) >
        CLOSE_ENOUGH)
    {
      LogError("shapePaths.at({}).GetIPath().GetStartPos() = {}, {}", i,
               shapePaths.at(i).GetIPath().GetStartPos().x,
               shapePaths.at(i).GetIPath().GetStartPos().y);
      LogError("m_shapesTargetPoint = {}, {}", m_shapePathsTargetPoint.x,
               m_shapePathsTargetPoint.y);
      LogError("targetPointFlt = {}, {}", targetPointFlt.x, targetPointFlt.y);
      LogError("radius = {}", radius);
      LogError("rotate = {}", rotate);
      LogError("std::cos(rotate) = {}", std::cos(rotate));
      LogError("std::sin(rotate) = {}", std::sin(rotate));
      LogError("scale = {}", scale);
      LogError("numSteps = {}", numSteps);
    }
    assert(SqDistance(shapePaths.at(i).GetIPath().GetStartPos(), m_shapePathsTargetPoint) <=
           CLOSE_ENOUGH);

    stepFraction.Increment();
  }

  return shapePaths;
}

inline auto ShapePart::MakeShapePathColorInfo() noexcept -> ShapePath::ColorInfo
{
  return ShapePath::ColorInfo{
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand),
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand),
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand),
  };
}

inline auto ShapePart::GetCircleRadius() const noexcept -> float
{
  const auto minDimension = static_cast<float>(
      std::min(m_goomInfo.GetScreenInfo().width, m_goomInfo.GetScreenInfo().height));
  const auto minRadius = float{m_minRadiusFraction * minDimension};
  const auto maxRadius = float{m_maxRadiusFraction * minDimension};
  const auto t = static_cast<float>(m_shapePartNum) / static_cast<float>(m_totalNumShapeParts - 1);

  return STD20::lerp(minRadius, maxRadius, t);
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
  auto positionT = std::make_unique<TValue>(TValue::StepType::CONTINUOUS_REVERSIBLE, numSteps);

  const auto params = ShapeFunctionParams{radius, AngleParams{}, direction};

  return CirclePath{std::move(positionT), GetCircleFunction(params)};
}

inline auto ShapePart::GetCircleFunction(const ShapeFunctionParams& params) -> CircleFunction
{
  static constexpr Vec2dFlt CENTRE_POS{0.0F, 0.0F};

  return {CENTRE_POS, params.radius, params.angleParams, params.direction};
}

auto ShapePart::SetWeightedMainColorMaps(
    const std::shared_ptr<const COLOR::RandomColorMaps> weightedMaps) noexcept -> void
{
  m_colorInfo.mainColorMaps = weightedMaps;

  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [this](const ShapePath& shapePath) { shapePath.UpdateMainColorInfo(*this); });

  UpdateMegaColorChangeMode();
}

inline auto ShapePart::UpdateMegaColorChangeMode() -> void
{
  if (not m_megaColorChangeTimer.Finished())
  {
    return;
  }

  static constexpr float PROB_MEGA_COLOR_CHANGE = 0.1F;
  m_megaColorChangeMode = m_goomRand.ProbabilityOf(PROB_MEGA_COLOR_CHANGE);
  if (m_megaColorChangeMode)
  {
    m_megaColorChangeTimer.ResetToZero();
  }
}

auto ShapePart::SetWeightedLowColorMaps(
    const std::shared_ptr<const COLOR::RandomColorMaps> weightedMaps) noexcept -> void
{
  m_colorInfo.lowColorMaps = weightedMaps;

  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [this](const ShapePath& shapePath) { shapePath.UpdateLowColorInfo(*this); });
}

auto ShapePart::SetWeightedInnerColorMaps(
    const std::shared_ptr<const COLOR::RandomColorMaps> weightedMaps) noexcept -> void
{
  m_colorInfo.innerColorMix =
      m_goomRand.GetRandInRange(MIN_INNER_COLOR_MIX_T, MAX_INNER_COLOR_MIX_T);

  m_colorInfo.innerColorMaps = weightedMaps;

  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [this](const ShapePath& shapePath) { shapePath.UpdateInnerColorInfo(*this); });
}

auto ShapePart::UpdateMainColorMapId(
    const COLOR::RandomColorMapsManager::ColorMapId mainColorMapId) noexcept -> void
{
  const std::shared_ptr<const COLOR::RandomColorMaps>& mainColorMaps = m_colorInfo.mainColorMaps;

  m_colorMapsManager.UpdateColorMapInfo(mainColorMapId, {mainColorMaps, COLOR_MAP_TYPES});
}

auto ShapePart::UpdateLowColorMapId(
    const COLOR::RandomColorMapsManager::ColorMapId lowColorMapId) noexcept -> void
{
  const std::shared_ptr<const COLOR::RandomColorMaps>& lowColorMaps = m_colorInfo.lowColorMaps;

  m_colorMapsManager.UpdateColorMapInfo(lowColorMapId, {lowColorMaps, COLOR_MAP_TYPES});
}

auto ShapePart::UpdateInnerColorMapId(
    const COLOR::RandomColorMapsManager::ColorMapId innerColorMapId) noexcept -> void
{
  const std::shared_ptr<const COLOR::RandomColorMaps>& innerColorMaps = m_colorInfo.innerColorMaps;

  m_colorMapsManager.UpdateColorMapInfo(innerColorMapId, {innerColorMaps, COLOR_MAP_TYPES});
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

  m_colorMapsManager.ChangeAllColorMapsNow();
}

inline auto ShapePart::ChangeAllColorMapsNow() noexcept -> void
{
  m_colorMapsManager.ChangeAllColorMapsNow();

  static constexpr float PROB_USE_EXTREME_MAX_DOT_RADIUS = 0.1F;
  m_useExtremeMaxShapeDotRadius = m_goomRand.ProbabilityOf(PROB_USE_EXTREME_MAX_DOT_RADIUS);
}

} // namespace GOOM::VISUAL_FX::SHAPES
