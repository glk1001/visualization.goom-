#pragma once

#include "../goom_visual_fx.h"
#include "circles_tentacle_layout.h"
#include "color/color_adjustment.h"
#include "color/color_maps.h"
#include "draw/goom_draw.h"
#include "point2d.h"
#include "tentacle3d.h"
#include "tentacle_plotter.h"
#include "utils/math/damping_functions.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

class TentacleDriver
{
public:
  TentacleDriver() noexcept = delete;
  TentacleDriver(DRAW::IGoomDraw& draw,
                 const UTILS::MATH::IGoomRand& goomRand,
                 const CirclesTentacleLayout& tentacleLayout) noexcept;

  auto SetWeightedColorMaps(const IVisualFx::WeightedColorMaps& weightedColorMaps) noexcept -> void;
  auto ChangeTentacleColorMaps() -> void;

  auto StartIterating() -> void;

  auto SetTentaclesEndCentrePos(const Point2dInt& newEndCentrePos) noexcept -> void;

  auto MultiplyIterZeroYValWaveFreq(float value) -> void;
  auto SetDominantColorMaps(const std::shared_ptr<const COLOR::IColorMap>& dominantMainColorMap,
                            const std::shared_ptr<const COLOR::IColorMap>& dominantLowColorMap)
      -> void;

  auto Update() -> void;

private:
  DRAW::IGoomDraw& m_draw;
  const UTILS::MATH::IGoomRand& m_goomRand;
  const Point2dInt m_screenMidpoint =
      MidpointFromOrigin({m_draw.GetDimensions().GetWidth(), m_draw.GetDimensions().GetHeight()});

  std::shared_ptr<const COLOR::IColorMap> m_dominantMainColorMap{};
  std::shared_ptr<const COLOR::IColorMap> m_dominantLowColorMap{};

  struct IterationParams
  {
    uint32_t numNodes;
    float length;
    float iterZeroYValWaveFreq;
    UTILS::MATH::SineWaveMultiplier iterZeroYValWave;
  };
  IterationParams m_tentacleParams;
  TentaclePlotter m_tentaclePlotter;

  struct TentacleAndAttributes
  {
    Tentacle3D tentacle3D;
    std::shared_ptr<const COLOR::IColorMap> mainColorMap;
    std::shared_ptr<const COLOR::IColorMap> lowColorMap;
    Pixel currentMainColor;
    Pixel currentLowColor;
  };
  static constexpr auto NUM_CURRENT_COLOR_STEPS = 500U;
  UTILS::TValue m_currentColorT{UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE,
                                NUM_CURRENT_COLOR_STEPS};
  static constexpr auto NUM_NODE_T_OFFSET_STEPS = 10U;
  UTILS::TValue m_nodeTOffset{UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE,
                              NUM_NODE_T_OFFSET_STEPS};
  std::vector<TentacleAndAttributes> m_tentacles;
  [[nodiscard]] auto GetTentacles(const CirclesTentacleLayout& tentacleLayout) const noexcept
      -> std::vector<TentacleAndAttributes>;
  [[nodiscard]] auto CreateNewTentacle2D() const noexcept -> std::unique_ptr<Tentacle2D>;
  uint32_t m_tentacleGroupSize = static_cast<uint32_t>(m_tentacles.size());
  [[nodiscard]] auto GetMixedColors(float dominantT,
                                    float nodeT,
                                    const TentacleAndAttributes& tentacleAndAttributes,
                                    float brightness) const -> DRAW::MultiplePixels;
  static constexpr float GAMMA = 0.8F;
  const COLOR::ColorAdjustment m_colorAdjust{GAMMA,
                                             COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR};

  static constexpr auto MIN_COLOR_SEGMENT_MIX_T     = 0.7F;
  static constexpr auto MAX_COLOR_SEGMENT_MIX_T     = 1.0F;
  static constexpr auto DEFAULT_COLOR_SEGMENT_MIX_T = 0.9F;
  float m_mainColorSegmentMixT                      = DEFAULT_COLOR_SEGMENT_MIX_T;
  float m_lowColorSegmentMixT                       = DEFAULT_COLOR_SEGMENT_MIX_T;
  auto ChangeSegmentMixes() noexcept -> void;

  size_t m_updateNum = 0U;
  auto CheckForTimerEvents() -> void;

  using StepType = UTILS::TValue::StepType;

  Point2dInt m_previousEndCentrePos             = m_screenMidpoint;
  Point2dInt m_targetEndCentrePos               = m_screenMidpoint;
  static constexpr auto NUM_STEPS_TO_END_TARGET = 100U;
  UTILS::TValue m_endCentrePosT{StepType::SINGLE_CYCLE, NUM_STEPS_TO_END_TARGET};
  [[nodiscard]] auto GetAcceptableEndCentrePos(
      const Point2dInt& requestedEndCentrePos) const noexcept -> Point2dInt;
  auto UpdateTentaclesEndCentrePosOffsets() noexcept -> void;
  [[nodiscard]] static auto GetNewRadiusEndCentrePosOffset(
      float radiusScale,
      const Point2dFlt& oldTentacleEndPos,
      const Point2dInt& newCentreEndPosOffset) noexcept -> V3dFlt;
  static constexpr auto MIN_END_RADIUS       = 10.0F;
  static constexpr auto MAX_END_RADIUS       = 150.0F;
  static constexpr auto NUM_END_RADIUS_STEPS = 50U;
  UTILS::IncrementedValue<float> m_endRadius{
      MIN_END_RADIUS, MAX_END_RADIUS, StepType::CONTINUOUS_REVERSIBLE, NUM_END_RADIUS_STEPS};
  auto UpdateTentaclesEndPos() noexcept -> void;

  auto PreDrawUpdateTentacles() noexcept -> void;
  auto DrawTentacles() noexcept -> void;
  auto PostDrawUpdateTentacles() noexcept -> void;
  auto IterateTentacle(Tentacle3D& tentacle) const noexcept -> void;
  bool m_useThickLines = true;
  [[nodiscard]] auto GetLineThickness(uint32_t tentacleNum) const noexcept -> uint8_t;
};

inline auto TentacleDriver::SetDominantColorMaps(
    const std::shared_ptr<const COLOR::IColorMap>& dominantMainColorMap,
    const std::shared_ptr<const COLOR::IColorMap>& dominantLowColorMap) -> void
{
  m_dominantMainColorMap = dominantMainColorMap;
  m_dominantLowColorMap  = dominantLowColorMap;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
