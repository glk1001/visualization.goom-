module;

#include <cstdint>

export module Goom.VisualFx.FxUtils:RandomPixelBlender;

import Goom.Color.ColorUtils;
import Goom.Draw.GoomDrawBase;
import Goom.Utils.Graphics.PixelBlend;
import Goom.Utils.Graphics.PixelUtils;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;

using GOOM::UTILS::GRAPHICS::GetColorAddPixelBlend;
using GOOM::UTILS::GRAPHICS::GetColorAlphaAndAddBlend;
using GOOM::UTILS::GRAPHICS::GetColorAlphaBlend;
using GOOM::UTILS::GRAPHICS::GetColorMultiplyPixelBlend;
using GOOM::UTILS::GRAPHICS::GetDarkenOnlyPixelBlend;
using GOOM::UTILS::GRAPHICS::GetLightenOnlyPixelBlend;
using GOOM::UTILS::GRAPHICS::GetPixelWithNewAlpha;
using GOOM::UTILS::GRAPHICS::GetSameLumaMixPixelBlend;
using GOOM::UTILS::MATH::IGoomRand;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::TValue;
using GOOM::UTILS::MATH::Weights;

export namespace GOOM::VISUAL_FX::FX_UTILS
{

class RandomPixelBlender
{
public:
  enum class PixelBlendType : UnderlyingEnumType
  {
    ADD,
    DARKEN_ONLY,
    LIGHTEN_ONLY,
    LUMA_MIX,
    MULTIPLY,
    ALPHA,
    ALPHA_AND_ADD,
  };
  static constexpr auto DEFAULT_ADD_WEIGHT           = 0.0F;
  static constexpr auto DEFAULT_DARKEN_ONLY_WEIGHT   = 0.0F;
  static constexpr auto DEFAULT_LIGHTEN_ONLY_WEIGHT  = 0.0F;
  static constexpr auto DEFAULT_LUMA_MIX_WEIGHT      = 5.0F;
  static constexpr auto DEFAULT_MULTIPLY_WEIGHT      = 0.0F;
  static constexpr auto DEFAULT_ALPHA_WEIGHT         = 20.0F;
  static constexpr auto DEFAULT_ALPHA_AND_ADD_WEIGHT = 20.0F;
  static constexpr auto DEFAULT_PIXEL_BLEND_TYPE     = PixelBlendType::ALPHA;

  explicit RandomPixelBlender(const IGoomRand& goomRand) noexcept;
  RandomPixelBlender(const IGoomRand& goomRand,
                     const Weights<PixelBlendType>::EventWeightPairs& weights) noexcept;

  auto Update() noexcept -> void;

