#include "shapes_fx.h"

#undef NO_LOGGING

#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "goom_config.h"
#include "goom_plugin_info.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/paths.h"
#include "utils/math/transform2d.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <cassert>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX
{

using COLOR::GetAllMapsUnweighted;
using COLOR::GetBrighterColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using COLOR::RandomColorMapsManager;
using DRAW::IGoomDraw;
using UTILS::Logging;
using UTILS::Timer;
using UTILS::TValue;
using UTILS::MATH::AngleParams;
using UTILS::MATH::CircleFunction;
using UTILS::MATH::CirclePath;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::IPath;
using UTILS::MATH::Transform2d;
using UTILS::MATH::TransformedPath;
using UTILS::MATH::TWO_PI;
using UTILS::MATH::U_HALF;

[[nodiscard]] inline auto GetNewRandomMinMaxLerpT(const IGoomRand& goomRand,
                                                  const float oldTMinMaxLerp) noexcept -> float
{
  static constexpr float SMALL_OFFSET = 0.2F;
  return goomRand.GetRandInRange(std::max(0.0F, -SMALL_OFFSET + oldTMinMaxLerp),
                                 std::min(1.0F, oldTMinMaxLerp + SMALL_OFFSET));
}

class ShapeGroup;

class ShapePath
{
public:
  struct ColorInfo
  {
    RandomColorMapsManager::ColorMapId mainColorMapId{};
    RandomColorMapsManager::ColorMapId lowColorMapId{};
    RandomColorMapsManager::ColorMapId innerColorMapId{};
  };
  ShapePath(std::shared_ptr<IPath> path, ColorInfo colorInfo) noexcept;

  auto UpdateMainColorInfo(ShapeGroup& parentShapeGroup) const noexcept -> void;
  auto UpdateLowColorInfo(ShapeGroup& parentShapeGroup) const noexcept -> void;
  auto UpdateInnerColorInfo(ShapeGroup& parentShapeGroup) const noexcept -> void;

  auto SetStepSize(float val) noexcept -> void;
  auto IncrementT() noexcept -> void;
  auto ResetT(float val) noexcept -> void;
  [[nodiscard]] auto HasJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasJustHitEndBoundary() const noexcept -> bool;
  [[nodiscard]] auto GetNextPoint() const noexcept -> Point2dInt;
  [[nodiscard]] auto GetColorInfo() const noexcept -> const ColorInfo&;
  [[nodiscard]] auto GetCurrentT() const noexcept -> float;

  [[nodiscard]] auto GetIPath() const noexcept -> const IPath&;

private:
  std::shared_ptr<IPath> m_path;
  ColorInfo m_colorInfo;
};

class ShapeGroup
{
public:
  ShapeGroup(const IGoomRand& goomRand,
             const PluginInfo& goomInfo,
             RandomColorMapsManager& colorMapsManager,
             uint32_t groupNum,
             float tMinMaxLerp) noexcept;
  ShapeGroup(const ShapeGroup&) noexcept = delete;
  ShapeGroup(ShapeGroup&&) noexcept = default;
  ~ShapeGroup() noexcept = default;
  auto operator=(const ShapeGroup&) -> ShapeGroup& = delete;
  auto operator=(ShapeGroup&&) -> ShapeGroup& = delete;

  auto SetWeightedMainColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;
  auto SetWeightedLowColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;
  auto SetWeightedInnerColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;

  auto SetShapesTargetPoint(const Point2dInt& targetPoint) -> void;

  auto Start() noexcept -> void;

  auto IncrementTs() noexcept -> void;
  auto ResetTs(float val) noexcept -> void;

  auto DoRandomChanges() noexcept -> void;
  auto UseRandomShapesSpeed() noexcept -> void;
  auto UseFixedShapesSpeed(float tMinMaxLerp) noexcept -> void;

  auto UpdateMainColorMapId(RandomColorMapsManager::ColorMapId mainColorMapId) noexcept -> void;
  auto UpdateLowColorMapId(RandomColorMapsManager::ColorMapId lowColorMapId) noexcept -> void;
  auto UpdateInnerColorMapId(RandomColorMapsManager::ColorMapId innerColorMapId) noexcept -> void;

  [[nodiscard]] auto GetNumShapes() const noexcept -> uint32_t;
  [[nodiscard]] auto GetShapePath(size_t shapeNum) const noexcept -> const ShapePath&;
  [[nodiscard]] auto GetCurrentShapesRadius() const noexcept -> int32_t;

  [[nodiscard]] auto GetColorMap(RandomColorMapsManager::ColorMapId colorMapId) const noexcept
      -> const IColorMap&;
  [[nodiscard]] auto GetCurrentColor(RandomColorMapsManager::ColorMapId colorMapId) const noexcept
      -> Pixel;
  [[nodiscard]] auto GetInnerColorMix() const noexcept -> float;

private:
  const IGoomRand& m_goomRand;
  const PluginInfo& m_goomInfo;
  Point2dInt m_shapesTargetPoint{U_HALF * m_goomInfo.GetScreenInfo().width,
                                 U_HALF* m_goomInfo.GetScreenInfo().height};
  const uint32_t m_groupNum;
  static constexpr float MIN_SHAPE_SPEED = 0.005F;
  static_assert((0.0F < MIN_SHAPE_SPEED) and (MIN_SHAPE_SPEED < 1.0F));
  static constexpr float MAX_SHAPE_SPEED = 0.020F;
  static_assert((0.0F < MAX_SHAPE_SPEED) and (MAX_SHAPE_SPEED < 1.0F));
  bool m_useRandomShapesSpeed = false;
  float m_currentTMinMaxLerp = 0.0F;
  float m_fixedTMinMaxLerp = 0.0F;
  auto SetShapesSpeed() noexcept -> void;
  [[nodiscard]] static auto GetShapesSpeed(float tMinMaxLerp) noexcept -> float;

  static constexpr int32_t MIN_SHAPE_RADIUS = 10;
  static constexpr int32_t MAX_SHAPE_RADIUS = 20;
  static constexpr uint32_t MIN_RADIUS_STEPS = 100;
  static constexpr uint32_t MAX_RADIUS_STEPS = 300;
  static constexpr uint32_t INITIAL_RADIUS_STEPS = 200;
  TValue m_radiusT{TValue::StepType::CONTINUOUS_REVERSIBLE, INITIAL_RADIUS_STEPS};

  static inline const std::set<RandomColorMaps::ColorMapTypes> COLOR_MAP_TYPES =
      RandomColorMaps::ALL_COLOR_MAP_TYPES;
  static constexpr float MIN_INNER_COLOR_MIX_T = 0.1F;
  static constexpr float MAX_INNER_COLOR_MIX_T = 0.9F;
  RandomColorMapsManager& m_colorMapsManager;
  struct ColorInfo
  {
    std::shared_ptr<RandomColorMaps> mainColorMaps;
    COLOR::COLOR_DATA::ColorMapName mainColormapName;
    std::shared_ptr<RandomColorMaps> lowColorMaps;
    COLOR::COLOR_DATA::ColorMapName lowColormapName;
    std::shared_ptr<RandomColorMaps> innerColorMaps;
    COLOR::COLOR_DATA::ColorMapName innerColormapName;
    float innerColorMix;
  };
  [[nodiscard]] auto GetInitialColorInfo() const noexcept -> ColorInfo;
  ColorInfo m_colorInfo{GetInitialColorInfo()};
  bool m_useRandomColorNames = false;

  static constexpr float MIN_COLOR_MAP_SPEED = 0.01F;
  static_assert((0.0F < MIN_COLOR_MAP_SPEED) and (MIN_COLOR_MAP_SPEED < 1.0F));
  static constexpr float MAX_COLOR_MAP_SPEED = 0.1F;
  static_assert((0.0F < MAX_COLOR_MAP_SPEED) and (MAX_COLOR_MAP_SPEED < 1.0F));
  TValue m_allColorsT;
  auto ChangeAllColorsT() noexcept -> void;

  std::vector<ShapePath> m_shapePaths{};
  auto ChangeAllColorMapsNow() noexcept -> void;
  auto SetRandomizedShapePaths() noexcept -> void;
  [[nodiscard]] auto GetRandomizedShapePaths() noexcept -> std::vector<ShapePath>;
  struct ShapeFunctionParams
  {
    float radius;
    AngleParams angleParams;
    CircleFunction::Direction direction;
  };
  [[nodiscard]] auto GetCircleRadius() const noexcept -> float;
  [[nodiscard]] auto GetCircleDirection() const noexcept -> CircleFunction::Direction;
  [[nodiscard]] auto MakeShapePathColorInfo() noexcept -> ShapePath::ColorInfo;
  [[nodiscard]] static auto GetCirclePath(float radius,
                                          CircleFunction::Direction direction,
                                          float speed) noexcept -> CirclePath;
  [[nodiscard]] static auto GetCircleFunction(const ShapeFunctionParams& params) -> CircleFunction;
};

static_assert(std::is_nothrow_move_constructible_v<ShapeGroup>);

ShapeGroup::ShapeGroup(const IGoomRand& goomRand,
                       const PluginInfo& goomInfo,
                       RandomColorMapsManager& colorMapsManager,
                       const uint32_t groupNum,
                       const float tMinMaxLerp) noexcept
  : m_goomRand{goomRand},
    m_goomInfo{goomInfo},
    m_groupNum{groupNum},
    m_colorMapsManager{colorMapsManager},
    m_allColorsT{TValue::StepType::CONTINUOUS_REVERSIBLE, GetShapesSpeed(tMinMaxLerp)}
{
}

auto ShapeGroup::GetInitialColorInfo() const noexcept -> ColorInfo
{
  return {GetAllMapsUnweighted(m_goomRand),
          COLOR::COLOR_DATA::ColorMapName::_NULL,
          GetAllMapsUnweighted(m_goomRand),
          COLOR::COLOR_DATA::ColorMapName::_NULL,
          GetAllMapsUnweighted(m_goomRand),
          COLOR::COLOR_DATA::ColorMapName::_NULL,
          m_goomRand.GetRandInRange(MIN_INNER_COLOR_MIX_T, MAX_INNER_COLOR_MIX_T)};
}

inline auto ShapeGroup::Start() noexcept -> void
{
  SetRandomizedShapePaths();
}

inline auto ShapeGroup::GetRandomizedShapePaths() noexcept -> std::vector<ShapePath>
{
  static constexpr uint32_t MIN_NUM_SHAPES = 4;
  static constexpr uint32_t MAX_NUM_SHAPES = 6;
  const uint32_t numShapes = m_goomRand.GetRandInRange(MIN_NUM_SHAPES, MAX_NUM_SHAPES + 1);

  const Vec2dFlt targetPointFlt{m_shapesTargetPoint.ToFlt()};

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

  static constexpr float MIN_ANGLE = 0.0F;
  static constexpr float MAX_ANGLE = TWO_PI;
  auto stepFraction = TValue{TValue::StepType::SINGLE_CYCLE, numShapes};

  const auto radius = float{GetCircleRadius()};
  const auto direction = CircleFunction::Direction{GetCircleDirection()};
  const auto speed = float{GetShapesSpeed(m_fixedTMinMaxLerp)};

  std::vector<ShapePath> shapePaths{};

  for (uint32_t i = 0; i < numShapes; ++i)
  {
    const auto rotate = float{STD20::lerp(MIN_ANGLE, MAX_ANGLE, stepFraction())};
    const auto scale = float{STD20::lerp(minScale, maxScale, stepFraction())};
    const auto centre = Vec2dFlt{
        targetPointFlt.x - (scale * radius * std::cos(rotate)),
        targetPointFlt.y - (scale * radius * std::sin(rotate)),
    };
    const auto transform = Transform2d{rotate, scale, centre};
    const auto colorInfo = MakeShapePathColorInfo();
    const auto circlePath = GetCirclePath(radius, direction, speed);
    const auto newBasePath = std::make_shared<TransformedPath>(circlePath.GetClone(), transform);

    shapePaths.emplace_back(newBasePath, colorInfo);

    assert(shapePaths.at(i).GetIPath().GetStartPos() == m_shapesTargetPoint);

    stepFraction.Increment();
  }

  return shapePaths;
}

inline auto ShapeGroup::MakeShapePathColorInfo() noexcept -> ShapePath::ColorInfo
{
  return ShapePath::ColorInfo{
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand),
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand),
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand),
  };
}

