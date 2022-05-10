#pragma once

#include "color/colormaps.h"
#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "shape_paths.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::SHAPES
{

class ShapePart
{
public:
  struct Params
  {
    uint32_t shapePartNum;
    uint32_t totalNumShapeParts;
    float minRadiusFraction;
    float maxRadiusFraction;
    int32_t minShapeDotRadius;
    int32_t maxShapeDotRadius;
    uint32_t maxNumShapePaths;
    float tMinMaxLerp;
    Point2dInt shapePathsTargetPoint;
    float minShapePathSpeed;
    float maxShapePathSpeed;
  };

  ShapePart(const UTILS::MATH::IGoomRand& goomRand,
            const PluginInfo& goomInfo,
            COLOR::RandomColorMapsManager& colorMapsManager,
            const Params& params) noexcept;
  ShapePart(const ShapePart&) noexcept = delete;
  ShapePart(ShapePart&&) noexcept = default;
  ~ShapePart() noexcept = default;
  auto operator=(const ShapePart&) -> ShapePart& = delete;
  auto operator=(ShapePart&&) -> ShapePart& = delete;

  auto SetWeightedMainColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps) noexcept
      -> void;
  auto SetWeightedLowColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps) noexcept
      -> void;
  auto SetWeightedInnerColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps) noexcept
      -> void;

  auto SetShapePathsTargetPoint(const Point2dInt& targetPoint) -> void;

  auto SetMinMaxShapePathSpeeds(float minShapePathSpeed, float maxShapePathSpeed) noexcept -> void;

  auto Start() noexcept -> void;

  auto Update() noexcept -> void;
  auto ResetTs(float val) noexcept -> void;

  auto DoRandomChanges() noexcept -> void;
  auto UseRandomShapePathsSpeed() noexcept -> void;
  auto UseFixedShapePathsSpeed(float tMinMaxLerp) noexcept -> void;
  auto UseEvenShapePartNumsForDirection(bool val) -> void;
  [[nodiscard]] static auto GetNewRandomMinMaxLerpT(const UTILS::MATH::IGoomRand& goomRand,
                                                    float oldTMinMaxLerp) noexcept -> float;

  auto UpdateMainColorMapId(COLOR::RandomColorMapsManager::ColorMapId mainColorMapId) noexcept
      -> void;
  auto UpdateLowColorMapId(COLOR::RandomColorMapsManager::ColorMapId lowColorMapId) noexcept
      -> void;
  auto UpdateInnerColorMapId(COLOR::RandomColorMapsManager::ColorMapId innerColorMapId) noexcept
      -> void;

  [[nodiscard]] auto GetNumShapePaths() const noexcept -> uint32_t;
  [[nodiscard]] auto GetShapePath(size_t shapePathNum) const noexcept -> const ShapePath&;
  [[nodiscard]] auto GetCurrentShapeDotRadius(bool varyRadius) const noexcept -> int32_t;
  [[nodiscard]] auto GetFirstShapePathPositionT() const noexcept -> float;
  [[nodiscard]] auto GetFirstShapePathTDistanceFromClosestBoundary() const noexcept -> float;
  [[nodiscard]] auto AreShapePathsCloseToMeeting() const noexcept -> bool;

  [[nodiscard]] auto GetColorMap(COLOR::RandomColorMapsManager::ColorMapId colorMapId)
      const noexcept -> const COLOR::IColorMap&;
  [[nodiscard]] auto GetCurrentColor(
      COLOR::RandomColorMapsManager::ColorMapId colorMapId) const noexcept -> Pixel;
  [[nodiscard]] auto GetInnerColorMix() const noexcept -> float;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  const PluginInfo& m_goomInfo;
  COLOR::RandomColorMapsManager& m_colorMapsManager;
  Point2dInt m_shapePathsTargetPoint;
  Point2dInt m_oldShapePathsTargetPoint{m_shapePathsTargetPoint};
  bool m_needToUpdateTargetPoint = false;
  Point2dInt m_newShapePathsTargetPoint;
  auto UpdateShapePathTargets() noexcept -> void;
  auto UpdateShapePathTransform(ShapePath& shapePath) const noexcept -> void;
  static constexpr uint32_t OLD_TO_NEW_NUM_STEPS = 100;
  UTILS::TValue m_oldToNewLerpT{UTILS::TValue::StepType::SINGLE_CYCLE, OLD_TO_NEW_NUM_STEPS};
  const uint32_t m_shapePartNum;
  const uint32_t m_totalNumShapeParts;
  const float m_minRadiusFraction;
  const float m_maxRadiusFraction;
  static constexpr uint32_t MIN_NUM_SHAPE_PATHS = 4;
  const uint32_t m_maxNumShapePaths;

  bool m_useEvenShapePartNumsForDirection = true;
  bool m_useRandomShapePathsSpeed = false;
  float m_currentTMinMaxLerp = 0.0F;
  float m_fixedTMinMaxLerp = 0.0F;
  float m_minShapePathSpeed;
  float m_maxShapePathSpeed;
  auto SetShapePathsSpeed() noexcept -> void;
  [[nodiscard]] auto GetShapePathSpeed(float tMinMaxLerp) const noexcept -> float;

  const int32_t m_minShapeDotRadius;
  const int32_t m_maxShapeDotRadius;
  static constexpr uint32_t MIN_DOT_RADIUS_STEPS = 100;
  static constexpr uint32_t MAX_DOT_RADIUS_STEPS = 300;
  static constexpr uint32_t INITIAL_DOT_RADIUS_STEPS = 200;
  UTILS::TValue m_dotRadiusT{UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE,
                             INITIAL_DOT_RADIUS_STEPS};

  static constexpr float MIN_COLOR_MAP_SPEED = 0.005F;
  static_assert((0.0F < MIN_COLOR_MAP_SPEED) and (MIN_COLOR_MAP_SPEED < 1.0F));
  static constexpr float MAX_COLOR_MAP_SPEED = 0.05F;
  static_assert((0.0F < MAX_COLOR_MAP_SPEED) and (MAX_COLOR_MAP_SPEED < 1.0F));
  UTILS::TValue m_allColorsT;
  auto ChangeAllColorsT() noexcept -> void;

  static inline const std::set<COLOR::RandomColorMaps::ColorMapTypes> COLOR_MAP_TYPES =
      COLOR::RandomColorMaps::ALL_COLOR_MAP_TYPES;
  static constexpr float MIN_INNER_COLOR_MIX_T = 0.1F;
  static constexpr float MAX_INNER_COLOR_MIX_T = 1.0F;
  struct ColorInfo
  {
    std::shared_ptr<COLOR::RandomColorMaps> mainColorMaps;
    COLOR::COLOR_DATA::ColorMapName mainColormapName;
    std::shared_ptr<COLOR::RandomColorMaps> lowColorMaps;
    COLOR::COLOR_DATA::ColorMapName lowColormapName;
    std::shared_ptr<COLOR::RandomColorMaps> innerColorMaps;
    COLOR::COLOR_DATA::ColorMapName innerColormapName;
    float innerColorMix;
  };
  [[nodiscard]] auto GetInitialColorInfo() const noexcept -> ColorInfo;
  ColorInfo m_colorInfo{GetInitialColorInfo()};
  bool m_useRandomColorNames = false;

  std::vector<ShapePath> m_shapePaths{};
  auto IncrementTs() noexcept -> void;
  auto ChangeAllColorMapsNow() noexcept -> void;
  auto SetRandomizedShapePaths() noexcept -> void;
  [[nodiscard]] auto GetRandomizedShapePaths() noexcept -> std::vector<ShapePath>;
  [[nodiscard]] auto GetShapePaths(uint32_t numShapePaths, float minScale, float maxScale) noexcept
      -> std::vector<ShapePath>;
  [[nodiscard]] static auto GetTransform2d(const Vec2dFlt& targetPoint,
                                           float radius,
                                           float scale,
                                           float rotate) noexcept -> UTILS::MATH::Transform2d;
  struct ShapeFunctionParams
  {
    float radius;
    UTILS::MATH::AngleParams angleParams;
    UTILS::MATH::CircleFunction::Direction direction;
  };
  [[nodiscard]] auto GetCircleRadius() const noexcept -> float;
  [[nodiscard]] auto GetCircleDirection() const noexcept -> UTILS::MATH::CircleFunction::Direction;
  [[nodiscard]] auto MakeShapePathColorInfo() noexcept -> ShapePath::ColorInfo;
  [[nodiscard]] static auto GetCirclePath(float radius,
                                          UTILS::MATH::CircleFunction::Direction direction,
                                          float speed) noexcept -> UTILS::MATH::CirclePath;
  [[nodiscard]] static auto GetCircleFunction(const ShapeFunctionParams& params)
      -> UTILS::MATH::CircleFunction;
};

