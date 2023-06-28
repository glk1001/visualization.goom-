//#undef NO_LOGGING

#include "goom_all_visual_fx.h"

#include "all_standard_visual_fx.h"
#include "filter_fx/filter_buffers_service.h"
#include "filter_fx/filter_colors_service.h"
#include "goom_config.h"
#include "goom_logger.h"
#include "sound_info.h"
#include "utils/math/goom_rand_base.h"
#include "utils/stopwatch.h"
#include "visual_fx/fx_helper.h"
#include "visual_fx/fx_utils/random_pixel_blender.h"
#include "visual_fx/goom_visual_fx.h"
#include "visual_fx/lines_fx.h"

#include <memory>
#include <string>
#include <unordered_set>

namespace GOOM::CONTROL
{

using CONTROL::GoomDrawables;
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
using VISUAL_FX::IVisualFx;
using VISUAL_FX::FX_UTILS::RandomPixelBlender;

GoomAllVisualFx::GoomAllVisualFx(Parallel& parallel,
                                 const FxHelper& fxHelper,
                                 const SmallImageBitmaps& smallBitmaps,
                                 const std::string& resourcesDirectory,
                                 IGoomStateHandler& goomStateHandler,
                                 std::unique_ptr<FilterBuffersService> filterBuffersService,
                                 std::unique_ptr<FilterColorsService> filterColorsService) noexcept
  : m_goomRand{fxHelper.goomRand},
    m_goomLogger{fxHelper.goomLogger},
    m_allStandardVisualFx{spimpl::make_unique_impl<AllStandardVisualFx>(
        parallel, fxHelper, smallBitmaps, resourcesDirectory)},
    m_zoomFilterFx{std::make_unique<ZoomFilterFx>(
        *fxHelper.goomInfo, std::move(filterBuffersService), std::move(filterColorsService))},
    m_goomStateHandler{&goomStateHandler}
{
  m_allStandardVisualFx->SetResetDrawBuffSettingsFunc([this](const GoomDrawables fx)
                                                      { ResetCurrentDrawBuffSettings(fx); });
}

auto GoomAllVisualFx::Start() noexcept -> void
{
  ChangeAllFxPixelBlenders();

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

auto GoomAllVisualFx::RefreshAllFx() noexcept -> void
{
  m_allStandardVisualFx->RefreshAllFx();
}

auto GoomAllVisualFx::ChangeAllFxColorMaps() noexcept -> void
{
  m_allStandardVisualFx->ChangeColorMaps();
}

auto GoomAllVisualFx::ChangeAllFxPixelBlenders() noexcept -> void
{
  m_allStandardVisualFx->ChangeAllFxPixelBlenders(GetNextPixelBlenderParams());
}

auto GoomAllVisualFx::GetNextPixelBlenderParams() const noexcept -> IVisualFx::PixelBlenderParams
{
  switch (m_globalBlendTypeWeight.GetRandomWeighted())
  {
    case GlobalBlendType::NONRANDOM:
      return {false, RandomPixelBlender::PixelBlendType::ADD};
    case GlobalBlendType::ASYNC_RANDOM:
      return {true};
    case GlobalBlendType::SYNC_RANDOM:
      return {false, RandomPixelBlender::GetRandomPixelBlendType(*m_goomRand)};
    default:
      FailFast();
  }
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
