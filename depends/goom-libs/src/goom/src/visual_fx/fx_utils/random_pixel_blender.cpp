#include "random_pixel_blender.h"

#include "color/color_utils.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "utils/graphics/pixel_blend.h"
#include "utils/graphics/pixel_utils.h"
#include "utils/math/goom_rand_base.h"

#include <cstdint>

namespace GOOM::VISUAL_FX::FX_UTILS
{
using UTILS::GRAPHICS::GetColorAddPixelBlend;
using UTILS::GRAPHICS::GetColorAlphaAndAddBlend;
using UTILS::GRAPHICS::GetColorAlphaBlend;
using UTILS::GRAPHICS::GetColorMultiplyPixelBlend;
using UTILS::GRAPHICS::GetDarkenOnlyPixelBlend;
using UTILS::GRAPHICS::GetLightenOnlyPixelBlend;
using UTILS::GRAPHICS::GetPixelWithNewAlpha;
using UTILS::GRAPHICS::GetSameLumaMixPixelBlend;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Weights;

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

  m_lumaMixT               = m_goomRand->GetRandInRange(MIN_LUMA_MIX_T, MAX_LUMA_MIX_T);
  m_previousPixelBlendFunc = m_nextPixelBlendFunc;
  m_nextPixelBlendType     = pixelBlendType;

  if (previousPixelBlendType != m_nextPixelBlendType)
  {
    m_nextPixelBlendFunc    = GetNextPixelBlendFunc();
    m_currentPixelBlendFunc = GetLerpedPixelBlendFunc();
  }

  m_lerpT.SetNumSteps(m_goomRand->GetRandInRange(MIN_LERP_STEPS, MAX_LERP_STEPS + 1U));
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
