#include "goom_all_visual_fx.h"

#include "all_standard_visual_fx.h"
#include "color/colormaps.h"
#include "color/colorutils.h"
#include "draw/goom_draw.h"
#include "goom_plugin_info.h"
#include "sound_info.h"
#include "utils/name_value_pairs.h"
#include "visual_fx/filters/filter_buffers_service.h"
#include "visual_fx/filters/filter_colors_service.h"
#include "visual_fx/fx_helpers.h"
#include "visual_fx_color_maps.h"

//#undef NDEBUG
#include <cassert>
#include <memory>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

using COLOR::GetBrighterColor;
using COLOR::GetBrighterColorInt;
using COLOR::GetLuma;
using COLOR::IColorMap;
using CONTROL::GoomDrawables;
using DRAW::IGoomDraw;
using FILTERS::FilterBuffersService;
using FILTERS::FilterColorsService;
using FILTERS::ZoomFilterSettings;
using UTILS::NameValuePairs;
using UTILS::Parallel;
using UTILS::SmallImageBitmaps;
using VISUAL_FX::FxHelpers;
using VISUAL_FX::LinesFx;
using VISUAL_FX::ZoomFilterFx;

static const Pixel RED_LINE = LinesFx::GetRedLineColor();
static const Pixel GREEN_LINE = LinesFx::GetGreenLineColor();
static const Pixel BLACK_LINE = LinesFx::GetBlackLineColor();

GoomAllVisualFx::GoomAllVisualFx(Parallel& parallel,
                                 const FxHelpers& fxHelpers,
                                 const SmallImageBitmaps& smallBitmaps,
                                 const std::string& resourcesDirectory,
                                 IGoomStateHandler& goomStateHandler,
                                 std::unique_ptr<FilterBuffersService> filterBuffersService,
                                 std::unique_ptr<FilterColorsService> filterColorsService) noexcept
  : m_allStandardVisualFx{spimpl::make_unique_impl<AllStandardVisualFx>(
        parallel, fxHelpers, smallBitmaps, resourcesDirectory)},
    m_zoomFilter_fx{std::make_unique<ZoomFilterFx>(parallel,
                                                   fxHelpers.GetGoomInfo(),
                                                   std::move(filterBuffersService),
                                                   std::move(filterColorsService))},
    m_goomLine1{std::make_unique<LinesFx>(
        fxHelpers,
        smallBitmaps,
        LinesFx::LineType::H_LINE,
        static_cast<float>(fxHelpers.GetGoomInfo().GetScreenInfo().height),
        BLACK_LINE,
        LinesFx::LineType::CIRCLE,
        INITIAL_SCREEN_HEIGHT_FRACTION_LINE1 *
            static_cast<float>(fxHelpers.GetGoomInfo().GetScreenInfo().height),
        GREEN_LINE)},
    m_goomLine2{std::make_unique<LinesFx>(
        fxHelpers,
        smallBitmaps,
        LinesFx::LineType::H_LINE,
        0,
        BLACK_LINE,
        LinesFx::LineType::CIRCLE,
        INITIAL_SCREEN_HEIGHT_FRACTION_LINE2 *
            static_cast<float>(fxHelpers.GetGoomInfo().GetScreenInfo().height),
        RED_LINE)},
    m_goomDraw{fxHelpers.GetDraw()},
    m_goomRand{fxHelpers.GetGoomRand()},
    m_goomStateHandler{goomStateHandler},
    m_visualFxColorMaps{fxHelpers.GetGoomRand()}
{
  m_allStandardVisualFx->SetResetDrawBuffSettingsFunc([&](const GoomDrawables fx)
                                                      { ResetCurrentDrawBuffSettings(fx); });
}

void GoomAllVisualFx::Start()
{
  m_goomLine1->Start();
  m_goomLine2->Start();

  m_zoomFilter_fx->Start();

  m_allStandardVisualFx->Start();
}

void GoomAllVisualFx::Finish()
{
  m_allStandardVisualFx->Finish();

  m_zoomFilter_fx->Finish();

  m_goomLine1->Finish();
  m_goomLine2->Finish();
}

void GoomAllVisualFx::ChangeState()
{
  m_allStandardVisualFx->SuspendFx();

  constexpr size_t MAX_TRIES = 10;
  const GoomStates oldState = m_goomStateHandler.GetCurrentState();

  for (size_t numTry = 0; numTry < MAX_TRIES; ++numTry)
  {
    m_goomStateHandler.ChangeToNextState();
    // Pick a different state if possible
    if (oldState != m_goomStateHandler.GetCurrentState())
    {
      break;
    }
  }

  m_currentGoomDrawables = m_goomStateHandler.GetCurrentDrawables();
  m_allStandardVisualFx->SetCurrentGoomDrawables(m_currentGoomDrawables);

  m_allStandardVisualFx->ResumeFx();
}

void GoomAllVisualFx::StartShaderExposureControl()
{
  m_allStandardVisualFx->StartShaderExposureControl();
}

auto GoomAllVisualFx::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_allStandardVisualFx->GetLastShaderEffects();
}

void GoomAllVisualFx::SetSingleBufferDots(const bool value)
{
  m_allStandardVisualFx->SetSingleBufferDots(value);
}

void GoomAllVisualFx::PostStateUpdate(const std::unordered_set<GoomDrawables>& oldGoomDrawables)
{
  m_allStandardVisualFx->PostStateUpdate(oldGoomDrawables);
}

void GoomAllVisualFx::RefreshAllFx()
{
  m_allStandardVisualFx->RefreshAllFx();
}

