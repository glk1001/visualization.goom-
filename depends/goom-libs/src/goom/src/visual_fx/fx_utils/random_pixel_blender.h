#pragma once

#include "draw/goom_draw.h"
#include "goom_config.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

namespace GOOM::VISUAL_FX::FX_UTILS
{

class RandomPixelBlender
{
public:
  enum class PixelBlendType
  {
    ADD,
    DARKEN_ONLY,
    LIGHTEN_ONLY,
    LUMA_MIX,
    MULTIPLY,
    ALPHA,
    _num // unused, and marks the enum end
  };
  static constexpr auto DEFAULT_ADD_WEIGHT          = 50.0F;
  static constexpr auto DEFAULT_DARKEN_ONLY_WEIGHT  = 10.0F;
  static constexpr auto DEFAULT_LIGHTEN_ONLY_WEIGHT = 10.0F;
  static constexpr auto DEFAULT_LUMA_MIX_WEIGHT     = 5.0F;
  static constexpr auto DEFAULT_MULTIPLY_WEIGHT     = 5.0F;
  static constexpr auto DEFAULT_ALPHA_WEIGHT        = 10.0F;

  explicit RandomPixelBlender(const UTILS::MATH::IGoomRand& goomRand) noexcept;
  RandomPixelBlender(
      const UTILS::MATH::IGoomRand& goomRand,
      const UTILS::MATH::Weights<PixelBlendType>::EventWeightPairs& weights) noexcept;

  auto Update() noexcept -> void;

  struct PixelBlenderParams
  {
    bool useRandomBlender{};
    FX_UTILS::RandomPixelBlender::PixelBlendType forceBlenderType{};
  };
  auto SetPixelBlendType(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetPixelBlendType(PixelBlendType pixelBlendType) noexcept -> void;
  auto SetRandomPixelBlendType() noexcept -> void;
  [[nodiscard]] auto GetCurrentPixelBlendFunc() const noexcept -> DRAW::IGoomDraw::PixelBlendFunc;

  [[nodiscard]] static auto GetRandomPixelBlendType(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> PixelBlendType;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  UTILS::MATH::Weights<PixelBlendType> m_pixelBlendTypeWeights;
  PixelBlendType m_nextPixelBlendType                      = PixelBlendType::ADD;
  DRAW::IGoomDraw::PixelBlendFunc m_previousPixelBlendFunc = GetColorAddPixelBlendFunc();
  DRAW::IGoomDraw::PixelBlendFunc m_nextPixelBlendFunc     = m_previousPixelBlendFunc;
  DRAW::IGoomDraw::PixelBlendFunc m_currentPixelBlendFunc  = m_previousPixelBlendFunc;
  static constexpr auto MAX_LERP_STEPS                     = 500U;
  static constexpr auto MIN_LERP_STEPS                     = 50U;
  UTILS::TValue m_lerpT{
      {UTILS::TValue::StepType::SINGLE_CYCLE, MIN_LERP_STEPS}
  };

  static const UTILS::MATH::Weights<PixelBlendType>::EventWeightPairs
      DEFAULT_PIXEL_BLEND_TYPE_WEIGHTS;

  auto SetPixelBlendFunc(PixelBlendType pixelBlendType) noexcept -> void;
  using PixelBlendFunc = DRAW::IGoomDraw::PixelBlendFunc;
  [[nodiscard]] auto GetNextPixelBlendFunc() const noexcept -> PixelBlendFunc;
  [[nodiscard]] auto GetLerpedPixelBlendFunc() const -> PixelBlendFunc;
  [[nodiscard]] static auto GetColorAddPixelBlendFunc() -> PixelBlendFunc;
  [[nodiscard]] static auto GetDarkenOnlyPixelBlendFunc() -> PixelBlendFunc;
  [[nodiscard]] static auto GetLightenOnlyPixelBlendFunc() -> PixelBlendFunc;
  [[nodiscard]] static auto GetColorMultiplyPixelBlendFunc() -> PixelBlendFunc;
  [[nodiscard]] static auto GetAlphaPixelBlendFunc() -> PixelBlendFunc;
  static constexpr auto MAX_LUMA_MIX_T = 1.0F;
  static constexpr auto MIN_LUMA_MIX_T = 0.3F;
  float m_lumaMixT                     = MIN_LUMA_MIX_T;
  [[nodiscard]] static auto GetSameLumaMixPixelBlendFunc(float bgndColor) -> PixelBlendFunc;
};

inline auto RandomPixelBlender::Update() noexcept -> void
{
  m_lerpT.Increment();

  if (m_lerpT() >= 1.0F)
  {
    m_currentPixelBlendFunc = m_nextPixelBlendFunc;
  }
}

inline auto RandomPixelBlender::GetCurrentPixelBlendFunc() const noexcept
    -> DRAW::IGoomDraw::PixelBlendFunc
{
  return m_currentPixelBlendFunc;
}

} // namespace GOOM::VISUAL_FX::FX_UTILS