inline auto ShapeGroup::GetCircleRadius() const noexcept -> float
{
  const float minRadius = 0.20F * static_cast<float>(m_goomInfo.GetScreenInfo().height);
  const float maxRadius = 0.67F * static_cast<float>(m_goomInfo.GetScreenInfo().height);
  const float t =
      static_cast<float>(m_groupNum) / static_cast<float>(ShapesFx::NUM_SHAPE_GROUPS - 1);

  return STD20::lerp(minRadius, maxRadius, t);
}

inline auto ShapeGroup::GetCircleDirection() const noexcept -> CircleFunction::Direction
{
  return UTILS::MATH::IsEven(m_groupNum) ? CircleFunction::Direction::COUNTER_CLOCKWISE
                                         : CircleFunction::Direction::CLOCKWISE;
}

inline auto ShapeGroup::GetCirclePath(const float radius,
                                      const CircleFunction::Direction direction,
                                      const float speed) noexcept -> CirclePath
{
  auto positionT = std::make_unique<TValue>(TValue::StepType::CONTINUOUS_REVERSIBLE, speed);

  const auto params = ShapeFunctionParams{radius, AngleParams{}, direction};

  return CirclePath{std::move(positionT), GetCircleFunction(params)};
}

inline auto ShapeGroup::GetCircleFunction(const ShapeFunctionParams& params) -> CircleFunction
{
  static constexpr Vec2dFlt CENTRE_POS{0.0F, 0.0F};

  return {CENTRE_POS, params.radius, params.angleParams, params.direction};
}