static_assert(std::is_nothrow_move_constructible_v<ShapePart>);

inline auto ShapePart::Start() noexcept -> void
{
  SetRandomizedShapePaths();
}

inline auto ShapePart::GetNumShapePaths() const noexcept -> uint32_t
{
  return static_cast<uint32_t>(m_shapePaths.size());
}

inline auto ShapePart::GetShapePath(const size_t shapePathNum) const noexcept -> const ShapePath&
{
  return m_shapePaths.at(shapePathNum);
}

inline auto ShapePart::GetColorMap(const COLOR::RandomColorMapsManager::ColorMapId colorMapId)
    const noexcept -> const COLOR::IColorMap&
{
  return m_colorMapsManager.GetColorMap(colorMapId);
}

inline auto ShapePart::GetCurrentColor(
    const COLOR::RandomColorMapsManager::ColorMapId colorMapId) const noexcept -> Pixel
{
  return GetColorMap(colorMapId).GetColor(m_allColorsT());
}

inline auto ShapePart::GetInnerColorMix() const noexcept -> float
{
  return m_colorInfo.innerColorMix;
}

inline auto ShapePart::GetCurrentShapeDotRadius(const bool varyRadius) const noexcept -> int32_t
{
  if (not varyRadius)
  {
    return m_minShapeDotRadius;
  }

  return STD20::lerp(m_minShapeDotRadius, m_maxShapeDotRadius, m_dotRadiusT());
}

