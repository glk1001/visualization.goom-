#include "goom_all_visual_fx.h"

#include "goom_plugin_info.h"
#include "sound_info.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/name_value_pairs.h"
#include "utils/parallel_utils.h"
#include "v2d.h"
#include "visual_fx/filters/filter_buffers_service.h"
#include "visual_fx/filters/filter_colors_service.h"
#include "visual_fx/flying_stars_fx.h"
#include "visual_fx/goom_dots_fx.h"
#include "visual_fx/ifs_dancers_fx.h"
#include "visual_fx/image_fx.h"
#include "visual_fx/tentacles_fx.h"
#include "visual_fx/tube_fx.h"

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

using CONTROL::GoomDrawable;
using DRAW::IGoomDraw;
using FILTERS::FilterBuffersService;
using FILTERS::FilterColorsService;
using FILTERS::ZoomFilterSettings;
using UTILS::NameValuePairs;
using UTILS::Parallel;
using UTILS::SmallImageBitmaps;
using VISUAL_FX::FlyingStarsFx;
using VISUAL_FX::GoomDotsFx;
using VISUAL_FX::IfsDancersFx;
using VISUAL_FX::ImageFx;
using VISUAL_FX::LinesFx;
using VISUAL_FX::TentaclesFx;
using VISUAL_FX::TubeFx;
using VISUAL_FX::ZoomFilterFx;

static const Pixel RED_LINE = LinesFx::GetRedLineColor();
static const Pixel GREEN_LINE = LinesFx::GetGreenLineColor();
static const Pixel BLACK_LINE = LinesFx::GetBlackLineColor();

GoomAllVisualFx::GoomAllVisualFx(Parallel& parallel,
                                 const IGoomDraw& draw,
                                 const PluginInfo& goomInfo,
                                 const SmallImageBitmaps& smallBitmaps,
                                 const std::string& resourcesDirectory,
                                 std::unique_ptr<FilterBuffersService> filterBuffersService,
                                 std::unique_ptr<FilterColorsService> filterColorsService) noexcept
  : m_zoomFilter_fx{std::make_shared<ZoomFilterFx>(
        parallel, goomInfo, std::move(filterBuffersService), std::move(filterColorsService))},
    m_star_fx{std::make_shared<FlyingStarsFx>(draw, goomInfo, smallBitmaps)},
    m_goomDots_fx{std::make_shared<GoomDotsFx>(draw, goomInfo, smallBitmaps)},
    m_ifs_fx{std::make_shared<IfsDancersFx>(draw, goomInfo, smallBitmaps)},
    m_image_fx{std::make_shared<ImageFx>(draw, goomInfo, resourcesDirectory)},
    m_tentacles_fx{std::make_shared<TentaclesFx>(draw, goomInfo)},
    m_tube_fx{std::make_shared<TubeFx>(draw, goomInfo, smallBitmaps)},
    m_goomLine1{std::make_shared<LinesFx>(draw,
                                          goomInfo,
                                          smallBitmaps,
                                          LinesFx::LineType::H_LINE,
                                          static_cast<float>(goomInfo.GetScreenInfo().height),
                                          BLACK_LINE,
                                          LinesFx::LineType::CIRCLE,
                                          INITIAL_SCREEN_HEIGHT_FRACTION_LINE1 *
                                              static_cast<float>(goomInfo.GetScreenInfo().height),
                                          GREEN_LINE)},
    m_goomLine2{std::make_shared<LinesFx>(draw,
                                          goomInfo,
                                          smallBitmaps,
                                          LinesFx::LineType::H_LINE,
                                          0,
                                          BLACK_LINE,
                                          LinesFx::LineType::CIRCLE,
                                          INITIAL_SCREEN_HEIGHT_FRACTION_LINE2 *
                                              static_cast<float>(goomInfo.GetScreenInfo().height),
                                          RED_LINE)},
    // clang-format off
    m_list {
      m_zoomFilter_fx,
      m_star_fx,
      m_ifs_fx,
      m_image_fx,
      m_goomDots_fx,
      m_tentacles_fx,
      m_tube_fx,
    },
    m_drawablesMap {
      {GoomDrawable::STARS, m_star_fx},
      {GoomDrawable::IFS, m_ifs_fx},
      {GoomDrawable::IMAGE, m_image_fx},
      {GoomDrawable::DOTS, m_goomDots_fx},
      {GoomDrawable::TENTACLES, m_tentacles_fx},
      {GoomDrawable::TUBE, m_tube_fx},
    }
// clang-format on
{
}

void GoomAllVisualFx::Start()
{
  m_goomLine1->Start();
  m_goomLine2->Start();

  for (auto& v : m_list)
  {
    v->Start();
  }
}

void GoomAllVisualFx::Finish()
{
  for (auto& v : m_list)
  {
    v->Finish();
  }

  m_goomLine1->Finish();
  m_goomLine2->Finish();
}