inline auto ShapeGroup::GetNumShapes() const noexcept -> uint32_t
{
  return static_cast<uint32_t>(m_shapePaths.size());
}

inline auto ShapeGroup::GetShapePath(const size_t shapeNum) const noexcept -> const ShapePath&
{
  return m_shapePaths.at(shapeNum);
}

inline auto ShapeGroup::GetColorMap(
    const RandomColorMapsManager::ColorMapId colorMapId) const noexcept -> const IColorMap&
{
  return m_colorMapsManager.GetColorMap(colorMapId);
}

inline auto ShapeGroup::GetCurrentColor(
    const RandomColorMapsManager::ColorMapId colorMapId) const noexcept -> Pixel
{
  return GetColorMap(colorMapId).GetColor(m_allColorsT());
}

inline auto ShapeGroup::GetInnerColorMix() const noexcept -> float
{
  return m_colorInfo.innerColorMix;
}

auto ShapeGroup::GetCurrentShapesRadius() const noexcept -> int32_t
{
  return STD20::lerp(MIN_SHAPE_RADIUS, MAX_SHAPE_RADIUS, m_radiusT());
}

inline auto ShapeGroup::SetWeightedMainColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void
{
  m_colorInfo.mainColorMaps = weightedMaps;
  m_colorInfo.mainColormapName =
      weightedMaps->GetRandomColorMapName(weightedMaps->GetRandomGroup());

  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [this](const ShapePath& shapePath) { shapePath.UpdateMainColorInfo(*this); });
}

inline auto ShapeGroup::SetWeightedLowColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void
{
  m_colorInfo.lowColorMaps = weightedMaps;
  m_colorInfo.lowColormapName = weightedMaps->GetRandomColorMapName(weightedMaps->GetRandomGroup());

  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [this](const ShapePath& shapePath) { shapePath.UpdateLowColorInfo(*this); });
}

inline auto ShapeGroup::SetWeightedInnerColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void
{
  m_colorInfo.innerColorMix =
      m_goomRand.GetRandInRange(MIN_INNER_COLOR_MIX_T, MAX_INNER_COLOR_MIX_T);

  m_colorInfo.innerColorMaps = weightedMaps;
  m_colorInfo.innerColormapName =
      weightedMaps->GetRandomColorMapName(weightedMaps->GetRandomGroup());

  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [this](const ShapePath& shapePath) { shapePath.UpdateInnerColorInfo(*this); });
}

inline auto ShapeGroup::SetShapesTargetPoint(const Point2dInt& targetPoint) -> void
{
  m_shapesTargetPoint = targetPoint;
}

