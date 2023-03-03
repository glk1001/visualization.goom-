//#undef NO_LOGGING

#include "goom_all_visual_fx.h"

#include "all_standard_visual_fx.h"
#include "color/color_maps.h"
#include "color/color_utils.h"
#include "draw/goom_draw.h"
#include "filter_fx/filter_buffers_service.h"
#include "filter_fx/filter_colors_service.h"
#include "goom_config.h"
#include "goom_logger.h"
#include "sound_info.h"
#include "utils/graphics/pixel_utils.h"
#include "utils/stopwatch.h"
#include "visual_fx/fx_helper.h"
#include "visual_fx/lines_fx.h"

#include <array>
#include <memory>

namespace GOOM::CONTROL
{

using COLOR::GetBrighterColor;
using COLOR::GetBrighterColorInt;
using COLOR::GetColorAdd;
using COLOR::GetColorMultiply;
using COLOR::GetLuma;
using COLOR::IColorMap;
using COLOR::IsCloseToBlack;
using CONTROL::GoomDrawables;
using DRAW::IGoomDraw;
using FILTER_FX::FilterBuffersService;
using FILTER_FX::FilterColorsService;
using FILTER_FX::ZoomFilterFx;
using FILTER_FX::ZoomFilterSettings;
using FILTER_FX::AFTER_EFFECTS::AfterEffectsTypes;
using UTILS::NameValuePairs;
using UTILS::Parallel;
using UTILS::Stopwatch;
using UTILS::GRAPHICS::SmallImageBitmaps;
using VISUAL_FX::FxHelper;

static constexpr auto SMALL_LUMA = 0.1F;

PixelBlender::PixelBlender(const UTILS::MATH::IGoomRand& goomRand) noexcept : m_goomRand{&goomRand}
{
}

auto PixelBlender::ChangePixelBlendFunc() noexcept -> void
{
  m_lumaMixT               = m_goomRand->GetRandInRange(MIN_LUMA_MIX_T, MAX_LUMA_MIX_T);
  m_previousBlendPixelFunc = m_currentBlendPixelFunc;
  m_currentPixelBlendType  = m_pixelBlendTypeWeights.GetRandomWeighted();
  m_currentBlendPixelFunc  = GetPixelBlendFunc();
  m_blendT.SetNumSteps(m_goomRand->GetRandInRange(MIN_BLEND_STEPS, MAX_BLEND_STEPS + 1U));
  m_blendT.Reset();
}

auto PixelBlender::GetPixelBlendFunc() const noexcept -> DRAW::IGoomDraw::BlendPixelFunc
{
  switch (m_currentPixelBlendType)
  {
    case PixelBlendType::ADD:
      return GetColorAddBlendPixelFunc();
    case PixelBlendType::MULTIPLY:
      return GetColorMultiplyBlendPixelFunc();
    case PixelBlendType::LUMA_MIX:
      return GetSameLumaMixBlendPixelFunc();
    default:
      FailFast();
  }
}

auto PixelBlender::GetCurrentPixelBlendFunc() const noexcept -> DRAW::IGoomDraw::BlendPixelFunc
{
  if (m_currentPixelBlendType == PixelBlendType::ADD)
  {
    return GetColorAddBlendPixelFunc();
  }
  return GetLerpedBlendPixelFunc();
}

// NOTE: Tried reverse add color (where oldColor is multiplied by intensity),
//       but the resulting black pixels don't look good.
auto PixelBlender::GetColorAddBlendPixelFunc() -> IGoomDraw::BlendPixelFunc
{
  return [](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  { return GetColorAdd(oldColor, GetBrighterColorInt(intBuffIntensity, newColor)); };
}

auto PixelBlender::GetLerpedBlendPixelFunc() const -> DRAW::IGoomDraw::BlendPixelFunc
{
  return [this](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  {
    return COLOR::GetRgbColorLerp(m_previousBlendPixelFunc(oldColor, newColor, intBuffIntensity),
                                  m_currentBlendPixelFunc(oldColor, newColor, intBuffIntensity),
                                  m_blendT());
  };
}

auto PixelBlender::GetColorMultiplyBlendPixelFunc() -> IGoomDraw::BlendPixelFunc
{
  return [](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  {
    if (IsCloseToBlack(oldColor))
    {
      return newColor;
    }
    return GetColorAdd(GetColorMultiply(oldColor, newColor),
                       GetBrighterColorInt(intBuffIntensity, newColor));
  };
}

auto PixelBlender::GetSameLumaMixBlendPixelFunc() const -> IGoomDraw::BlendPixelFunc
{
  return [this](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  {
    const auto newColorLuma = GetLuma(newColor);
    if (newColorLuma < SMALL_LUMA)
    {
      return GetColorAdd(oldColor, newColor);
    }

    const auto oldColorLuma = GetLuma(oldColor);
    if (oldColorLuma < SMALL_LUMA)
    {
      return GetColorAdd(oldColor, newColor);
    }

    const auto buffIntensity = static_cast<float>(intBuffIntensity) / channel_limits<float>::max();

    const auto brightness = std::min((oldColorLuma + (buffIntensity * newColorLuma)) /
                                         STD20::lerp(oldColorLuma, newColorLuma, m_lumaMixT),
                                     COLOR::MAX_BRIGHTNESS);

    const auto finalNewColor = IColorMap::GetColorMix(oldColor, newColor, m_lumaMixT);
    return GetBrighterColor(brightness, finalNewColor);
  };
}

GoomAllVisualFx::GoomAllVisualFx(Parallel& parallel,
                                 const FxHelper& fxHelper,
                                 const SmallImageBitmaps& smallBitmaps,
                                 const std::string& resourcesDirectory,
                                 IGoomStateHandler& goomStateHandler,
                                 std::unique_ptr<FilterBuffersService> filterBuffersService,
                                 std::unique_ptr<FilterColorsService> filterColorsService) noexcept
  : m_draw{&fxHelper.GetDraw()},
    m_goomRand{&fxHelper.GetGoomRand()},
    m_goomLogger{&fxHelper.GetGoomLogger()},
    m_allStandardVisualFx{spimpl::make_unique_impl<AllStandardVisualFx>(
        parallel, fxHelper, smallBitmaps, resourcesDirectory)},
    m_zoomFilterFx{std::make_unique<ZoomFilterFx>(parallel,
                                                  fxHelper.GetGoomInfo(),
                                                  std::move(filterBuffersService),
                                                  std::move(filterColorsService))},
    m_goomStateHandler{&goomStateHandler}
{
  m_allStandardVisualFx->SetResetDrawBuffSettingsFunc([this](const GoomDrawables fx)
                                                      { ResetCurrentDrawBuffSettings(fx); });
}

auto GoomAllVisualFx::Start() noexcept -> void
{
  ChangeDrawPixelBlend();

  m_allStandardVisualFx->Start();
  m_adaptiveExposure.Start();
  m_zoomFilterFx->Start();
}

auto GoomAllVisualFx::Finish() noexcept -> void
{
  m_allStandardVisualFx->Finish();

  m_zoomFilterFx->Finish();
}

auto GoomAllVisualFx::ChangeState() noexcept -> void
{
  m_allStandardVisualFx->SuspendFx();

  static constexpr auto MAX_TRIES = 10U;
  const auto oldState             = m_goomStateHandler->GetCurrentState();

  for (auto numTry = 0U; numTry < MAX_TRIES; ++numTry)
  {
    m_goomStateHandler->ChangeToNextState();

    if ((not m_allowMultiThreadedStates) and
        GoomStateInfo::IsMultiThreaded(m_goomStateHandler->GetCurrentState()))
    {
      continue;
    }

    // Pick a different state if possible
    if (oldState != m_goomStateHandler->GetCurrentState())
    {
      break;
    }
  }

  m_currentGoomDrawables = m_goomStateHandler->GetCurrentDrawables();
  m_allStandardVisualFx->SetCurrentGoomDrawables(m_currentGoomDrawables);
  m_allStandardVisualFx->ChangeShaderVariables();

  m_allStandardVisualFx->GetLinesFx().ResetLineModes();

  m_allStandardVisualFx->ResumeFx();
}

auto GoomAllVisualFx::StartExposureControl() noexcept -> void
{
  m_doExposureControl = true;
}

auto GoomAllVisualFx::GetLastShaderVariables() const noexcept -> const GoomShaderVariables&
{
  return m_allStandardVisualFx->GetLastShaderVariables();
}

auto GoomAllVisualFx::SetSingleBufferDots(const bool value) noexcept -> void
{
  m_allStandardVisualFx->SetSingleBufferDots(value);
}

auto GoomAllVisualFx::PostStateUpdate(
    const std::unordered_set<GoomDrawables>& oldGoomDrawables) noexcept -> void
{
  m_allStandardVisualFx->PostStateUpdate(oldGoomDrawables);
}

auto GoomAllVisualFx::RefreshAllFx() noexcept -> void
{
  m_allStandardVisualFx->RefreshAllFx();
}

inline auto GoomAllVisualFx::ResetCurrentDrawBuffSettings(const GoomDrawables fx) noexcept -> void
{
  m_resetDrawBuffSettings(GetCurrentBuffSettings(fx));
}

inline auto GoomAllVisualFx::GetCurrentBuffSettings(const GoomDrawables fx) const noexcept
    -> FXBuffSettings
{
  const auto buffIntensity = m_goomRand->GetRandInRange(
      GoomStateInfo::GetBuffIntensityRange(m_goomStateHandler->GetCurrentState(), fx));
  // Careful here. > 1 reduces smearing.
  static constexpr auto INTENSITY_FACTOR = 1.0F;
  return {INTENSITY_FACTOR * buffIntensity};
}

auto GoomAllVisualFx::ChangeAllFxColorMaps() noexcept -> void
{
  m_allStandardVisualFx->ChangeColorMaps();
}

auto GoomAllVisualFx::ChangeDrawPixelBlend() noexcept -> void
{
  m_pixelBlender.ChangePixelBlendFunc();
  m_draw->SetBlendPixelFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
}

auto GoomAllVisualFx::UpdateFilterSettings(const ZoomFilterSettings& filterSettings) noexcept
    -> void
{
  if (filterSettings.filterEffectsSettingsHaveChanged)
  {
    m_zoomFilterFx->UpdateFilterEffectsSettings(filterSettings.filterEffectsSettings);
  }

  m_zoomFilterFx->UpdateFilterBufferSettings(filterSettings.filterBufferSettings);
  m_zoomFilterFx->UpdateFilterColorSettings(
      filterSettings.filterEffectsSettings.afterEffectsSettings
          .active[AfterEffectsTypes::BLOCK_WAVY]);

  m_allStandardVisualFx->SetZoomMidpoint(filterSettings.filterEffectsSettings.zoomMidpoint);
}

auto GoomAllVisualFx::ApplyCurrentStateToSingleBuffer() noexcept -> void
{
  m_allStandardVisualFx->ApplyCurrentStateToSingleBuffer();
}

auto GoomAllVisualFx::ApplyCurrentStateToMultipleBuffers(const AudioSamples& soundData) noexcept
    -> void
{
  m_allStandardVisualFx->ApplyCurrentStateToMultipleBuffers(soundData);
}

auto GoomAllVisualFx::ApplyEndEffectIfNearEnd(const Stopwatch::TimeValues& timeValues) noexcept
    -> void
{
  m_allStandardVisualFx->ApplyEndEffectIfNearEnd(timeValues);
}

auto GoomAllVisualFx::GetCurrentColorMapsNames() const noexcept -> std::unordered_set<std::string>
{
  return m_allStandardVisualFx->GetActiveColorMapsNames();
}

auto GoomAllVisualFx::GetZoomFilterFxNameValueParams() const noexcept -> NameValuePairs
{
  return m_zoomFilterFx->GetNameValueParams();
}

} // namespace GOOM::CONTROL
