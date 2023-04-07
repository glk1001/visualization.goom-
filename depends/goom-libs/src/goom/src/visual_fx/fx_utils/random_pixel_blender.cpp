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

static constexpr auto ADD_WEIGHT          = 50.0F;
static constexpr auto DARKEN_ONLY_WEIGHT  = 10.0F;
static constexpr auto LIGHTEN_ONLY_WEIGHT = 10.0F;
static constexpr auto LUMA_MIX_WEIGHT     = 5.0F;
static constexpr auto MULTIPLY_WEIGHT     = 5.0F;

auto RandomPixelBlender::GetDefaultPixelBlender(const IGoomRand& goomRand) noexcept
    -> RandomPixelBlender
{
  return {
      goomRand,
      {{RandomPixelBlender::PixelBlendType::ADD, ADD_WEIGHT},
        {RandomPixelBlender::PixelBlendType::DARKEN_ONLY, DARKEN_ONLY_WEIGHT},
        {RandomPixelBlender::PixelBlendType::LIGHTEN_ONLY, LIGHTEN_ONLY_WEIGHT},
        {RandomPixelBlender::PixelBlendType::LUMA_MIX, LUMA_MIX_WEIGHT},
        {RandomPixelBlender::PixelBlendType::MULTIPLY, MULTIPLY_WEIGHT}}
  };
}

RandomPixelBlender::RandomPixelBlender(
    const UTILS::MATH::IGoomRand& goomRand,
    const UTILS::MATH::Weights<PixelBlendType>::EventWeightPairs& weights) noexcept
  : m_goomRand{&goomRand}, m_pixelBlendTypeWeights{goomRand, weights}
{
}

auto RandomPixelBlender::ChangePixelBlendFunc() noexcept -> void
{
  const auto previousPixelBlendType = m_nextPixelBlendType;

  m_lumaMixT               = m_goomRand->GetRandInRange(MIN_LUMA_MIX_T, MAX_LUMA_MIX_T);
  m_previousPixelBlendFunc = m_nextPixelBlendFunc;
  m_nextPixelBlendType     = m_pixelBlendTypeWeights.GetRandomWeighted();

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