inline auto ShapeGroup::UpdateMainColorMapId(
    const RandomColorMapsManager::ColorMapId mainColorMapId) noexcept -> void
{
  const std::shared_ptr<RandomColorMaps>& mainColorMaps = m_colorInfo.mainColorMaps;

  const COLOR::COLOR_DATA::ColorMapName colormapName =
      not m_useRandomColorNames
          ? m_colorInfo.mainColormapName
          : mainColorMaps->GetRandomColorMapName(mainColorMaps->GetRandomGroup());

  m_colorMapsManager.UpdateColorMapInfo(mainColorMapId,
                                        {mainColorMaps, colormapName, COLOR_MAP_TYPES});
}

inline auto ShapeGroup::UpdateLowColorMapId(
    const RandomColorMapsManager::ColorMapId lowColorMapId) noexcept -> void
{
  const std::shared_ptr<RandomColorMaps>& lowColorMaps = m_colorInfo.lowColorMaps;

  const COLOR::COLOR_DATA::ColorMapName colormapName =
      not m_useRandomColorNames
          ? m_colorInfo.lowColormapName
          : lowColorMaps->GetRandomColorMapName(lowColorMaps->GetRandomGroup());

  m_colorMapsManager.UpdateColorMapInfo(lowColorMapId,
                                        {lowColorMaps, colormapName, COLOR_MAP_TYPES});
}

inline auto ShapeGroup::UpdateInnerColorMapId(
    const RandomColorMapsManager::ColorMapId innerColorMapId) noexcept -> void
{
  const std::shared_ptr<RandomColorMaps>& innerColorMaps = m_colorInfo.innerColorMaps;

  const COLOR::COLOR_DATA::ColorMapName colormapName =
      not m_useRandomColorNames
          ? m_colorInfo.innerColormapName
          : innerColorMaps->GetRandomColorMapName(innerColorMaps->GetRandomGroup());

  m_colorMapsManager.UpdateColorMapInfo(innerColorMapId,
                                        {innerColorMaps, colormapName, COLOR_MAP_TYPES});
}

inline auto ShapeGroup::IncrementTs() noexcept -> void
{
  std::for_each(begin(m_shapePaths), end(m_shapePaths), [](ShapePath& path) { path.IncrementT(); });

  m_allColorsT.Increment();
  m_radiusT.Increment();
}

inline auto ShapeGroup::ResetTs(const float val) noexcept -> void
{
  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [&val](ShapePath& path) { path.ResetT(val); });
}

inline auto ShapeGroup::UseRandomShapesSpeed() noexcept -> void
{
  m_useRandomShapesSpeed = true;
}

inline auto ShapeGroup::UseFixedShapesSpeed(const float tMinMaxLerp) noexcept -> void
{
  m_fixedTMinMaxLerp = tMinMaxLerp;
  m_useRandomShapesSpeed = false;
}

inline void ShapeGroup::DoRandomChanges() noexcept
{
  if ((not m_shapePaths.at(0).HasJustHitStartBoundary()) and
      (not m_shapePaths.at(0).HasJustHitEndBoundary()))
  {
    return;
  }

  SetRandomizedShapePaths();
  SetShapesSpeed();
  ChangeAllColorMapsNow();
  ChangeAllColorsT();
}

inline auto ShapeGroup::ChangeAllColorMapsNow() noexcept -> void
{
  static constexpr float PROB_USE_RANDOM_COLOR_NAMES = 0.2F;
  m_useRandomColorNames = m_goomRand.ProbabilityOf(PROB_USE_RANDOM_COLOR_NAMES);
  m_colorMapsManager.ChangeAllColorMapsNow();
}

inline auto ShapeGroup::ChangeAllColorsT() noexcept -> void
{
  const float t = m_goomRand.GetRandInRange(0.0F, 1.0F);
  m_allColorsT.SetStepSize(STD20::lerp(MIN_COLOR_MAP_SPEED, MAX_COLOR_MAP_SPEED, t));
}

inline auto ShapeGroup::SetRandomizedShapePaths() noexcept -> void
{
  m_shapePaths = GetRandomizedShapePaths();
}

inline auto ShapeGroup::SetShapesSpeed() noexcept -> void
{
  m_currentTMinMaxLerp = m_useRandomShapesSpeed
                             ? GetNewRandomMinMaxLerpT(m_goomRand, m_currentTMinMaxLerp)
                             : m_fixedTMinMaxLerp;
  const float newSpeed = GetShapesSpeed(m_currentTMinMaxLerp);
  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [&newSpeed](ShapePath& path) { path.SetStepSize(newSpeed); });

  m_radiusT.SetNumSteps(STD20::lerp(MIN_RADIUS_STEPS, MAX_RADIUS_STEPS, m_currentTMinMaxLerp));
}

inline auto ShapeGroup::GetShapesSpeed(const float tMinMaxLerp) noexcept -> float
{
  return STD20::lerp(MIN_SHAPE_SPEED, MAX_SHAPE_SPEED, tMinMaxLerp);
}

inline ShapePath::ShapePath(const std::shared_ptr<IPath> path, const ColorInfo colorInfo) noexcept
  : m_path{path}, m_colorInfo{colorInfo}
{
}