void GoomAllVisualFx::ChangeState()
{
  SuspendFx();

  constexpr size_t MAX_TRIES = 10;
  const size_t oldStateIndex = m_state.GetCurrentStateIndex();

  for (size_t numTry = 0; numTry < MAX_TRIES; ++numTry)
  {
    m_state.DoRandomStateChange();
    // Pick a different state if possible
    if (oldStateIndex != m_state.GetCurrentStateIndex())
    {
      break;
    }
  }

  m_currentGoomDrawables = m_state.GetCurrentDrawables();

  ResumeFx();
}

void GoomAllVisualFx::PostStateUpdate(const std::unordered_set<GoomDrawable>& oldGoomDrawables)
{
  for (const auto& currentlyDrawable : m_currentGoomDrawables)
  {
    if (!CanDraw(currentlyDrawable))
    {
      continue;
    }
    const bool wasActiveInPreviousState =
        oldGoomDrawables.find(GoomDrawable::IFS) != oldGoomDrawables.end();
    m_drawablesMap.at(currentlyDrawable)->PostStateUpdate(wasActiveInPreviousState);
  }
}

void GoomAllVisualFx::SuspendFx()
{
  for (const auto& currentlyDrawable : m_currentGoomDrawables)
  {
    if (!CanDraw(currentlyDrawable))
    {
      continue;
    }
    m_drawablesMap.at(currentlyDrawable)->Suspend();
  }
}

void GoomAllVisualFx::ResumeFx()
{
  for (const auto& currentlyDrawable : m_currentGoomDrawables)
  {
    if (!CanDraw(currentlyDrawable))
    {
      continue;
    }
    m_drawablesMap.at(currentlyDrawable)->Resume();
  }
}

void GoomAllVisualFx::RefreshAll()
{
  for (const auto& currentlyDrawable : m_currentGoomDrawables)
  {
    if (!CanDraw(currentlyDrawable))
    {
      continue;
    }
    m_drawablesMap.at(currentlyDrawable)->Refresh();
  }
}

void GoomAllVisualFx::SetZoomMidPoint(const V2dInt& zoomMidPoint)
{
  for (const auto& currentlyDrawable : m_currentGoomDrawables)
  {
    if (!CanDraw(currentlyDrawable))
    {
      continue;
    }
    m_drawablesMap.at(currentlyDrawable)->SetZoomMidPoint(zoomMidPoint);
  }
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

  SetZoomMidPoint(filterSettings.filterEffectsSettings.zoomMidPoint);
}

void GoomAllVisualFx::DisplayGoomLines(const AudioSamples& soundData)
{
  assert(IsCurrentlyDrawable(GoomDrawable::LINES));

  m_goomLine2->SetPower(m_goomLine1->GetPower());

  m_goomLine1->DrawLines(soundData.GetSample(0), soundData.GetSampleMinMax(0));
  m_goomLine2->DrawLines(soundData.GetSample(1), soundData.GetSampleMinMax(1));
}

void GoomAllVisualFx::ApplyDotsIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawable::DOTS))
  {
    return;
  }
  if (!m_singleBufferDots)
  {
    return;
  }

  ResetDrawBuffSettings(m_state.GetCurrentBuffSettings(GoomDrawable::DOTS));
  m_goomDots_fx->ApplySingle();
}

void GoomAllVisualFx::ApplyDotsToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawable::DOTS))
  {
    return;
  }
  if (m_singleBufferDots)
  {
    return;
  }

  ResetDrawBuffSettings(m_state.GetCurrentBuffSettings(GoomDrawable::DOTS));
  m_goomDots_fx->ApplyMultiple();
}

void GoomAllVisualFx::ApplyIfsToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawable::IFS))
  {
    m_ifs_fx->ApplyNoDraw();
    return;
  }

  ResetDrawBuffSettings(m_state.GetCurrentBuffSettings(GoomDrawable::IFS));
  m_ifs_fx->ApplyMultiple();
}

void GoomAllVisualFx::ApplyImageToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawable::IMAGE))
  {
    return;
  }

  ResetDrawBuffSettings(m_state.GetCurrentBuffSettings(GoomDrawable::IMAGE));
  m_image_fx->ApplyMultiple();
}

void GoomAllVisualFx::ApplyTentaclesToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawable::TENTACLES))
  {
    m_tentacles_fx->ApplyNoDraw();
    return;
  }

  ResetDrawBuffSettings(m_state.GetCurrentBuffSettings(GoomDrawable::TENTACLES));
  m_tentacles_fx->ApplyMultiple();
}

void GoomAllVisualFx::ApplyTubeToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawable::TUBE))
  {
    m_tube_fx->ApplyNoDraw();
    return;
  }

  ResetDrawBuffSettings(m_state.GetCurrentBuffSettings(GoomDrawable::TUBE));
  m_tube_fx->ApplyMultiple();
}

void GoomAllVisualFx::ApplyStarsToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawable::STARS))
  {
    return;
  }

  ResetDrawBuffSettings(m_state.GetCurrentBuffSettings(GoomDrawable::STARS));
  m_star_fx->ApplyMultiple();
}

inline void GoomAllVisualFx::ResetDrawBuffSettings(const FXBuffSettings& settings)
{
  m_resetDrawBuffSettings(settings);
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