inline auto ShapePart::GetFirstShapePathPositionT() const noexcept -> float
{
  if (0 == GetNumShapePaths())
  {
    return 1.0F;
  }

  return GetShapePath(0).GetCurrentT();
}

inline auto ShapePart::GetFirstShapePathTDistanceFromClosestBoundary() const noexcept -> float
{
  const float positionT = GetFirstShapePathPositionT();

  if (positionT < UTILS::MATH::HALF)
  {
    return positionT;
  }

  return 1.0F - positionT;
}

inline auto ShapePart::AreShapePathsCloseToMeeting() const noexcept -> bool
{
  static constexpr float T_MEETING_CUTOFF = 0.1F;
  const float positionT = GetFirstShapePathPositionT();

  return (T_MEETING_CUTOFF > positionT) || (positionT > (1.0F - T_MEETING_CUTOFF));
}

inline auto ShapePart::Update() noexcept -> void
{
  IncrementTs();
  UpdateShapePathTargets();
}

inline auto ShapePart::IncrementTs() noexcept -> void
{
  std::for_each(begin(m_shapePaths), end(m_shapePaths), [](ShapePath& path) { path.IncrementT(); });

  m_oldToNewLerpT.Increment();
  m_allColorsT.Increment();
  m_dotRadiusT.Increment();
}

inline auto ShapePart::ResetTs(const float val) noexcept -> void
{
  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [&val](ShapePath& path) { path.ResetT(val); });
}

inline auto ShapePart::UseRandomShapePathsSpeed() noexcept -> void
{
  m_useRandomShapePathsSpeed = true;
}

inline auto ShapePart::UseFixedShapePathsSpeed(const float tMinMaxLerp) noexcept -> void
{
  m_fixedTMinMaxLerp = tMinMaxLerp;
  m_useRandomShapePathsSpeed = false;
}

inline auto ShapePart::UseEvenShapePartNumsForDirection(const bool val) -> void
{
  m_useEvenShapePartNumsForDirection = val;
}

inline auto ShapePart::SetRandomizedShapePaths() noexcept -> void
{
  m_shapePaths = GetRandomizedShapePaths();
}

inline auto ShapePart::GetNewRandomMinMaxLerpT(const UTILS::MATH::IGoomRand& goomRand,
                                               const float oldTMinMaxLerp) noexcept -> float
{
  static constexpr float SMALL_OFFSET = 0.2F;
  return goomRand.GetRandInRange(std::max(0.0F, -SMALL_OFFSET + oldTMinMaxLerp),
                                 std::min(1.0F, oldTMinMaxLerp + SMALL_OFFSET));
}

} // namespace GOOM::VISUAL_FX::SHAPES