inline auto ShapePath::SetStepSize(const float val) noexcept -> void
{
  m_path->SetStepSize(val);
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

inline auto ShapePath::GetNextPoint() const noexcept -> Point2dInt
{
  return m_path->GetNextPoint();
}

auto ShapePath::GetCurrentT() const noexcept -> float
{
  return m_path->GetPositionT()();
}

auto ShapePath::GetIPath() const noexcept -> const IPath&
{
  return *m_path;
}

inline auto ShapePath::GetColorInfo() const noexcept -> const ColorInfo&
{
  return m_colorInfo;
}

inline auto ShapePath::UpdateMainColorInfo(ShapeGroup& parentShapeGroup) const noexcept -> void
{
  parentShapeGroup.UpdateMainColorMapId(m_colorInfo.mainColorMapId);
}

inline auto ShapePath::UpdateLowColorInfo(ShapeGroup& parentShapeGroup) const noexcept -> void
{
  parentShapeGroup.UpdateLowColorMapId(m_colorInfo.lowColorMapId);
}

inline auto ShapePath::UpdateInnerColorInfo(ShapeGroup& parentShapeGroup) const noexcept -> void
{
  parentShapeGroup.UpdateInnerColorMapId(m_colorInfo.innerColorMapId);
}

class ShapesFx::ShapesFxImpl
{
public:
  explicit ShapesFxImpl(const FxHelper& fxHelper) noexcept;

  auto SetWeightedMainColorMaps(size_t shapeGroupNum,
                                std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;
  auto SetWeightedLowColorMaps(size_t shapeGroupNum,
                               std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;
  auto SetWeightedInnerColorMaps(size_t shapeGroupNum,
                                 std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;

  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) -> void;

  auto Start() noexcept -> void;
  auto ApplyMultiple() noexcept -> void;

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  RandomColorMapsManager m_colorMapsManager{};
  static constexpr uint32_t TIME_BEFORE_SYNCHRONISED_CHANGE = 5000;
  Timer m_synchronisedShapeChangesTimer{TIME_BEFORE_SYNCHRONISED_CHANGE};

  [[nodiscard]] auto AllColorMapsValid() const noexcept -> bool;

  [[nodiscard]] auto GetInitialShapeGroups() noexcept -> std::vector<ShapeGroup>;
  std::vector<ShapeGroup> m_shapeGroups{GetInitialShapeGroups()};
  [[nodiscard]] auto GetFirstShapePositionT() const noexcept -> float;
  [[nodiscard]] auto GetFirstShapeTDistanceFromClosestBoundary() const noexcept -> float;
  [[nodiscard]] auto HasFirstShapeJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasFirstShapeJustHitEndBoundary() const noexcept -> bool;
  [[nodiscard]] auto FirstShapeAtMeetingPoint() const noexcept -> bool;
  auto DoChanges() noexcept -> void;
  auto DoRandomChanges() noexcept -> void;
  auto SetShapeSpeeds() noexcept -> void;
  auto SetFixedShapeSpeeds() noexcept -> void;
  auto SetRandomShapeSpeeds() noexcept -> void;
  static constexpr float STARTING_FIXED_T_MIN_MAX_LERP = 0.5F;
  float m_fixedTMinMaxLerp = GetNewRandomMinMaxLerpT(m_goomRand, STARTING_FIXED_T_MIN_MAX_LERP);
  float m_brightnessAttenuation = 1.0F;
  auto SetBrightnessAttenuation() noexcept -> void;
  auto DrawShapeGroups() noexcept -> void;
  auto DrawShapes(const ShapeGroup& shapeGroup) noexcept -> void;
  auto DrawShape(const ShapeGroup& shapeGroup, size_t shapeNum) noexcept -> void;
  [[nodiscard]] auto GetMaxRadius(const ShapeGroup& shapeGroup) const noexcept -> int32_t;
  static constexpr float T_MEETING_CUTOFF = 0.1F;
  [[nodiscard]] auto AreShapesCloseToMeeting() const noexcept -> bool;
  struct ShapeColors
  {
    Pixel mainColor;
    Pixel lowColor;
  };
  RandomColorMapsManager::ColorMapId m_meetingPointMainColorId{
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand)};
  RandomColorMapsManager::ColorMapId m_meetingPointLowColorId{
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand)};
  static constexpr uint32_t NUM_MEETING_POINT_COLOR_STEPS = 50;
  TValue m_meetingPointColorsT{TValue::StepType::CONTINUOUS_REVERSIBLE,
                               NUM_MEETING_POINT_COLOR_STEPS};
  [[nodiscard]] auto GetCurrentMeetingPointColors() const noexcept -> ShapeColors;
  auto DrawCircleShape(const Point2dInt& centre,
                       int32_t maxRadius,
                       const ShapeColors& shapeColors,
                       const IColorMap& innerColorMap,
                       float innerColorMix) noexcept -> void;
  [[nodiscard]] static auto GetInnerColorCutoffRadius(int32_t maxRadius) noexcept -> int32_t;
  [[nodiscard]] static auto GetCurrentShapeColors(
      const ShapeGroup& shapeGroup, const ShapePath::ColorInfo& shapePathColorInfo) noexcept
      -> ShapeColors;
  [[nodiscard]] auto GetColors(int32_t radius,
                               int32_t innerColorCutoffRadius,
                               float brightness,
                               const ShapeColors& meetingPointColors,
                               const ShapeColors& shapeColors,
                               const Pixel& innerColor,
                               float innerColorMix) const noexcept -> std::vector<Pixel>;
  [[nodiscard]] static auto GetColorsWithoutInner(float brightness,
                                                  const ShapeColors& shapeColors) noexcept
      -> std::vector<Pixel>;
  [[nodiscard]] static auto GetColorsWithInner(float brightness,
                                               const ShapeColors& shapeColors,
                                               const Pixel& innerColor,
                                               float innerColorMix) noexcept -> std::vector<Pixel>;
  [[nodiscard]] static auto GetFinalMeetingPointColors(
      float brightness, const ShapeColors& meetingPointColors) noexcept -> std::vector<Pixel>;
};

ShapesFx::ShapesFx(const FxHelper& fxHelper) noexcept
  : m_fxImpl{spimpl::make_unique_impl<ShapesFxImpl>(fxHelper)}
{
}

auto ShapesFx::GetFxName() const noexcept -> std::string
{
  return "shapes";
}

auto ShapesFx::SetWeightedMainColorMaps(
    const size_t shapeGroupNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  m_fxImpl->SetWeightedMainColorMaps(shapeGroupNum, weightedMaps);
}

auto ShapesFx::SetWeightedLowColorMaps(const size_t shapeGroupNum,
                                       const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  m_fxImpl->SetWeightedLowColorMaps(shapeGroupNum, weightedMaps);
}

auto ShapesFx::SetWeightedInnerColorMaps(
    const size_t shapeGroupNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  m_fxImpl->SetWeightedInnerColorMaps(shapeGroupNum, weightedMaps);
}

auto ShapesFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) -> void
{
  m_fxImpl->SetZoomMidpoint(zoomMidpoint);
}