inline void GoomAllVisualFx::ResetCurrentDrawBuffSettings(const GoomDrawables fx)
{
  m_resetDrawBuffSettings(GetCurrentBuffSettings(fx));
}

inline auto GoomAllVisualFx::GetCurrentBuffSettings(const GoomDrawables fx) const -> FXBuffSettings
{
  return GoomStateInfo::GetBuffSettings(m_goomStateHandler.GetCurrentState(), fx);
}

void GoomAllVisualFx::ChangeAllFxColorMaps()
{
  m_allStandardVisualFx->ChangeColorMaps();
  ChangeLineColorMaps();
}

void GoomAllVisualFx::ChangeDrawPixelBlend()
{
  if (m_goomRand.ProbabilityOf(1.0F))
  {
    m_goomDraw.SetDefaultBlendPixelFunc();
  }
  else if (m_goomRand.ProbabilityOf(0.0F))
  {
    m_goomDraw.SetBlendPixelFunc(GetSameLumaBlendPixelFunc());
  }
  else if (m_goomRand.ProbabilityOf(0.0F))
  {
    m_goomDraw.SetBlendPixelFunc(GetSameLumaMixBlendPixelFunc());
  }
  else
  {
    m_goomDraw.SetBlendPixelFunc(GetReverseColorAddBlendPixelPixelFunc());
  }
}

auto GoomAllVisualFx::GetReverseColorAddBlendPixelPixelFunc() -> IGoomDraw::BlendPixelFunc
{
  return [](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  { return COLOR::GetColorAdd(COLOR::GetBrighterColorInt(intBuffIntensity, oldColor), newColor); };
}

auto GoomAllVisualFx::GetSameLumaBlendPixelFunc() -> IGoomDraw::BlendPixelFunc
{
  return [](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  {
    const float newColorLuma =
        GetLuma(newColor) * (static_cast<float>(intBuffIntensity) / channel_limits<float>::max());
    if (newColorLuma < 0.1F)
    {
      return COLOR::GetColorAdd(oldColor, newColor);
    }
    const float oldColorLuma = GetLuma(oldColor);
    const float brightness = 1.0F + (oldColorLuma / newColorLuma);

    const auto red = static_cast<uint32_t>(brightness * static_cast<float>(newColor.R()));
    const auto green = static_cast<uint32_t>(brightness * static_cast<float>(newColor.G()));
    const auto blue = static_cast<uint32_t>(brightness * static_cast<float>(newColor.B()));

    return Pixel{red, green, blue, MAX_ALPHA};
  };
}

auto GoomAllVisualFx::GetSameLumaMixBlendPixelFunc() -> IGoomDraw::BlendPixelFunc
{
  return [](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  {
    const float newColorLuma =
        GetLuma(newColor) * (static_cast<float>(intBuffIntensity) / channel_limits<float>::max());
    if (newColorLuma < 0.1F)
    {
      return COLOR::GetColorAdd(oldColor, newColor);
    }
    const float oldColorLuma = GetLuma(oldColor);
    const float brightness = 0.5F * (1.0F + (oldColorLuma / newColorLuma));

    const Pixel finalNewColor = IColorMap::GetColorMix(oldColor, newColor, 0.7F);
    const auto red = static_cast<uint32_t>(brightness * static_cast<float>(finalNewColor.R()));
    const auto green = static_cast<uint32_t>(brightness * static_cast<float>(finalNewColor.G()));
    const auto blue = static_cast<uint32_t>(brightness * static_cast<float>(finalNewColor.B()));

    return Pixel{red, green, blue, MAX_ALPHA};
  };
}

void GoomAllVisualFx::UpdateFilterSettings(const ZoomFilterSettings& filterSettings,
                                           const bool updateFilterEffects)
{
  if (updateFilterEffects)
  {
    m_zoomFilter_fx->UpdateFilterEffectsSettings(filterSettings.filterEffectsSettings);
  }

  m_zoomFilter_fx->UpdateFilterBufferSettings(filterSettings.filterBufferSettings);
  m_zoomFilter_fx->UpdateFilterColorSettings(filterSettings.filterColorSettings);

  m_allStandardVisualFx->SetZoomMidPoint(filterSettings.filterEffectsSettings.zoomMidPoint);
}

void GoomAllVisualFx::ApplyCurrentStateToSingleBuffer()
{
  m_allStandardVisualFx->ApplyCurrentStateToSingleBuffer();
}

void GoomAllVisualFx::ApplyCurrentStateToMultipleBuffers()
{
  m_allStandardVisualFx->ApplyCurrentStateToMultipleBuffers();
}

void GoomAllVisualFx::DisplayGoomLines(const AudioSamples& soundData)
{
  assert(IsCurrentlyDrawable(GoomDrawables::LINES));

  m_goomLine2->SetPower(m_goomLine1->GetPower());

  m_goomLine1->DrawLines(soundData.GetSample(0), soundData.GetSampleMinMax(0));
  m_goomLine2->DrawLines(soundData.GetSample(1), soundData.GetSampleMinMax(1));
}

void GoomAllVisualFx::ChangeLineColorMaps()
{
  m_visualFxColorMaps.SetNextColorMapSet();

  m_goomLine1->SetWeightedColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::LINES1));
  m_goomLine2->SetWeightedColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::LINES2));
}

auto GoomAllVisualFx::GetZoomFilterFxNameValueParams() const -> NameValuePairs
{
  return m_zoomFilter_fx->GetNameValueParams();
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
