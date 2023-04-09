#include "random_pixel_blender.h"

#include "goom_config.h"
#include "utils/graphics/pixel_blend.h"
#include "utils/math/goom_rand_base.h"

namespace GOOM::VISUAL_FX::FX_UTILS
{
using UTILS::GRAPHICS::GetColorAddPixelBlend;
using UTILS::GRAPHICS::GetColorMultiplyPixelBlend;
using UTILS::GRAPHICS::GetDarkenOnlyPixelBlend;
using UTILS::GRAPHICS::GetLightenOnlyPixelBlend;
using UTILS::GRAPHICS::GetSameLumaMixPixelBlend;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Weights;

const Weights<RandomPixelBlender::PixelBlendType>::EventWeightPairs
    // NOLINTNEXTLINE(cert-err58-cpp): Fix with C++20 and 'constexpr'.
    RandomPixelBlender::DEFAULT_PIXEL_BLEND_TYPE_WEIGHTS{
        {         RandomPixelBlender::PixelBlendType::ADD,          DEFAULT_ADD_WEIGHT},
        { RandomPixelBlender::PixelBlendType::DARKEN_ONLY,  DEFAULT_DARKEN_ONLY_WEIGHT},
        {RandomPixelBlender::PixelBlendType::LIGHTEN_ONLY, DEFAULT_LIGHTEN_ONLY_WEIGHT},
        {    RandomPixelBlender::PixelBlendType::LUMA_MIX,     DEFAULT_LUMA_MIX_WEIGHT},
        {    RandomPixelBlender::PixelBlendType::MULTIPLY,     DEFAULT_MULTIPLY_WEIGHT}
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
    default:
      FailFast();
  }
}

auto RandomPixelBlender::GetLerpedPixelBlendFunc() const -> PixelBlendFunc
{
  return [this](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  {
    return COLOR::GetRgbColorLerp(m_previousPixelBlendFunc(oldColor, newColor, intBuffIntensity),
                                  m_nextPixelBlendFunc(oldColor, newColor, intBuffIntensity),
                                  m_lerpT());
  };
}

auto RandomPixelBlender::GetColorAddPixelBlendFunc() -> PixelBlendFunc
{
  return [](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  { return GetColorAddPixelBlend(oldColor, newColor, intBuffIntensity); };
}

auto RandomPixelBlender::GetDarkenOnlyPixelBlendFunc() -> PixelBlendFunc
{
  return [](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  { return GetDarkenOnlyPixelBlend(oldColor, newColor, intBuffIntensity); };
}

auto RandomPixelBlender::GetLightenOnlyPixelBlendFunc() -> PixelBlendFunc
{
  return [](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  { return GetLightenOnlyPixelBlend(oldColor, newColor, intBuffIntensity); };
}

auto RandomPixelBlender::GetColorMultiplyPixelBlendFunc() -> PixelBlendFunc
{
  return [](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  { return GetColorMultiplyPixelBlend(oldColor, newColor, intBuffIntensity); };
}

auto RandomPixelBlender::GetSameLumaMixPixelBlendFunc(const float lumaMixT) -> PixelBlendFunc
{
  return [lumaMixT](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  { return GetSameLumaMixPixelBlend(lumaMixT, oldColor, newColor, intBuffIntensity); };
}

} // namespace GOOM::VISUAL_FX::FX_UTILS