auto ShapesFx::Start() noexcept -> void
{
  m_fxImpl->Start();
}

auto ShapesFx::Finish() noexcept -> void
{
  // nothing to do
}

auto ShapesFx::ApplyMultiple() noexcept -> void
{
  m_fxImpl->ApplyMultiple();
}

ShapesFx::ShapesFxImpl::ShapesFxImpl(const FxHelper& fxHelper) noexcept
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()}
{
}

auto ShapesFx::ShapesFxImpl::GetInitialShapeGroups() noexcept -> std::vector<ShapeGroup>
{
  std::vector<ShapeGroup> shapeGroups{};

  for (uint32_t i = 0; i < NUM_SHAPE_GROUPS; ++i)
  {
    static constexpr float T_MIN_MAX_LERP = 0.5F;
    shapeGroups.emplace_back(m_goomRand, m_goomInfo, m_colorMapsManager, i, T_MIN_MAX_LERP);
  }

  return shapeGroups;
}

inline auto ShapesFx::ShapesFxImpl::GetFirstShapePositionT() const noexcept -> float
{
  if (0 == m_shapeGroups.front().GetNumShapes())
  {
    return 1.0F;
  }

  return m_shapeGroups.front().GetShapePath(0).GetCurrentT();
}

inline auto ShapesFx::ShapesFxImpl::HasFirstShapeJustHitStartBoundary() const noexcept -> bool
{
  if (0 == m_shapeGroups.front().GetNumShapes())
  {
    return false;
  }

  return m_shapeGroups.front().GetShapePath(0).HasJustHitStartBoundary();
}

inline auto ShapesFx::ShapesFxImpl::HasFirstShapeJustHitEndBoundary() const noexcept -> bool
{
  if (0 == m_shapeGroups.front().GetNumShapes())
  {
    return false;
  }

  return m_shapeGroups.front().GetShapePath(0).HasJustHitEndBoundary();
}

inline auto ShapesFx::ShapesFxImpl::FirstShapeAtMeetingPoint() const noexcept -> bool
{
  return HasFirstShapeJustHitStartBoundary() || HasFirstShapeJustHitEndBoundary();
}

inline auto ShapesFx::ShapesFxImpl::GetFirstShapeTDistanceFromClosestBoundary() const noexcept
    -> float
{
  const float positionT = GetFirstShapePositionT();

  if (positionT < UTILS::MATH::HALF)
  {
    return positionT;
  }

  return 1.0F - positionT;
}

auto ShapesFx::ShapesFxImpl::AllColorMapsValid() const noexcept -> bool
{
  for (const auto& shapeGroup : m_shapeGroups)
  {
    for (size_t shapeNum = 0; shapeNum < shapeGroup.GetNumShapes(); ++shapeNum)
    {
      assert(shapeGroup.GetShapePath(shapeNum).GetColorInfo().mainColorMapId.IsSet());
      assert(shapeGroup.GetShapePath(shapeNum).GetColorInfo().lowColorMapId.IsSet());
      assert(shapeGroup.GetShapePath(shapeNum).GetColorInfo().innerColorMapId.IsSet());
    }
  }
  return true;
}

inline auto ShapesFx::ShapesFxImpl::SetWeightedMainColorMaps(
    const size_t shapeGroupNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  assert(AllColorMapsValid());
  m_shapeGroups.at(shapeGroupNum).SetWeightedMainColorMaps(weightedMaps);
  assert(AllColorMapsValid());
}

inline auto ShapesFx::ShapesFxImpl::SetWeightedLowColorMaps(
    const size_t shapeGroupNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  assert(AllColorMapsValid());
  m_shapeGroups.at(shapeGroupNum).SetWeightedLowColorMaps(weightedMaps);
  assert(AllColorMapsValid());
}

inline auto ShapesFx::ShapesFxImpl::SetWeightedInnerColorMaps(
    const size_t shapeGroupNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  assert(AllColorMapsValid());
  m_shapeGroups.at(shapeGroupNum).SetWeightedInnerColorMaps(weightedMaps);
  assert(AllColorMapsValid());
}

inline auto ShapesFx::ShapesFxImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) -> void
{
  if (static constexpr float PROB_ACCEPT_NEW_MIDPOINT = 0.1F;
      not m_goomRand.ProbabilityOf(PROB_ACCEPT_NEW_MIDPOINT))
  {
    return;
  }

  const auto xMax = static_cast<int32_t>(m_goomInfo.GetScreenInfo().width - 1);
  const auto yMax = static_cast<int32_t>(m_goomInfo.GetScreenInfo().height - 1);
  const int32_t xCutoff = xMax / 5;
  const int32_t yCutoff = yMax / 5;

  const Point2dInt shapesTargetPoint{
      std::clamp(zoomMidpoint.x, xCutoff, xMax - xCutoff),
      std::clamp(zoomMidpoint.y, yCutoff, yMax - yCutoff),
  };

  std::for_each(begin(m_shapeGroups), end(m_shapeGroups),
                [&shapesTargetPoint](ShapeGroup& shapeGroup)
                { shapeGroup.SetShapesTargetPoint(shapesTargetPoint); });
}