  struct PixelBlenderParams
  {
    bool useRandomBlender{};
    PixelBlendType forceBlenderType{};
  };
  auto SetPixelBlendType(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetPixelBlendType(PixelBlendType pixelBlendType) noexcept -> void;
  auto SetRandomPixelBlendType() noexcept -> void;
  [[nodiscard]] auto GetCurrentPixelBlendFunc() const noexcept -> DRAW::IGoomDraw::PixelBlendFunc;

  [[nodiscard]] static auto GetRandomPixelBlendType(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> PixelBlendType;

private:
  const IGoomRand* m_goomRand;
  Weights<PixelBlendType> m_pixelBlendTypeWeights;
  PixelBlendType m_nextPixelBlendType                      = DEFAULT_PIXEL_BLEND_TYPE;
  DRAW::IGoomDraw::PixelBlendFunc m_previousPixelBlendFunc = GetNextPixelBlendFunc();
  DRAW::IGoomDraw::PixelBlendFunc m_nextPixelBlendFunc     = m_previousPixelBlendFunc;
  DRAW::IGoomDraw::PixelBlendFunc m_currentPixelBlendFunc  = m_previousPixelBlendFunc;
  static constexpr auto LERP_STEPS_RANGE                   = NumberRange{50U, 500U};
  TValue m_lerpT{
      {TValue::StepType::SINGLE_CYCLE, LERP_STEPS_RANGE.Min()}
  };

  static const Weights<PixelBlendType>::EventWeightPairs DEFAULT_PIXEL_BLEND_TYPE_WEIGHTS;

  auto SetPixelBlendFunc(PixelBlendType pixelBlendType) noexcept -> void;
  using PixelBlendFunc = DRAW::IGoomDraw::PixelBlendFunc;
  [[nodiscard]] auto GetNextPixelBlendFunc() const noexcept -> PixelBlendFunc;
  [[nodiscard]] auto GetLerpedPixelBlendFunc() const -> PixelBlendFunc;
  [[nodiscard]] static auto GetColorAddPixelBlendFunc() -> PixelBlendFunc;
  [[nodiscard]] static auto GetDarkenOnlyPixelBlendFunc() -> PixelBlendFunc;
  [[nodiscard]] static auto GetLightenOnlyPixelBlendFunc() -> PixelBlendFunc;
  [[nodiscard]] static auto GetColorMultiplyPixelBlendFunc() -> PixelBlendFunc;
  [[nodiscard]] static auto GetAlphaPixelBlendFunc() -> PixelBlendFunc;
  [[nodiscard]] static auto GetAlphaAndAddPixelBlendFunc() -> PixelBlendFunc;
  static constexpr auto LUMA_MIX_T_RANGE = NumberRange{0.3F, 1.0F};
  float m_lumaMixT                       = LUMA_MIX_T_RANGE.Min();
  [[nodiscard]] static auto GetSameLumaMixPixelBlendFunc(float lumaMixT) -> PixelBlendFunc;
};

} // namespace GOOM::VISUAL_FX::FX_UTILS

namespace GOOM::VISUAL_FX::FX_UTILS
{

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

const Weights<RandomPixelBlender::PixelBlendType>::EventWeightPairs
    // NOLINTNEXTLINE(cert-err58-cpp): How to fix this?
    RandomPixelBlender::DEFAULT_PIXEL_BLEND_TYPE_WEIGHTS{
        {          RandomPixelBlender::PixelBlendType::ADD,           DEFAULT_ADD_WEIGHT},
        {  RandomPixelBlender::PixelBlendType::DARKEN_ONLY,   DEFAULT_DARKEN_ONLY_WEIGHT},
        { RandomPixelBlender::PixelBlendType::LIGHTEN_ONLY,  DEFAULT_LIGHTEN_ONLY_WEIGHT},
        {     RandomPixelBlender::PixelBlendType::LUMA_MIX,      DEFAULT_LUMA_MIX_WEIGHT},
        {     RandomPixelBlender::PixelBlendType::MULTIPLY,      DEFAULT_MULTIPLY_WEIGHT},
        {        RandomPixelBlender::PixelBlendType::ALPHA,         DEFAULT_ALPHA_WEIGHT},
        {RandomPixelBlender::PixelBlendType::ALPHA_AND_ADD, DEFAULT_ALPHA_AND_ADD_WEIGHT},
};

auto RandomPixelBlender::GetRandomPixelBlendType(const IGoomRand& goomRand) noexcept
    -> PixelBlendType
{
  const auto pixelBlendTypeWeights =
      UTILS::MATH::Weights<PixelBlendType>{goomRand, DEFAULT_PIXEL_BLEND_TYPE_WEIGHTS};

  return pixelBlendTypeWeights.GetRandomWeighted();
}

RandomPixelBlender::RandomPixelBlender(const UTILS::MATH::IGoomRand& goomRand) noexcept
  : m_goomRand{&goomRand}, m_pixelBlendTypeWeights{goomRand, DEFAULT_PIXEL_BLEND_TYPE_WEIGHTS}
{
}

RandomPixelBlender::RandomPixelBlender(
    const UTILS::MATH::IGoomRand& goomRand,
    const UTILS::MATH::Weights<PixelBlendType>::EventWeightPairs& weights) noexcept
  : m_goomRand{&goomRand}, m_pixelBlendTypeWeights{goomRand, weights}
{
}

auto RandomPixelBlender::SetPixelBlendType(const PixelBlenderParams& pixelBlenderParams) noexcept
    -> void
{
  if (pixelBlenderParams.useRandomBlender)
  {
    SetRandomPixelBlendType();
  }
  else
  {
    SetPixelBlendType(pixelBlenderParams.forceBlenderType);
  }
}

auto RandomPixelBlender::SetPixelBlendType(const PixelBlendType pixelBlendType) noexcept -> void
{
  SetPixelBlendFunc(pixelBlendType);
}

auto RandomPixelBlender::SetRandomPixelBlendType() noexcept -> void
{
  SetPixelBlendFunc(m_pixelBlendTypeWeights.GetRandomWeighted());
}

auto RandomPixelBlender::SetPixelBlendFunc(const PixelBlendType pixelBlendType) noexcept -> void
{
  const auto previousPixelBlendType = m_nextPixelBlendType;

  m_lumaMixT               = m_goomRand->GetRandInRange(LUMA_MIX_T_RANGE);
  m_previousPixelBlendFunc = m_nextPixelBlendFunc;
  m_nextPixelBlendType     = pixelBlendType;

  if (previousPixelBlendType != m_nextPixelBlendType)
  {
    m_nextPixelBlendFunc    = GetNextPixelBlendFunc();
    m_currentPixelBlendFunc = GetLerpedPixelBlendFunc();
  }

  m_lerpT.SetNumSteps(m_goomRand->GetRandInRange(LERP_STEPS_RANGE));
  m_lerpT.Reset();
}

auto RandomPixelBlender::GetNextPixelBlendFunc() const noexcept -> PixelBlendFunc
{
  switch (m_nextPixelBlendType)
  {
    case PixelBlendType::ADD:
      return GetColorAddPixelBlendFunc();
    case PixelBlendType::DARKEN_ONLY:
      return GetDarkenOnlyPixelBlendFunc();
    case PixelBlendType::LIGHTEN_ONLY:
      return GetLightenOnlyPixelBlendFunc();
    case PixelBlendType::LUMA_MIX:
      return GetSameLumaMixPixelBlendFunc(m_lumaMixT);
    case PixelBlendType::MULTIPLY:
      return GetColorMultiplyPixelBlendFunc();
    case PixelBlendType::ALPHA:
      return GetAlphaPixelBlendFunc();
    case PixelBlendType::ALPHA_AND_ADD:
      return GetAlphaAndAddPixelBlendFunc();
  }
}

auto RandomPixelBlender::GetLerpedPixelBlendFunc() const -> PixelBlendFunc
{
  return [this](const Pixel& bgndColor,
                const uint32_t intBuffIntensity,
                const Pixel& fgndColor,
                const PixelChannelType newAlpha)
  {
    return COLOR::GetRgbColorLerp(
        m_previousPixelBlendFunc(bgndColor, intBuffIntensity, fgndColor, newAlpha),
        m_nextPixelBlendFunc(bgndColor, intBuffIntensity, fgndColor, newAlpha),
        m_lerpT());
  };
}

auto RandomPixelBlender::GetColorAddPixelBlendFunc() -> PixelBlendFunc
{
  return [](const Pixel& bgndColor,
            const uint32_t intBuffIntensity,
            const Pixel& fgndColor,
            const PixelChannelType newAlpha)
  {
    if (0 == bgndColor.A())
    {
      return GetPixelWithNewAlpha(fgndColor, newAlpha);
    }
    return GetColorAddPixelBlend(bgndColor, intBuffIntensity, fgndColor, newAlpha);
  };
}

auto RandomPixelBlender::GetDarkenOnlyPixelBlendFunc() -> PixelBlendFunc
{
  return [](const Pixel& bgndColor,
            const uint32_t intBuffIntensity,
            const Pixel& fgndColor,
            const PixelChannelType newAlpha)
  {
    if (0 == bgndColor.A())
    {
      return GetPixelWithNewAlpha(fgndColor, newAlpha);
    }
    return GetDarkenOnlyPixelBlend(bgndColor, intBuffIntensity, fgndColor, newAlpha);
  };
}

auto RandomPixelBlender::GetLightenOnlyPixelBlendFunc() -> PixelBlendFunc
{
  return [](const Pixel& bgndColor,
            const uint32_t intBuffIntensity,
            const Pixel& fgndColor,
            const PixelChannelType newAlpha)
  {
    if (0 == bgndColor.A())
    {
      return GetPixelWithNewAlpha(fgndColor, newAlpha);
    }
    return GetLightenOnlyPixelBlend(bgndColor, intBuffIntensity, fgndColor, newAlpha);
  };
}

auto RandomPixelBlender::GetColorMultiplyPixelBlendFunc() -> PixelBlendFunc
{
  return [](const Pixel& bgndColor,
            const uint32_t intBuffIntensity,
            const Pixel& fgndColor,
            const PixelChannelType newAlpha)
  {
    if (0 == bgndColor.A())
    {
      return GetPixelWithNewAlpha(fgndColor, newAlpha);
    }
    return GetColorMultiplyPixelBlend(bgndColor, intBuffIntensity, fgndColor, newAlpha);
  };
}

auto RandomPixelBlender::GetAlphaPixelBlendFunc() -> PixelBlendFunc
{
  return [](const Pixel& bgndColor,
            [[maybe_unused]] const uint32_t intBuffIntensity,
            const Pixel& fgndColor,
            const PixelChannelType newAlpha)
  {
    if (0 == bgndColor.A())
    {
      return GetPixelWithNewAlpha(fgndColor, newAlpha);
    }
    return GetColorAlphaBlend(bgndColor, fgndColor, newAlpha);
  };
}

auto RandomPixelBlender::GetAlphaAndAddPixelBlendFunc() -> PixelBlendFunc
{
  return [](const Pixel& bgndColor,
            const uint32_t intBuffIntensity,
            const Pixel& fgndColor,
            const PixelChannelType newAlpha)
  {
    if (0 == bgndColor.A())
    {
      return GetPixelWithNewAlpha(fgndColor, newAlpha);
    }
    return GetColorAlphaAndAddBlend(bgndColor, intBuffIntensity, fgndColor, newAlpha);
  };
}

auto RandomPixelBlender::GetSameLumaMixPixelBlendFunc(const float lumaMixT) -> PixelBlendFunc
{
  return [lumaMixT](const Pixel& bgndColor,
                    const uint32_t intBuffIntensity,
                    const Pixel& fgndColor,
                    const PixelChannelType newAlpha)
  {
    if (0 == bgndColor.A())
    {
      return GetPixelWithNewAlpha(fgndColor, newAlpha);
    }
    return GetSameLumaMixPixelBlend(lumaMixT, bgndColor, intBuffIntensity, fgndColor, newAlpha);
  };
}

} // namespace GOOM::VISUAL_FX::FX_UTILS
