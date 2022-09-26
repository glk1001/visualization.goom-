#pragma once

#include "../goom_visual_fx.h"
#include "color/color_adjustment.h"
#include "color/random_color_maps.h"
#include "color/random_color_maps_manager.h"
#include "draw/goom_draw.h"
#include "goom_graphic.h"
#include "tentacle2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

struct V3dFlt
{
  float x = 0.0;
  float y = 0.0;
  float z = 0.0;
};
constexpr auto operator+(const V3dFlt& point1, const V3dFlt& point2) noexcept -> V3dFlt;
constexpr auto lerp(const V3dFlt& point1, const V3dFlt& point2, float t) noexcept -> V3dFlt;

class Tentacle3D
{
public:
  Tentacle3D() noexcept = delete;
  Tentacle3D(std::unique_ptr<Tentacle2D> tentacle,
             const UTILS::MATH::IGoomRand& goomRand) noexcept;
  Tentacle3D(const Tentacle3D&) noexcept = delete;
  Tentacle3D(Tentacle3D&&) noexcept      = default;
  ~Tentacle3D() noexcept;
  auto operator=(const Tentacle3D&) -> Tentacle3D& = delete;
  auto operator=(Tentacle3D&&) -> Tentacle3D&      = delete;

  auto Get2DTentacle() -> Tentacle2D&;
  [[nodiscard]] auto Get2DTentacle() const -> const Tentacle2D&;

  auto SetWeightedColorMaps(const IVisualFx::WeightedColorMaps& weightedColorMaps) noexcept -> void;
  auto ChangeColorMaps() -> void;

  [[nodiscard]] auto GetMixedColors(size_t nodeNum,
                                    const DRAW::MultiplePixels& dominantColors,
                                    float brightness) const -> DRAW::MultiplePixels;

  static constexpr float START_SMALL_X = 10.0F;
  [[nodiscard]] auto GetStartPos() const -> const V3dFlt&;
  auto SetStartPos(const V3dFlt& val) noexcept -> void;

  auto SetEndPos(const V3dFlt& val) noexcept -> void;
  auto SetEndPosOffset(const V3dFlt& val) noexcept -> void;

  [[nodiscard]] auto GetTentacleVertices() const -> std::vector<V3dFlt>;
  auto Update() noexcept -> void;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  std::unique_ptr<Tentacle2D> m_tentacle;

  COLOR::RandomColorMapsManager m_colorMapsManager{};
  std::shared_ptr<const COLOR::RandomColorMaps> m_mainColorMaps{};
  std::shared_ptr<const COLOR::RandomColorMaps> m_lowColorMaps{};
  COLOR::RandomColorMapsManager::ColorMapId m_mainColorMapID{
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand)};
  COLOR::RandomColorMapsManager::ColorMapId m_lowColorMapID{
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand)};

  static constexpr float MIN_COLOR_SEGMENT_MIX_T     = 0.7F;
  static constexpr float MAX_COLOR_SEGMENT_MIX_T     = 1.0F;
  static constexpr float DEFAULT_COLOR_SEGMENT_MIX_T = 0.9F;
  float m_mainColorSegmentMixT                       = DEFAULT_COLOR_SEGMENT_MIX_T;
  float m_lowColorSegmentMixT                        = DEFAULT_COLOR_SEGMENT_MIX_T;

  static constexpr float GAMMA = 0.8F;
  const COLOR::ColorAdjustment m_colorAdjust{GAMMA,
                                             COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR};

  V3dFlt m_startPos{};
  V3dFlt m_startPosOffset{};
  V3dFlt m_previousStartPosOffset{};
  V3dFlt m_endPos{};
  V3dFlt m_endPosOffset{};
  V3dFlt m_previousEndPosOffset{};

  static constexpr uint32_t NUM_POS_OFFSET_STEPS = 100;
  UTILS::TValue m_startPosOffsetT{UTILS::TValue::StepType::SINGLE_CYCLE, NUM_POS_OFFSET_STEPS};
  UTILS::TValue m_endPosOffsetT{UTILS::TValue::StepType::SINGLE_CYCLE, NUM_POS_OFFSET_STEPS};

  [[nodiscard]] auto GetCurrentStartPostOffset() const noexcept -> V3dFlt;
  [[nodiscard]] auto GetCurrentEndPostOffset() const noexcept -> V3dFlt;

  [[nodiscard]] auto GetMixedColors(size_t nodeNum,
                                    const DRAW::MultiplePixels& dominantColors) const
      -> DRAW::MultiplePixels;
};

inline auto Tentacle3D::Get2DTentacle() -> Tentacle2D&
{
  return *m_tentacle;
}

inline auto Tentacle3D::Get2DTentacle() const -> const Tentacle2D&
{
  return *m_tentacle;
}

inline auto Tentacle3D::GetStartPos() const -> const V3dFlt&
{
  return m_startPos;
}

inline auto Tentacle3D::SetStartPos(const V3dFlt& val) noexcept -> void
{
  m_startPos = val;
}

inline auto Tentacle3D::SetEndPos(const V3dFlt& val) noexcept -> void
{
  m_endPos = val;
}

inline auto Tentacle3D::Update() noexcept -> void
{
  m_startPosOffsetT.Increment();
  m_endPosOffsetT.Increment();
}

constexpr auto operator+(const V3dFlt& point1, const V3dFlt& point2) noexcept -> V3dFlt
{
  return {
      point1.x + point2.x,
      point1.y + point2.y,
      point1.z + point2.z,
  };
}

constexpr auto lerp(const V3dFlt& point1, const V3dFlt& point2, const float t) noexcept -> V3dFlt
{
  return {
      STD20::lerp(point1.x, point2.x, t),
      STD20::lerp(point1.y, point2.y, t),
      STD20::lerp(point1.z, point2.z, t),
  };
}

} // namespace GOOM::VISUAL_FX::TENTACLES