inline auto ShapesFx::ShapesFxImpl::Start() noexcept -> void
{
  SetFixedShapeSpeeds();

  std::for_each(begin(m_shapeGroups), end(m_shapeGroups),
                [](ShapeGroup& shapeGroup) { shapeGroup.Start(); });

  assert(AllColorMapsValid());
}

inline auto ShapesFx::ShapesFxImpl::ApplyMultiple() noexcept -> void
{
  SetBrightnessAttenuation();
  DrawShapeGroups();
  DoChanges();
}

inline auto ShapesFx::ShapesFxImpl::DrawShapeGroups() noexcept -> void
{
  assert(AllColorMapsValid());

  std::for_each(begin(m_shapeGroups), end(m_shapeGroups),
                [this](ShapeGroup& shapeGroup)
                {
                  DrawShapes(shapeGroup);
                  shapeGroup.IncrementTs();
                });

  if (FirstShapeAtMeetingPoint())
  {
    m_meetingPointColorsT.Increment();
  }

  assert(AllColorMapsValid());
}

inline auto ShapesFx::ShapesFxImpl::AreShapesCloseToMeeting() const noexcept -> bool
{
  const float positionT = GetFirstShapePositionT();

  return (T_MEETING_CUTOFF > positionT) || (positionT > (1.0F - T_MEETING_CUTOFF));
}

inline auto ShapesFx::ShapesFxImpl::SetBrightnessAttenuation() noexcept -> void
{
  if (not AreShapesCloseToMeeting())
  {
    m_brightnessAttenuation = 1.0F;
    return;
  }

  const float distanceFromOne = 1.0F - GetFirstShapeTDistanceFromClosestBoundary();

  const uint32_t totalNumShapes = ShapesFx::NUM_SHAPE_GROUPS * m_shapeGroups.front().GetNumShapes();
  const float minBrightness = 2.0F / static_cast<float>(totalNumShapes);
  static constexpr float EXPONENT = 5.0F;
  m_brightnessAttenuation = STD20::lerp(1.0F, minBrightness, std::pow(distanceFromOne, EXPONENT));
}

inline auto ShapesFx::ShapesFxImpl::DrawShapes(const ShapeGroup& shapeGroup) noexcept -> void
{
  for (size_t shapeNum = 0; shapeNum < shapeGroup.GetNumShapes(); ++shapeNum)
  {
    DrawShape(shapeGroup, shapeNum);
  }
}

inline auto ShapesFx::ShapesFxImpl::DoChanges() noexcept -> void
{
  m_synchronisedShapeChangesTimer.Increment();
  if (m_synchronisedShapeChangesTimer.Finished())
  {
    SetShapeSpeeds();
    m_synchronisedShapeChangesTimer.ResetToZero();
  }

  DoRandomChanges();
}

inline auto ShapesFx::ShapesFxImpl::DoRandomChanges() noexcept -> void
{
  std::for_each(begin(m_shapeGroups), end(m_shapeGroups),
                [](ShapeGroup& shapeGroup) { shapeGroup.DoRandomChanges(); });
}

inline auto ShapesFx::ShapesFxImpl::SetShapeSpeeds() noexcept -> void
{
  if (constexpr float PROB_FIXED_SPEEDS = 0.95F; m_goomRand.ProbabilityOf(PROB_FIXED_SPEEDS))
  {
    SetFixedShapeSpeeds();
  }
  else
  {
    SetRandomShapeSpeeds();
  }
}

inline auto ShapesFx::ShapesFxImpl::SetFixedShapeSpeeds() noexcept -> void
{
  m_fixedTMinMaxLerp = GetNewRandomMinMaxLerpT(m_goomRand, m_fixedTMinMaxLerp);
  const float positionT = GetFirstShapePositionT();

  std::for_each(begin(m_shapeGroups), end(m_shapeGroups),
                [this, &positionT](ShapeGroup& shapeGroup)
                {
                  shapeGroup.UseFixedShapesSpeed(m_fixedTMinMaxLerp);
                  shapeGroup.ResetTs(positionT);
                });
}

inline auto ShapesFx::ShapesFxImpl::SetRandomShapeSpeeds() noexcept -> void
{
  std::for_each(begin(m_shapeGroups), end(m_shapeGroups),
                [](ShapeGroup& shapeGroup) { shapeGroup.UseRandomShapesSpeed(); });
}

inline auto ShapesFx::ShapesFxImpl::GetMaxRadius(const ShapeGroup& shapeGroup) const noexcept
    -> int32_t
{
  static constexpr int32_t MAX_RADIUS_JITTER = 3;

  int32_t maxRadius =
      shapeGroup.GetCurrentShapesRadius() + m_goomRand.GetRandInRange(0, MAX_RADIUS_JITTER + 1);

  if (AreShapesCloseToMeeting())
  {
    static constexpr float EXTRA_RADIUS = 10.0F;
    static constexpr float EXPONENT = 10.0F;
    const float distanceFromOne = 1.0F - GetFirstShapeTDistanceFromClosestBoundary();
    maxRadius += static_cast<int32_t>(std::pow(distanceFromOne, EXPONENT) * EXTRA_RADIUS);
  }

  return maxRadius;
}

inline auto ShapesFx::ShapesFxImpl::DrawShape(const ShapeGroup& shapeGroup,
                                              const size_t shapeNum) noexcept -> void
{
  const ShapePath& shapePath = shapeGroup.GetShapePath(shapeNum);
  const ShapePath::ColorInfo& shapePathColorInfo = shapePath.GetColorInfo();

  const Point2dInt point = shapePath.GetNextPoint();
  const ShapeColors shapeColors = GetCurrentShapeColors(shapeGroup, shapePathColorInfo);
  const IColorMap& innerColorMap = shapeGroup.GetColorMap(shapePathColorInfo.innerColorMapId);

  DrawCircleShape(point, GetMaxRadius(shapeGroup), shapeColors, innerColorMap,
                  shapeGroup.GetInnerColorMix());
}

inline auto ShapesFx::ShapesFxImpl::DrawCircleShape(const Point2dInt& centre,
                                                    const int32_t maxRadius,
                                                    const ShapeColors& shapeColors,
                                                    const IColorMap& innerColorMap,
                                                    const float innerColorMix) noexcept -> void
{
  TValue innerColorT{UTILS::TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(maxRadius - 1)};
  const int32_t innerColorCutoffRadius = GetInnerColorCutoffRadius(maxRadius);

  const ShapeColors meetingPointColors = GetCurrentMeetingPointColors();

  static constexpr float MIN_BRIGHTNESS = 1.0F;
  static constexpr float MAX_BRIGHTNESS = 5.0F;
  TValue brightnessT{TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(maxRadius)};

  for (int32_t radius = maxRadius; radius > 1; --radius)
  {
    const float brightness =
        m_brightnessAttenuation * STD20::lerp(MIN_BRIGHTNESS, MAX_BRIGHTNESS, brightnessT());
    const Pixel innerColor = innerColorMap.GetColor(innerColorT());
    const std::vector<Pixel> colors =
        GetColors(radius, innerColorCutoffRadius, brightness, meetingPointColors, shapeColors,
                  innerColor, innerColorMix);

    m_draw.Circle(centre, radius, colors);

    brightnessT.Increment();
    innerColorT.Increment();
  }
}

inline auto ShapesFx::ShapesFxImpl::GetInnerColorCutoffRadius(const int32_t maxRadius) noexcept
    -> int32_t
{
  static constexpr int32_t RADIUS_FRAC = 3;
  static constexpr int32_t MIN_CUTOFF = 10;
  return std::max(MIN_CUTOFF, maxRadius / RADIUS_FRAC);
}

inline auto ShapesFx::ShapesFxImpl::GetCurrentShapeColors(
    const ShapeGroup& shapeGroup, const ShapePath::ColorInfo& shapePathColorInfo) noexcept
    -> ShapeColors
{
  return {shapeGroup.GetCurrentColor(shapePathColorInfo.mainColorMapId),
          shapeGroup.GetCurrentColor(shapePathColorInfo.lowColorMapId)};
}

auto ShapesFx::ShapesFxImpl::GetColors(const int32_t radius,
                                       const int32_t innerColorCutoffRadius,
                                       const float brightness,
                                       const ShapeColors& meetingPointColors,
                                       const ShapeColors& shapeColors,
                                       const Pixel& innerColor,
                                       const float innerColorMix) const noexcept
    -> std::vector<Pixel>
{
  if (FirstShapeAtMeetingPoint())
  {
    return GetFinalMeetingPointColors(brightness, meetingPointColors);
  }

  return radius <= innerColorCutoffRadius
             ? GetColorsWithoutInner(brightness, shapeColors)
             : GetColorsWithInner(brightness, shapeColors, innerColor, innerColorMix);
}

static constexpr float MAIN_COLOR_BRIGHTNESS_FACTOR = 0.5F;
static constexpr float LOW_COLOR_BRIGHTNESS_FACTOR = 0.5F;

inline auto ShapesFx::ShapesFxImpl::GetColorsWithoutInner(const float brightness,
                                                          const ShapeColors& shapeColors) noexcept
    -> std::vector<Pixel>
{
  const Pixel mainColor =
      GetBrighterColor(MAIN_COLOR_BRIGHTNESS_FACTOR * brightness, shapeColors.mainColor);
  const Pixel lowColor =
      GetBrighterColor(LOW_COLOR_BRIGHTNESS_FACTOR * brightness, shapeColors.lowColor);

  return {mainColor, lowColor};
}

inline auto ShapesFx::ShapesFxImpl::GetColorsWithInner(const float brightness,
                                                       const ShapeColors& shapeColors,
                                                       const Pixel& innerColor,
                                                       const float innerColorMix) noexcept
    -> std::vector<Pixel>
{
  const Pixel mainColor =
      GetBrighterColor(MAIN_COLOR_BRIGHTNESS_FACTOR * brightness,
                       IColorMap::GetColorMix(shapeColors.mainColor, innerColor, innerColorMix));
  const Pixel lowColor =
      GetBrighterColor(LOW_COLOR_BRIGHTNESS_FACTOR * brightness,
                       IColorMap::GetColorMix(shapeColors.lowColor, innerColor, innerColorMix));

  return {mainColor, lowColor};
}

inline auto ShapesFx::ShapesFxImpl::GetCurrentMeetingPointColors() const noexcept -> ShapeColors
{
  return {
      m_colorMapsManager.GetColorMap(m_meetingPointMainColorId).GetColor(m_meetingPointColorsT()),
      m_colorMapsManager.GetColorMap(m_meetingPointLowColorId).GetColor(m_meetingPointColorsT()),
  };
}

inline auto ShapesFx::ShapesFxImpl::GetFinalMeetingPointColors(
    const float brightness, const ShapeColors& meetingPointColors) noexcept -> std::vector<Pixel>
{
  static constexpr float BRIGHTNESS_FACTOR = 7.0F;
  return {GetBrighterColor(brightness, meetingPointColors.mainColor),
          GetBrighterColor(BRIGHTNESS_FACTOR * brightness, meetingPointColors.lowColor)};
}

} // namespace GOOM::VISUAL_FX
