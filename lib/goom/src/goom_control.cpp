/**
* file: goom_control.c (onetime goom_core.c)
 * author: Jean-Christophe Hoelt (which is not so proud of it)
 *
 * Contains the core of goom's work.
 *
 * (c)2000-2003, by iOS-software.
 *
 *  - converted to C++14 2021-02-01 (glk)
 *
 */

//#include <valgrind/callgrind.h>

#include "goom_control.h"

#include "control/goom_events.h"
#include "control/goom_image_buffers.h"
#include "control/goom_lock.h"
#include "control/goom_states.h"
#include "control/goom_title_display.h"
#include "convolve_fx.h"
#include "draw/goom_draw_to_buffer.h"
#include "draw/text_draw.h"
#include "filters.h"
#include "filters/filter_buffers_service.h"
#include "filters/filter_colors_service.h"
#include "filters/filter_settings.h"
#include "filters/filter_settings_service.h"
#include "flying_stars_fx.h"
#include "goom_config.h"
#include "goom_dots_fx.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "goom_visual_fx.h"
#include "goomutils/colormaps.h"
#include "goomutils/enumutils.h"
#include "goomutils/goomrand.h"
#include "goomutils/logging_control.h"
#undef NO_LOGGING
#include "goomutils/graphics/small_image_bitmaps.h"
#include "goomutils/logging.h"
#include "goomutils/name_value_pairs.h"
#include "goomutils/parallel_utils.h"
#include "goomutils/random_colormaps.h"
#include "goomutils/spimpl.h"
#include "goomutils/strutils.h"
#include "goomutils/t_values.h"
#include "goomutils/timer.h"
#include "ifs_dancers_fx.h"
#include "lines_fx.h"
#include "tentacles_fx.h"
#include "tube_fx.h"

#include <array>
#include <cmath>
#include <cstdint>
#if __cplusplus > 201402L
#include <filesystem>
#endif
//#undef NDEBUG
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#if __cplusplus > 201402L
#include <variant>
#endif
#include <goomutils/colormaps.h>
#include <vector>

//#define SHOW_STATE_TEXT_ON_SCREEN

namespace GOOM
{

using CONTROL::GoomDrawable;
using CONTROL::GoomEvents;
using CONTROL::GoomImageBuffers;
using CONTROL::GoomLock;
using CONTROL::GoomStates;
using CONTROL::GoomTitleDisplay;
using DRAW::GoomDrawToBuffer;
using DRAW::TextDraw;
using FILTERS::FilterBuffersService;
using FILTERS::FilterColorsService;
using FILTERS::FilterSettingsService;
using FILTERS::Vitesse;
using UTILS::ColorMapGroup;
using UTILS::GetAllMapsUnweighted;
using UTILS::GetAllSlimMaps;
using UTILS::GetAllStandardMaps;
using UTILS::GetBlueStandardMaps;
using UTILS::GetCitiesStandardMaps;
using UTILS::GetColdStandardMaps;
using UTILS::GetGreenStandardMaps;
using UTILS::GetHeatStandardMaps;
using UTILS::GetMostlySequentialSlimMaps;
using UTILS::GetMostlySequentialStandardMaps;
using UTILS::GetNameValuesString;
using UTILS::GetOrangeStandardMaps;
using UTILS::GetPastelStandardMaps;
using UTILS::GetPurpleStandardMaps;
using UTILS::GetRandInRange;
using UTILS::GetRedStandardMaps;
using UTILS::GetSeasonsStandardMaps;
using UTILS::GetSlightlyDivergingSlimMaps;
using UTILS::GetSlightlyDivergingStandardMaps;
using UTILS::GetYellowStandardMaps;
using UTILS::IColorMap;
using UTILS::Logging;
using UTILS::NUM;
using UTILS::Parallel;
using UTILS::ProbabilityOfMInN;
using UTILS::RandomColorMaps;
using UTILS::SmallImageBitmaps;
using UTILS::StringSplit;
using UTILS::Timer;
using UTILS::TValue;

// TODO: put that as variable in PluginInfo
constexpr int32_t MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE = 200;

using GoomEvent = GoomEvents::GoomEvent;

struct GoomVisualFx
{
  GoomVisualFx() noexcept = delete;
  GoomVisualFx(Parallel& parallel,
               const IGoomDraw& draw,
               const std::shared_ptr<const PluginInfo>& goomInfo,
               const SmallImageBitmaps& smallBitmaps,
               std::unique_ptr<FilterBuffersService> filterBuffersService,
               std::unique_ptr<FilterColorsService> filterColorsService) noexcept;

  std::shared_ptr<ConvolveFx> convolve_fx;
  std::shared_ptr<ZoomFilterFx> zoomFilter_fx;
  std::shared_ptr<FlyingStarsFx> star_fx;
  std::shared_ptr<GoomDotsFx> goomDots_fx;
  std::shared_ptr<IfsDancersFx> ifs_fx;
  std::shared_ptr<TentaclesFx> tentacles_fx;
  std::shared_ptr<TubeFx> tube_fx;

  std::vector<std::shared_ptr<IVisualFx>> list;
  std::map<GoomDrawable, std::shared_ptr<IVisualFx>> map;
};

GoomVisualFx::GoomVisualFx(Parallel& parallel,
                           const IGoomDraw& draw,
                           const std::shared_ptr<const PluginInfo>& goomInfo,
                           const SmallImageBitmaps& smallBitmaps,
                           std::unique_ptr<FilterBuffersService> filterBuffersService,
                           std::unique_ptr<FilterColorsService> filterColorsService) noexcept
  : convolve_fx{std::make_shared<ConvolveFx>(parallel, goomInfo)},
    zoomFilter_fx{std::make_shared<ZoomFilterFx>(
        parallel, goomInfo, std::move(filterBuffersService), std::move(filterColorsService))},
    star_fx{std::make_shared<FlyingStarsFx>(draw, goomInfo, smallBitmaps)},
    goomDots_fx{std::make_shared<GoomDotsFx>(draw, goomInfo, smallBitmaps)},
    ifs_fx{std::make_shared<IfsDancersFx>(draw, goomInfo, smallBitmaps)},
    tentacles_fx{std::make_shared<TentaclesFx>(draw, goomInfo)},
    tube_fx{std::make_shared<TubeFx>(draw, goomInfo, smallBitmaps)},
    // clang-format off
    list{
      convolve_fx,
      zoomFilter_fx,
      star_fx,
      ifs_fx,
      goomDots_fx,
      tentacles_fx,
      tube_fx,
    },
    map{
      {GoomDrawable::STARS, star_fx},
      {GoomDrawable::IFS, ifs_fx},
      {GoomDrawable::DOTS, goomDots_fx},
      {GoomDrawable::TENTACLES, tentacles_fx},
      {GoomDrawable::TUBE, tube_fx},
    }
// clang-format on
{
}

struct GoomData
{
  GoomLock lock{}; // pour empecher de nouveaux changements
  int32_t stopLines = 0;
  int32_t updatesSinceLastZoomEffectsChange = 0; // nombre de Cycle Depuis Dernier Changement
  // duree de la transition entre afficher les lignes ou pas
  int32_t drawLinesDuration = LinesFx::MIN_LINE_DURATION;
  int32_t lineMode = LinesFx::MIN_LINE_DURATION; // l'effet lineaire a dessiner

  uint32_t stateSelectionBlocker = 0;
  int32_t previousZoomSpeed = Vitesse::DEFAULT_VITESSE + 1;

  std::string title{};
};

class GoomControl::GoomControlImpl
{
public:
  GoomControlImpl(uint32_t screenWidth, uint32_t screenHeight, std::string resourcesDirectory);

  void Swap(GoomControl::GoomControlImpl& other) noexcept = delete;

  void SetScreenBuffer(const std::shared_ptr<PixelBuffer>& buffer);

  [[nodiscard]] auto GetScreenWidth() const -> uint32_t;
  [[nodiscard]] auto GetScreenHeight() const -> uint32_t;

  void Start();
  void Finish();

  void Update(const AudioSamples& soundData,
              float fps,
              const std::string& songTitle,
              const std::string& message);

private:
  Parallel m_parallel{-1}; // max cores - 1
  GoomDrawToBuffer m_multiBufferDraw;
  const std::shared_ptr<WritablePluginInfo> m_goomInfo;
  GoomImageBuffers m_imageBuffers;
  const std::string m_resourcesDirectory;
  FilterSettingsService m_filterSettingsService;
  const SmallImageBitmaps m_smallBitmaps;
  GoomVisualFx m_visualFx;
  GoomStates m_states{};
  GoomEvents m_goomEvent{};
  uint32_t m_updateNum = 0;
  uint32_t m_timeInState = 0;
  uint32_t m_cycle = 0;
  std::unordered_set<GoomDrawable> m_curGDrawables{};
  GoomData m_goomData{};

  static constexpr uint32_t MIN_UPDATES_TO_LOG = 6;

  bool m_singleBufferDots = true;
  static constexpr uint32_t MIN_NUM_OVEREXPOSED_UPDATES = 1000;
  static constexpr uint32_t MAX_NUM_OVEREXPOSED_UPDATES = 10000;
  Timer m_convolveAllowOverexposed{MIN_NUM_OVEREXPOSED_UPDATES};
  static constexpr uint32_t MIN_NUM_NOT_OVEREXPOSED_UPDATES = 10000;
  static constexpr uint32_t MAX_NUM_NOT_OVEREXPOSED_UPDATES = 10000;
  Timer m_convolveNotAllowOverexposed{0};

  // Line Fx
  static constexpr float INITIAL_SCREEN_HEIGHT_FRACTION_LINE1 = 0.4F;
  static constexpr float INITIAL_SCREEN_HEIGHT_FRACTION_LINE2 = 0.2F;
  LinesFx m_goomLine1;
  LinesFx m_goomLine2;

  void UpdateTimers();
  void ChangeColorMaps();
  void ProcessAudio(const AudioSamples& soundData) const;

  // Changement d'effet de zoom !
  void ChangeZoomEffect();
  void UpdateFilterSettings();
  void ApplyZoom();
  void UpdateBuffers();
  void RotateDrawBuffers();
  [[nodiscard]] auto GetCurrentBuffers() const -> std::vector<PixelBuffer*>;
  void ResetDrawBuffSettings(const FXBuffSettings& settings);

  void ApplyCurrentStateToSingleBuffer();
  void ApplyCurrentStateToMultipleBuffers();
  void ApplyDotsIfRequired();
  void ApplyDotsToBothBuffersIfRequired();
  void ApplyIfsToBothBuffersIfRequired();
  void ApplyTentaclesToBothBuffersIfRequired();
  void ApplyStarsToBothBuffersIfRequired();
  void ApplyTubeToBothBuffersIfRequired();

  void DoIfsRenew();

  static constexpr uint32_t NORMAL_UPDATE_LOCK_TIME = 50;
  static constexpr uint32_t REVERSE_SPEED_AND_STOP_SPEED_LOCK_TIME = 75;
  static constexpr uint32_t REVERSE_SPEED_LOCK_TIME = 100;
  static constexpr uint32_t MEGA_LENT_LOCK_TIME_INCREASE = 50;
  static constexpr uint32_t CHANGE_VITESSE_LOCK_TIME_INCREASE = 50;
  static constexpr uint32_t CHANGE_SWITCH_VALUES_LOCK_TIME = 150;
  [[nodiscard]] auto IsLocked() const -> bool;
  [[nodiscard]] auto GetLockTime() const -> uint32_t;
  void UpdateLock();
  void SetLockTime(uint32_t val);
  void IncreaseLockTime(uint32_t byAmount);

  void ChangeState();
  void DoChangeState();
  void SuspendCurrentState();
  void SetNextState();
  void ResumeCurrentState();

  void BigNormalUpdate();
  void MegaLentUpdate();

  void ChangeAllowOverexposed();
  void ChangeBlockyWavy();
  void ChangeClippedColor();
  void ChangeNoise();
  void ChangeRotation();
  void ChangeSwitchValues();
  void ChangeSpeedReverse();
  void ChangeVitesse();
  void ChangeStopSpeeds();

  static constexpr uint32_t NUM_BLOCKY_WAVY_UPDATES = 100;
  Timer m_blockyWavyTimer{NUM_BLOCKY_WAVY_UPDATES};
  static constexpr uint32_t NUM_NOISE_UPDATES = 100;
  Timer m_noiseTimer{NUM_NOISE_UPDATES};
  static constexpr uint32_t NUM_ALLOW_OVEREXPOSED_UPDATES = 100;
  Timer m_allowOverexposedTimer{NUM_ALLOW_OVEREXPOSED_UPDATES};

  std::reference_wrapper<const IColorMap> m_clippedColorMap{
      RandomColorMaps().GetRandomColorMap(ColorMapGroup::DIVERGING_BLACK)};
  TValue m_clippedT{TValue::StepType::CONTINUOUS_REVERSIBLE, 1000U};

  // on verifie qu'il ne se pas un truc interressant avec le son.
  void ChangeFilterModeIfMusicChanges();
  [[nodiscard]] auto ChangeFilterModeEventHappens() -> bool;
  void ChangeFilterMode();
  void ChangeMilieu();

  // baisser regulierement la vitesse
  void RegularlyLowerTheSpeed();

  // arret demande
  void StopLinesRequest();
  void StopLinesIfRequested();

  // arret aleatore.. changement de mode de ligne..
  void StopRandomLineChangeMode();

  // arreter de decrémenter au bout d'un certain temps
  void StopDecrementingAfterAWhile();
  void StopDecrementing();

  // tout ceci ne sera fait qu'en cas de non-blocage
  void BigUpdateIfNotLocked();
  void BigUpdate();

  // gros frein si la musique est calme
  void BigBreakIfMusicIsCalm();
  void BigBreak();

  void DisplayLinesIfInAGoom(const AudioSamples& soundData);
  void DisplayLines(const AudioSamples& soundData);
  void ChooseGoomLine(float* param1,
                      float* param2,
                      Pixel* couleur,
                      LinesFx::LineType* mode,
                      float* amplitude,
                      int farVal);

  void DisplayTitle(const std::string& songTitle, const std::string& message, float fps);
  void UpdateMessages(const std::string& messages);

  std::unique_ptr<GoomTitleDisplay> m_goomTitleDisplay{};
  GoomDrawToBuffer m_goomTextOutput;
  std::unique_ptr<TextDraw> m_updateMessagesDisplay{};
  std::string m_updateMessagesFontFile{};
  auto GetFontDirectory() const;
#ifdef SHOW_STATE_TEXT_ON_SCREEN
  void DisplayStateText();
#endif
};

auto GoomControl::GetRandSeed() -> uint64_t
{
  return GOOM::UTILS::GetRandSeed();
}

void GoomControl::SetRandSeed(const uint64_t seed)
{
  LogDebug("Set goom seed = {}.", seed);
  GOOM::UTILS::SetRandSeed(seed);
}

GoomControl::GoomControl(const uint32_t width,
                         const uint32_t height,
                         const std::string& resourcesDirectory)
  : m_controller{spimpl::make_unique_impl<GoomControlImpl>(width, height, resourcesDirectory)}
{
}

void GoomControl::SetScreenBuffer(const std::shared_ptr<PixelBuffer>& buffer)
{
  m_controller->SetScreenBuffer(buffer);
}

void GoomControl::Start()
{
  m_controller->Start();
}

void GoomControl::Finish()
{
  m_controller->Finish();
}

void GoomControl::Update(const AudioSamples& s,
                         const float fps,
                         const std::string& songTitle,
                         const std::string& message)
{
  m_controller->Update(s, fps, songTitle, message);
}

static const Pixel RED_LINE = GetRedLineColor();
static const Pixel GREEN_LINE = GetGreenLineColor();
static const Pixel BLACK_LINE = GetBlackLineColor();

GoomControl::GoomControlImpl::GoomControlImpl(const uint32_t screenWidth,
                                              const uint32_t screenHeight,
                                              std::string resourcesDirectory)
  : m_multiBufferDraw{screenWidth, screenHeight},
    m_goomInfo{std::make_shared<WritablePluginInfo>(screenWidth, screenHeight)},
    m_imageBuffers{screenWidth, screenHeight},
    m_resourcesDirectory{std::move(resourcesDirectory)},
    m_filterSettingsService{m_parallel, m_goomInfo, m_resourcesDirectory},
    m_smallBitmaps{m_resourcesDirectory},
    m_visualFx{m_parallel,
               m_multiBufferDraw,
               std::const_pointer_cast<const PluginInfo>(
                   std::dynamic_pointer_cast<PluginInfo>(m_goomInfo)),
               m_smallBitmaps,
               m_filterSettingsService.GetFilterBuffersService(),
               FilterSettingsService::GetFilterColorsService()},
    m_goomLine1{m_multiBufferDraw,
                std::const_pointer_cast<const PluginInfo>(
                    std::dynamic_pointer_cast<PluginInfo>(m_goomInfo)),
                m_smallBitmaps,
                LinesFx::LineType::H_LINE,
                static_cast<float>(screenHeight),
                BLACK_LINE,
                LinesFx::LineType::CIRCLE,
                INITIAL_SCREEN_HEIGHT_FRACTION_LINE1 * static_cast<float>(screenHeight),
                GREEN_LINE},
    m_goomLine2{m_multiBufferDraw,
                std::const_pointer_cast<const PluginInfo>(
                    std::dynamic_pointer_cast<PluginInfo>(m_goomInfo)),
                m_smallBitmaps,
                LinesFx::LineType::H_LINE,
                0,
                BLACK_LINE,
                LinesFx::LineType::CIRCLE,
                INITIAL_SCREEN_HEIGHT_FRACTION_LINE2 * static_cast<float>(screenHeight),
                RED_LINE},
    m_goomTextOutput{screenWidth, screenHeight}
{
  LogDebug("Initialize goom: screenWidth = {}, screenHeight = {}.", screenWidth, screenHeight);

  RotateDrawBuffers();
}

inline auto GoomControl::GoomControlImpl::GetFontDirectory() const
{
  return m_resourcesDirectory + PATH_SEP + FONTS_DIR;
}

void GoomControl::GoomControlImpl::SetScreenBuffer(const std::shared_ptr<PixelBuffer>& buffer)
{
  m_imageBuffers.SetOutputBuff(buffer);
}

auto GoomControl::GoomControlImpl::GetScreenWidth() const -> uint32_t
{
  return m_goomInfo->GetScreenInfo().width;
}

auto GoomControl::GoomControlImpl::GetScreenHeight() const -> uint32_t
{
  return m_goomInfo->GetScreenInfo().height;
}

inline auto GoomControl::GoomControlImpl::ChangeFilterModeEventHappens() -> bool
{
  return m_goomEvent.Happens(GoomEvent::CHANGE_FILTER_MODE);
}

void GoomControl::GoomControlImpl::Start()
{
  if (m_resourcesDirectory.empty())
  {
    throw std::logic_error("Cannot start Goom - resource directory not set.");
  }

  m_updateNum = 0;
  m_timeInState = 0;

  ChangeColorMaps();

  m_updateMessagesFontFile = GetFontDirectory() + PATH_SEP + "verdana.ttf";
  m_curGDrawables = m_states.GetCurrentDrawables();

  // TODO MAKE line a visual FX
  // TODO Pass resource dir in constructor
  m_goomLine1.SetResourcesDirectory(m_resourcesDirectory);
  m_goomLine1.Start();
  m_goomLine2.SetResourcesDirectory(m_resourcesDirectory);
  m_goomLine2.Start();

  m_visualFx.convolve_fx->SetAllowOverexposed(true);
  m_convolveAllowOverexposed.ResetToZero();

  m_filterSettingsService.Start();
  UpdateFilterSettings();

  for (auto& v : m_visualFx.list)
  {
    v->Start();
  }

  DoChangeState();
}

enum class GoomEffect
{
  DOTS0 = 0,
  DOTS1,
  DOTS2,
  DOTS3,
  DOTS4,
  LINES1,
  LINES2,
  IFS,
  STARS,
  STARS_LOW,
  TENTACLES,
  TUBE,
  TUBE_LOW,
  _NUM // unused and must be last
};

struct GoomStateColorMatch
{
  GoomEffect effect;
  std::function<std::shared_ptr<RandomColorMaps>()> getColorMaps;
};
using GoomStateColorMatchedSet = std::array<GoomStateColorMatch, NUM<GoomEffect>>;
static const std::array<GoomStateColorMatchedSet, 8> GOOM_STATE_COLOR_MATCHED_SETS{{
    {{
        {GoomEffect::DOTS0, GetAllStandardMaps},
        {GoomEffect::DOTS1, GetAllStandardMaps},
        {GoomEffect::DOTS2, GetAllStandardMaps},
        {GoomEffect::DOTS3, GetAllStandardMaps},
        {GoomEffect::DOTS4, GetAllStandardMaps},
        {GoomEffect::LINES1, GetAllStandardMaps},
        {GoomEffect::LINES2, GetAllStandardMaps},
        {GoomEffect::IFS, GetAllStandardMaps},
        {GoomEffect::STARS, GetAllStandardMaps},
        {GoomEffect::STARS_LOW, GetAllStandardMaps},
        {GoomEffect::TENTACLES, GetAllStandardMaps},
        {GoomEffect::TUBE, GetAllStandardMaps},
        {GoomEffect::TUBE_LOW, GetAllStandardMaps},
    }},
    {{
        {GoomEffect::DOTS0, GetRedStandardMaps},
        {GoomEffect::DOTS1, GetBlueStandardMaps},
        {GoomEffect::DOTS2, GetGreenStandardMaps},
        {GoomEffect::DOTS3, GetYellowStandardMaps},
        {GoomEffect::DOTS4, GetSlightlyDivergingSlimMaps},
        {GoomEffect::LINES1, GetMostlySequentialStandardMaps},
        {GoomEffect::LINES2, GetSlightlyDivergingStandardMaps},
        {GoomEffect::IFS, GetSlightlyDivergingStandardMaps},
        {GoomEffect::STARS, GetMostlySequentialSlimMaps},
        {GoomEffect::STARS_LOW, GetSlightlyDivergingStandardMaps},
        {GoomEffect::TENTACLES, GetSlightlyDivergingSlimMaps},
        {GoomEffect::TUBE, GetSlightlyDivergingStandardMaps},
        {GoomEffect::TUBE_LOW, GetSlightlyDivergingSlimMaps},
    }},
    {{
        {GoomEffect::DOTS0, GetOrangeStandardMaps},
        {GoomEffect::DOTS1, GetPurpleStandardMaps},
        {GoomEffect::DOTS2, GetGreenStandardMaps},
        {GoomEffect::DOTS3, GetYellowStandardMaps},
        {GoomEffect::DOTS4, GetSlightlyDivergingSlimMaps},
        {GoomEffect::LINES1, GetSlightlyDivergingSlimMaps},
        {GoomEffect::LINES2, GetSlightlyDivergingStandardMaps},
        {GoomEffect::IFS, GetSlightlyDivergingSlimMaps},
        {GoomEffect::STARS, GetHeatStandardMaps},
        {GoomEffect::STARS_LOW, GetAllSlimMaps},
        {GoomEffect::TENTACLES, GetYellowStandardMaps},
        {GoomEffect::TUBE, GetYellowStandardMaps},
        {GoomEffect::TUBE_LOW, GetBlueStandardMaps},
    }},
    {{
        {GoomEffect::DOTS0, GetRedStandardMaps},
        {GoomEffect::DOTS1, GetBlueStandardMaps},
        {GoomEffect::DOTS2, GetOrangeStandardMaps},
        {GoomEffect::DOTS3, GetYellowStandardMaps},
        {GoomEffect::DOTS4, GetPastelStandardMaps},
        {GoomEffect::LINES1, GetAllSlimMaps},
        {GoomEffect::LINES2, GetBlueStandardMaps},
        {GoomEffect::IFS, GetColdStandardMaps},
        {GoomEffect::STARS, GetSlightlyDivergingSlimMaps},
        {GoomEffect::STARS_LOW, GetBlueStandardMaps},
        {GoomEffect::TENTACLES, GetMostlySequentialStandardMaps},
        {GoomEffect::TUBE, GetMostlySequentialStandardMaps},
        {GoomEffect::TUBE_LOW, GetHeatStandardMaps},
    }},
    {{
        {GoomEffect::DOTS0, GetRedStandardMaps},
        {GoomEffect::DOTS1, GetBlueStandardMaps},
        {GoomEffect::DOTS2, GetGreenStandardMaps},
        {GoomEffect::DOTS3, GetPurpleStandardMaps},
        {GoomEffect::DOTS4, GetSlightlyDivergingSlimMaps},
        {GoomEffect::LINES1, GetSlightlyDivergingStandardMaps},
        {GoomEffect::LINES2, GetRedStandardMaps},
        {GoomEffect::IFS, GetCitiesStandardMaps},
        {GoomEffect::STARS, GetBlueStandardMaps},
        {GoomEffect::STARS_LOW, GetMostlySequentialStandardMaps},
        {GoomEffect::TENTACLES, GetPurpleStandardMaps},
        {GoomEffect::TUBE, GetPurpleStandardMaps},
        {GoomEffect::TUBE_LOW, GetPastelStandardMaps},
    }},
    {{
        {GoomEffect::DOTS0, GetRedStandardMaps},
        {GoomEffect::DOTS1, GetBlueStandardMaps},
        {GoomEffect::DOTS2, GetGreenStandardMaps},
        {GoomEffect::DOTS3, GetYellowStandardMaps},
        {GoomEffect::DOTS4, GetSlightlyDivergingSlimMaps},
        {GoomEffect::LINES1, GetSlightlyDivergingStandardMaps},
        {GoomEffect::LINES2, GetRedStandardMaps},
        {GoomEffect::IFS, GetPastelStandardMaps},
        {GoomEffect::STARS, GetPastelStandardMaps},
        {GoomEffect::STARS_LOW, GetMostlySequentialStandardMaps},
        {GoomEffect::TENTACLES, GetSeasonsStandardMaps},
        {GoomEffect::TUBE, GetSeasonsStandardMaps},
        {GoomEffect::TUBE_LOW, GetColdStandardMaps},
    }},
    {{
        {GoomEffect::DOTS0, GetRedStandardMaps},
        {GoomEffect::DOTS1, GetBlueStandardMaps},
        {GoomEffect::DOTS2, GetGreenStandardMaps},
        {GoomEffect::DOTS3, GetYellowStandardMaps},
        {GoomEffect::DOTS4, GetHeatStandardMaps},
        {GoomEffect::LINES1, GetSlightlyDivergingStandardMaps},
        {GoomEffect::LINES2, GetRedStandardMaps},
        {GoomEffect::IFS, GetPastelStandardMaps},
        {GoomEffect::STARS, GetPastelStandardMaps},
        {GoomEffect::STARS_LOW, GetColdStandardMaps},
        {GoomEffect::TENTACLES, GetSeasonsStandardMaps},
        {GoomEffect::TUBE, GetSeasonsStandardMaps},
        {GoomEffect::TUBE_LOW, GetCitiesStandardMaps},
    }},
    {{
        {GoomEffect::DOTS0, GetRedStandardMaps},
        {GoomEffect::DOTS1, GetBlueStandardMaps},
        {GoomEffect::DOTS2, GetGreenStandardMaps},
        {GoomEffect::DOTS3, GetYellowStandardMaps},
        {GoomEffect::DOTS4, GetHeatStandardMaps},
        {GoomEffect::LINES1, GetSlightlyDivergingStandardMaps},
        {GoomEffect::LINES2, GetRedStandardMaps},
        {GoomEffect::IFS, GetPastelStandardMaps},
        {GoomEffect::STARS, GetPastelStandardMaps},
        {GoomEffect::STARS_LOW, GetAllMapsUnweighted},
        {GoomEffect::TENTACLES, GetGreenStandardMaps},
        {GoomEffect::TUBE, GetAllMapsUnweighted},
        {GoomEffect::TUBE_LOW, GetAllSlimMaps},
    }},
}};

inline auto GetNextColorMatchedSet() -> const GoomStateColorMatchedSet&
{
  return GOOM_STATE_COLOR_MATCHED_SETS[GetRandInRange(0U, GOOM_STATE_COLOR_MATCHED_SETS.size())];
}

void GoomControl::GoomControlImpl::ChangeColorMaps()
{
  m_clippedColorMap = RandomColorMaps().GetRandomColorMap(ColorMapGroup::DIVERGING_BLACK);

  const GoomStateColorMatchedSet& colorMatchedSet = GetNextColorMatchedSet();

  for (const auto& colorMatch : colorMatchedSet)
  {
    switch (colorMatch.effect)
    {
      case GoomEffect::DOTS0:
        m_visualFx.goomDots_fx->SetWeightedColorMaps(0, colorMatch.getColorMaps());
        break;
      case GoomEffect::DOTS1:
        m_visualFx.goomDots_fx->SetWeightedColorMaps(1, colorMatch.getColorMaps());
        break;
      case GoomEffect::DOTS2:
        m_visualFx.goomDots_fx->SetWeightedColorMaps(2, colorMatch.getColorMaps());
        break;
      case GoomEffect::DOTS3:
        m_visualFx.goomDots_fx->SetWeightedColorMaps(3, colorMatch.getColorMaps());
        break;
      case GoomEffect::DOTS4:
        m_visualFx.goomDots_fx->SetWeightedColorMaps(4, colorMatch.getColorMaps());
        break;
      case GoomEffect::IFS:
        m_visualFx.ifs_fx->SetWeightedColorMaps(colorMatch.getColorMaps());
        break;
      case GoomEffect::LINES1:
        m_goomLine1.SetWeightedColorMaps(colorMatch.getColorMaps());
        break;
      case GoomEffect::LINES2:
        m_goomLine2.SetWeightedColorMaps(GetAllStandardMaps());
        break;
      case GoomEffect::STARS:
        m_visualFx.star_fx->SetWeightedColorMaps(colorMatch.getColorMaps());
        break;
      case GoomEffect::STARS_LOW:
        m_visualFx.star_fx->SetWeightedLowColorMaps(colorMatch.getColorMaps());
        break;
      case GoomEffect::TENTACLES:
        m_visualFx.tentacles_fx->SetWeightedColorMaps(colorMatch.getColorMaps());
        break;
      case GoomEffect::TUBE:
        m_visualFx.tube_fx->SetWeightedColorMaps(colorMatch.getColorMaps());
        break;
      case GoomEffect::TUBE_LOW:
        m_visualFx.tube_fx->SetWeightedLowColorMaps(colorMatch.getColorMaps());
        break;
      default:
        break;
    }
  }
}

void GoomControl::GoomControlImpl::Finish()
{
  for (auto& v : m_visualFx.list)
  {
    v->Finish();
  }

  m_updateNum = 0;
}

void GoomControl::GoomControlImpl::Update(const AudioSamples& soundData,
                                          const float fps,
                                          const std::string& songTitle,
                                          const std::string& message)
{
  //  CALLGRIND_START_INSTRUMENTATION;

  ++m_updateNum;

  UpdateTimers();

  // Elargissement de l'intervalle d'évolution des points!
  // Calcul du deplacement des petits points ...
  // Widening of the interval of evolution of the points!
  // Calculation of the displacement of small points ...

  ProcessAudio(soundData);

  UpdateLock();

  ChangeFilterModeIfMusicChanges();
  BigUpdateIfNotLocked();
  BigBreakIfMusicIsCalm();

  RegularlyLowerTheSpeed();
  StopDecrementingAfterAWhile();
  ChangeZoomEffect();

  ApplyCurrentStateToSingleBuffer();

  ApplyZoom();

  ApplyCurrentStateToMultipleBuffers();

  // Gestion du Scope - Scope management
  StopLinesIfRequested();
  StopRandomLineChangeMode();
  DisplayLinesIfInAGoom(soundData);

  UpdateBuffers();

#ifdef SHOW_STATE_TEXT_ON_SCREEN
  DisplayStateText();
#endif
  DisplayTitle(songTitle, message, fps);

  ++m_cycle;

  //  CALLGRIND_STOP_INSTRUMENTATION;
  //  CALLGRIND_DUMP_STATS;
}

void GoomControl::GoomControlImpl::UpdateTimers()
{
  ++m_timeInState;

  m_convolveAllowOverexposed.Increment();
  m_convolveNotAllowOverexposed.Increment();

  m_blockyWavyTimer.Increment();
  m_noiseTimer.Increment();
  m_allowOverexposedTimer.Increment();
}

void GoomControl::GoomControlImpl::ProcessAudio(const AudioSamples& soundData) const
{
  /* ! etude du signal ... */
  m_goomInfo->ProcessSoundSample(soundData);
}

void GoomControl::GoomControlImpl::ApplyCurrentStateToSingleBuffer()
{
  // applyIfsIfRequired();
  ApplyDotsIfRequired();
}

void GoomControl::GoomControlImpl::ApplyCurrentStateToMultipleBuffers()
{
  //  LogInfo("Begin");

  ApplyDotsToBothBuffersIfRequired();
  ApplyIfsToBothBuffersIfRequired();
  ApplyTentaclesToBothBuffersIfRequired();
  ApplyStarsToBothBuffersIfRequired();
  ApplyTubeToBothBuffersIfRequired();
  //  ApplyImageIfRequired();

  //  LogInfo("End");
}

void GoomControl::GoomControlImpl::ResetDrawBuffSettings(const FXBuffSettings& settings)
{
  m_multiBufferDraw.SetBuffIntensity(settings.buffIntensity);
  m_multiBufferDraw.SetAllowOverexposed(settings.allowOverexposed);
}

void GoomControl::GoomControlImpl::UpdateBuffers()
{
  // affichage et swappage des buffers...
  m_visualFx.convolve_fx->Convolve(m_imageBuffers.GetP1(), m_imageBuffers.GetOutputBuff());

  RotateDrawBuffers();

  if (m_convolveAllowOverexposed.Finished())
  {
    m_visualFx.convolve_fx->SetAllowOverexposed(false);
    m_convolveNotAllowOverexposed.SetTimeLimit(
        GetRandInRange(MIN_NUM_NOT_OVEREXPOSED_UPDATES, MAX_NUM_NOT_OVEREXPOSED_UPDATES + 1));
  }
  else if (m_convolveNotAllowOverexposed.Finished())
  {
    m_visualFx.convolve_fx->SetAllowOverexposed(true);
    m_convolveAllowOverexposed.SetTimeLimit(
        GetRandInRange(MIN_NUM_OVEREXPOSED_UPDATES, MAX_NUM_OVEREXPOSED_UPDATES + 1));
  }
}

inline void GoomControl::GoomControlImpl::RotateDrawBuffers()
{
  m_imageBuffers.RotateBuffers();
  m_multiBufferDraw.SetBuffers(this->GetCurrentBuffers());
}

inline auto GoomControl::GoomControlImpl::GetCurrentBuffers() const -> std::vector<PixelBuffer*>
{
  return {&m_imageBuffers.GetP1(), &m_imageBuffers.GetP2()};
}

inline auto GoomControl::GoomControlImpl::IsLocked() const -> bool
{
  return m_goomData.lock.IsLocked();
}

inline auto GoomControl::GoomControlImpl::GetLockTime() const -> uint32_t
{
  return m_goomData.lock.GetLockTime();
}

inline void GoomControl::GoomControlImpl::UpdateLock()
{
  m_goomData.lock.Update();
}

inline void GoomControl::GoomControlImpl::SetLockTime(const uint32_t val)
{
  m_goomData.lock.SetLockTime(val);
}

inline void GoomControl::GoomControlImpl::IncreaseLockTime(const uint32_t byAmount)
{
  m_goomData.lock.IncreaseLockTime(byAmount);
}

void GoomControl::GoomControlImpl::ChangeFilterModeIfMusicChanges()
{
  if ((0 == m_goomInfo->GetSoundInfo().GetTimeSinceLastGoom()) ||
      (m_goomData.updatesSinceLastZoomEffectsChange > MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE))
  {
    LogDebug("Try to change the filter mode.");
    if (ChangeFilterModeEventHappens())
    {
      ChangeFilterMode();
    }
  }
  LogDebug("sound GetTimeSinceLastGoom() = {}", m_goomInfo->GetSoundInfo().GetTimeSinceLastGoom());
}

void GoomControl::GoomControlImpl::ChangeFilterMode()
{
  m_filterSettingsService.SetRandomFilterSettings();
  m_filterSettingsService.SetMiddlePoints();
  m_curGDrawables = m_states.GetCurrentDrawables();

  DoIfsRenew();
}

void GoomControl::GoomControlImpl::ChangeState()
{
  if (m_goomData.stateSelectionBlocker)
  {
    --m_goomData.stateSelectionBlocker;
  }
  else if (m_goomEvent.Happens(GoomEvent::CHANGE_STATE))
  {
    m_goomData.stateSelectionBlocker = 3;
    DoChangeState();
  }
}

void GoomControl::GoomControlImpl::DoChangeState()
{
  const auto oldGDrawables = m_states.GetCurrentDrawables();

  SuspendCurrentState();
  SetNextState();
  ResumeCurrentState();

  m_curGDrawables = m_states.GetCurrentDrawables();
  m_timeInState = 0;

  ChangeColorMaps();

  if (m_states.IsCurrentlyDrawable(GoomDrawable::IFS))
  {
#if __cplusplus <= 201402L
    if (oldGDrawables.find(GoomDrawable::IFS) == oldGDrawables.end())
#else
    if (!oldGDrawables.contains(GoomDrawable::IFS))
#endif
    {
      m_visualFx.ifs_fx->Init();
    }
    else if (m_goomEvent.Happens(GoomEvent::IFS_RENEW))
    {
      DoIfsRenew();
    }
    m_visualFx.ifs_fx->UpdateIncr();
  }

  if (!m_states.IsCurrentlyDrawable(GoomDrawable::SCOPE))
  {
    m_goomData.stopLines = 0xF000 & 5;
  }
  if (!m_states.IsCurrentlyDrawable(GoomDrawable::FAR_SCOPE))
  {
    m_goomData.stopLines = 0;
    m_goomData.lineMode = m_goomData.drawLinesDuration;
  }
}

void GoomControl::GoomControlImpl::ResumeCurrentState()
{
  for (const auto& drawable : m_states.GetCurrentDrawables())
  {
    if (m_visualFx.map.find(drawable) == m_visualFx.map.end())
    {
      continue;
    }
    m_visualFx.map.at(drawable)->Resume();
  }
}

void GoomControl::GoomControlImpl::SuspendCurrentState()
{
  for (const auto& drawable : m_states.GetCurrentDrawables())
  {
    if (m_visualFx.map.find(drawable) == m_visualFx.map.end())
    {
      continue;
    }
    m_visualFx.map.at(drawable)->Suspend();
  }
}

void GoomControl::GoomControlImpl::SetNextState()
{
  constexpr size_t MAX_TRIES = 10;
  const size_t oldStateIndex = m_states.GetCurrentStateIndex();

  for (size_t numTry = 0; numTry < MAX_TRIES; ++numTry)
  {
    m_states.DoRandomStateChange();
    // Pick a different state if possible
    if (oldStateIndex != m_states.GetCurrentStateIndex())
    {
      break;
    }
  }
}

void GoomControl::GoomControlImpl::BigBreakIfMusicIsCalm()
{
  constexpr float CALM_SPEED = 0.01F;
  constexpr uint32_t CALM_CYCLES = 16;

  if ((m_goomInfo->GetSoundInfo().GetSpeed() < CALM_SPEED) &&
      (m_filterSettingsService.GetROVitesseSetting().GetVitesse() < (Vitesse::STOP_SPEED - 4)) &&
      (0 == (m_cycle % CALM_CYCLES)))
  {
    BigBreak();
  }
}

inline void GoomControl::GoomControlImpl::BigBreak()
{
  m_filterSettingsService.GetRWVitesseSetting().GoSlowerBy(3);

  ChangeColorMaps();
}

inline void GoomControl::GoomControlImpl::BigUpdateIfNotLocked()
{
  if (!IsLocked())
  {
    BigUpdate();
  }
}

void GoomControl::GoomControlImpl::BigUpdate()
{
  // Reperage de goom (acceleration forte de l'acceleration du volume).
  // Coup de boost de la vitesse si besoin.
  // Goom tracking (strong acceleration of volume acceleration).
  // Speed boost if needed.
  if (0 == m_goomInfo->GetSoundInfo().GetTimeSinceLastGoom())
  {
    BigNormalUpdate();
  }

  // mode mega-lent
  if (m_goomEvent.Happens(GoomEvent::CHANGE_TO_MEGA_LENT_MODE))
  {
    MegaLentUpdate();
  }
}

void GoomControl::GoomControlImpl::BigNormalUpdate()
{
  SetLockTime(NORMAL_UPDATE_LOCK_TIME);

  ChangeState();
  ChangeSpeedReverse();
  ChangeStopSpeeds();
  ChangeRotation();
  ChangeMilieu();
  ChangeNoise();
  ChangeBlockyWavy();
  ChangeAllowOverexposed();
  ChangeVitesse();
  ChangeSwitchValues();

  m_singleBufferDots = ProbabilityOfMInN(1, 20);
}

void GoomControl::GoomControlImpl::MegaLentUpdate()
{
  IncreaseLockTime(MEGA_LENT_LOCK_TIME_INCREASE);

  m_filterSettingsService.GetRWVitesseSetting().SetVitesse(Vitesse::STOP_SPEED - 1);
  m_filterSettingsService.SetDefaultTranLerpIncrement();
  m_filterSettingsService.SetTranLerpToMaxSwitchMult(1.0F);
}

inline void GoomControl::GoomControlImpl::ChangeMilieu()
{
  m_filterSettingsService.ChangeMilieu();
}

void GoomControl::GoomControlImpl::ChangeVitesse()
{
  const auto goFasterVal = static_cast<int32_t>(
      std::lround(3.5F * std::log10(1.0F + (500.0F * m_goomInfo->GetSoundInfo().GetSpeed()))));
  const int32_t newVitesse = Vitesse::STOP_SPEED - goFasterVal;
  const int32_t oldVitesse = m_filterSettingsService.GetROVitesseSetting().GetVitesse();

  if (newVitesse >= oldVitesse)
  {
    return;
  }

  constexpr uint32_t VITESSE_CYCLES = 3;
  constexpr int32_t FAST_SPEED = Vitesse::STOP_SPEED - 6;
  constexpr int32_t FASTER_SPEED = Vitesse::STOP_SPEED - 7;
  constexpr int32_t SLOW_SPEED = Vitesse::STOP_SPEED - 1;
  constexpr float OLD_TO_NEW_MIX = 0.4F;

  // on accelere
  if (((newVitesse < FASTER_SPEED) && (oldVitesse < FAST_SPEED) &&
       (0 == (m_cycle % VITESSE_CYCLES))) ||
      m_goomEvent.Happens(GoomEvent::FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE))
  {
    m_filterSettingsService.GetRWVitesseSetting().SetVitesse(SLOW_SPEED);
    m_filterSettingsService.GetRWVitesseSetting().ToggleReverseVitesse();
  }
  else
  {
    m_filterSettingsService.GetRWVitesseSetting().SetVitesse(
        static_cast<int32_t>(std::lround(stdnew::lerp(
            static_cast<float>(oldVitesse), static_cast<float>(newVitesse), OLD_TO_NEW_MIX))));
  }

  IncreaseLockTime(CHANGE_VITESSE_LOCK_TIME_INCREASE);
}

void GoomControl::GoomControlImpl::ChangeSpeedReverse()
{
  // Retablir le zoom avant.
  // Restore zoom in.

  constexpr uint32_t REVERSE_VITESSE_CYCLES = 13;
  constexpr int32_t SLOW_SPEED = Vitesse::STOP_SPEED - 2;

  if ((m_filterSettingsService.GetROVitesseSetting().GetReverseVitesse()) &&
      ((m_cycle % REVERSE_VITESSE_CYCLES) != 0) &&
      m_goomEvent.Happens(GoomEvent::FILTER_REVERSE_OFF_AND_STOP_SPEED))
  {
    m_filterSettingsService.GetRWVitesseSetting().SetReverseVitesse(false);
    m_filterSettingsService.GetRWVitesseSetting().SetVitesse(SLOW_SPEED);
    SetLockTime(REVERSE_SPEED_AND_STOP_SPEED_LOCK_TIME);
  }
  if (m_goomEvent.Happens(GoomEvent::FILTER_REVERSE_ON))
  {
    m_filterSettingsService.GetRWVitesseSetting().SetReverseVitesse(true);
    SetLockTime(REVERSE_SPEED_LOCK_TIME);
  }
}

void GoomControl::GoomControlImpl::ChangeStopSpeeds()
{
  if (m_goomEvent.Happens(GoomEvent::FILTER_VITESSE_STOP_SPEED_MINUS1))
  {
    constexpr int32_t SLOW_SPEED = Vitesse::STOP_SPEED - 1;
    m_filterSettingsService.GetRWVitesseSetting().SetVitesse(SLOW_SPEED);
  }
  if (m_goomEvent.Happens(GoomEvent::FILTER_VITESSE_STOP_SPEED))
  {
    m_filterSettingsService.GetRWVitesseSetting().SetVitesse(Vitesse::STOP_SPEED);
  }
}

void GoomControl::GoomControlImpl::ChangeSwitchValues()
{
  if (GetLockTime() > CHANGE_SWITCH_VALUES_LOCK_TIME)
  {
    m_filterSettingsService.SetDefaultTranLerpIncrement();
    m_filterSettingsService.SetTranLerpToMaxSwitchMult(1.0F);
  }
}

void GoomControl::GoomControlImpl::ChangeNoise()
{
  if (!m_noiseTimer.Finished())
  {
    return;
  }

  if (m_goomEvent.Happens(GoomEvent::TURN_OFF_NOISE))
  {
    m_filterSettingsService.SetNoisifySetting(false);
  }
  else
  {
    m_filterSettingsService.SetNoisifySetting(true);
    m_filterSettingsService.SetNoiseFactorSetting(GetRandInRange(0.1F, 1.0F));
    m_noiseTimer.ResetToZero();
  }
}

void GoomControl::GoomControlImpl::ChangeRotation()
{
  if (m_goomEvent.Happens(GoomEvent::FILTER_STOP_ROTATION))
  {
    m_filterSettingsService.SetRotateToZero();
  }
  else if (m_goomEvent.Happens(GoomEvent::FILTER_DECREASE_ROTATION))
  {
    constexpr float ROTATE_SLOWER_FACTOR = 0.9F;
    m_filterSettingsService.MultiplyRotateSetting(ROTATE_SLOWER_FACTOR);
  }
  else if (m_goomEvent.Happens(GoomEvent::FILTER_INCREASE_ROTATION))
  {
    constexpr float ROTATE_FASTER_FACTOR = 1.1F;
    m_filterSettingsService.MultiplyRotateSetting(ROTATE_FASTER_FACTOR);
  }
  else if (m_goomEvent.Happens(GoomEvent::FILTER_TOGGLE_ROTATION))
  {
    m_filterSettingsService.ToggleRotateSetting();
  }
}

void GoomControl::GoomControlImpl::ChangeBlockyWavy()
{
  if (!m_blockyWavyTimer.Finished())
  {
    return;
  }

  m_filterSettingsService.SetBlockyWavySetting(
      m_goomEvent.Happens(GoomEvent::CHANGE_BLOCKY_WAVY_TO_ON));

  m_blockyWavyTimer.ResetToZero();
}

inline void GoomControl::GoomControlImpl::ChangeClippedColor()
{
  m_filterSettingsService.SetClippedColor(
      UTILS::GetBrighterColor(0.1F, m_clippedColorMap.get().GetColor(m_clippedT()), true));
  m_clippedT.Increment();
}

void GoomControl::GoomControlImpl::ChangeAllowOverexposed()
{
  if (!m_allowOverexposedTimer.Finished())
  {
    return;
  }

  m_visualFx.zoomFilter_fx->SetBuffSettings(
      {/*.buffIntensity = */ FXBuffSettings::INITIAL_BUFF_INTENSITY,
       /*.allowOverexposed = */ m_goomEvent.Happens(
           GoomEvent::CHANGE_ZOOM_FILTER_ALLOW_OVEREXPOSED_TO_ON)});

  m_allowOverexposedTimer.ResetToZero();
}

/* Changement d'effet de zoom !
 */
void GoomControl::GoomControlImpl::ChangeZoomEffect()
{
  ChangeBlockyWavy();
  ChangeClippedColor();
  ChangeAllowOverexposed();

  if (!m_filterSettingsService.HasFilterModeChangedSinceLastUpdate())
  {
    if (m_goomData.updatesSinceLastZoomEffectsChange > MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE)
    {
      m_goomData.updatesSinceLastZoomEffectsChange = 0;

      ChangeRotation();
      DoIfsRenew();
    }
    else
    {
      ++m_goomData.updatesSinceLastZoomEffectsChange;
    }
  }
  else
  {
    m_goomData.updatesSinceLastZoomEffectsChange = 0;
    m_filterSettingsService.SetDefaultTranLerpIncrement();

    int32_t diff =
        m_filterSettingsService.GetROVitesseSetting().GetVitesse() - m_goomData.previousZoomSpeed;
    if (diff < 0)
    {
      diff = -diff;
    }

    if (diff > 2)
    {
      m_filterSettingsService.MultiplyTranLerpIncrement((diff + 2) / 2);
    }
    m_goomData.previousZoomSpeed = m_filterSettingsService.GetROVitesseSetting().GetVitesse();
    m_filterSettingsService.SetTranLerpToMaxSwitchMult(1.0F);

    if ((0 == m_goomInfo->GetSoundInfo().GetTimeSinceLastGoom()) &&
        (m_goomInfo->GetSoundInfo().GetTotalGoomsInCurrentCycle() < 2))
    {
      m_filterSettingsService.SetTranLerpIncrement(0);
      m_filterSettingsService.SetDefaultTranLerpToMaxSwitchMult();

      ChangeRotation();
      DoIfsRenew();
    }
  }
}

void GoomControl::GoomControlImpl::StopDecrementingAfterAWhile()
{
  if (0 == (m_cycle % 101))
  {
    StopDecrementing();
  }
}

void GoomControl::GoomControlImpl::StopDecrementing()
{
  // TODO What used to be here???
}

void GoomControl::GoomControlImpl::RegularlyLowerTheSpeed()
{
  constexpr uint32_t LOWER_SPEED_CYCLES = 73;
  constexpr int32_t FAST_SPEED = Vitesse::STOP_SPEED - 5;

  if ((0 == (m_cycle % LOWER_SPEED_CYCLES)) &&
      (m_filterSettingsService.GetROVitesseSetting().GetVitesse() < FAST_SPEED))
  {
    m_filterSettingsService.GetRWVitesseSetting().GoSlowerBy(1);
  }
}

void GoomControl::GoomControlImpl::ApplyZoom()
{
  UpdateFilterSettings();

  m_visualFx.zoomFilter_fx->ZoomFilterFastRgb(m_imageBuffers.GetP1(), m_imageBuffers.GetP2());
}

inline void GoomControl::GoomControlImpl::UpdateFilterSettings()
{
  if (m_filterSettingsService.HaveEffectsSettingsChangedSinceLastUpdate())
  {
    m_visualFx.zoomFilter_fx->UpdateFilterEffectsSettings(
        m_filterSettingsService.GetFilterSettings().filterEffectsSettings);
    m_filterSettingsService.NotifyUpdatedFilterEffectsSettings();
  }

  m_visualFx.zoomFilter_fx->UpdateFilterBufferSettings(
      m_filterSettingsService.GetFilterSettings().filterBufferSettings);

  ChangeClippedColor();

  m_visualFx.zoomFilter_fx->UpdateFilterColorSettings(
      m_filterSettingsService.GetFilterSettings().filterColorSettings);
}

void GoomControl::GoomControlImpl::ApplyDotsIfRequired()
{
  // LogInfo("Begin");

#if __cplusplus <= 201402L
  if (m_curGDrawables.find(GoomDrawable::DOTS) == m_curGDrawables.end())
#else
  if (!m_curGDrawables.contains(GoomDrawable::DOTS))
#endif
  {
    return;
  }

  if (!m_singleBufferDots)
  {
    return;
  }

  ResetDrawBuffSettings(m_states.GetCurrentBuffSettings(GoomDrawable::DOTS));
  m_visualFx.goomDots_fx->ApplySingle();

  // LogInfo("End");
}

void GoomControl::GoomControlImpl::ApplyDotsToBothBuffersIfRequired()
{
  // LogInfo("Begin");

#if __cplusplus <= 201402L
  if (m_curGDrawables.find(GoomDrawable::DOTS) == m_curGDrawables.end())
#else
  if (!m_curGDrawables.contains(GoomDrawable::DOTS))
#endif
  {
    return;
  }

  if (m_singleBufferDots)
  {
    return;
  }

  ResetDrawBuffSettings(m_states.GetCurrentBuffSettings(GoomDrawable::DOTS));
  m_visualFx.goomDots_fx->ApplyMultiple();

  // LogInfo("End");
}

void GoomControl::GoomControlImpl::ApplyIfsToBothBuffersIfRequired()
{
  // LogInfo("Begin");

#if __cplusplus <= 201402L
  if (m_curGDrawables.find(GoomDrawable::IFS) == m_curGDrawables.end())
#else
  if (!m_curGDrawables.contains(GoomDrawable::IFS))
#endif
  {
    m_visualFx.ifs_fx->ApplyNoDraw();
    return;
  }

  ResetDrawBuffSettings(m_states.GetCurrentBuffSettings(GoomDrawable::IFS));
  m_visualFx.ifs_fx->ApplyMultiple();

  // LogInfo("End");
}

void GoomControl::GoomControlImpl::DoIfsRenew()
{
  m_visualFx.ifs_fx->Renew();
}

void GoomControl::GoomControlImpl::ApplyTentaclesToBothBuffersIfRequired()
{
  //  LogInfo("Begin");

#if __cplusplus <= 201402L
  if (m_curGDrawables.find(GoomDrawable::TENTACLES) == m_curGDrawables.end())
#else
  if (!m_curGDrawables.contains(GoomDrawable::TENTACLES))
#endif
  {
    m_visualFx.tentacles_fx->ApplyNoDraw();
    return;
  }

  ResetDrawBuffSettings(m_states.GetCurrentBuffSettings(GoomDrawable::TENTACLES));
  m_visualFx.tentacles_fx->ApplyMultiple();

  //  LogInfo("End");
}

void GoomControl::GoomControlImpl::ApplyTubeToBothBuffersIfRequired()
{
  //  LogInfo("Begin");

#if __cplusplus <= 201402L
  if (m_curGDrawables.find(GoomDrawable::TUBE) == m_curGDrawables.end())
#else
  if (!m_curGDrawables.contains(GoomDrawable::TUBE))
#endif
  {
    m_visualFx.tube_fx->ApplyNoDraw();
    return;
  }

  //  m_stats.DoTube();
  ResetDrawBuffSettings(m_states.GetCurrentBuffSettings(GoomDrawable::TUBE));
  m_visualFx.tube_fx->ApplyMultiple();

  //  LogInfo("End");
}

void GoomControl::GoomControlImpl::ApplyStarsToBothBuffersIfRequired()
{
  //  LogInfo("Begin");

#if __cplusplus <= 201402L
  if (m_curGDrawables.find(GoomDrawable::STARS) == m_curGDrawables.end())
#else
  if (!m_curGDrawables.contains(GoomDrawable::STARS))
#endif
  {
    return;
  }

  LogDebug("curGDrawables stars is set.");
  ResetDrawBuffSettings(m_states.GetCurrentBuffSettings(GoomDrawable::STARS));
  m_visualFx.star_fx->ApplyMultiple();

  //  LogInfo("End");
}

void GoomControl::GoomControlImpl::StopLinesIfRequested()
{
  if ((m_goomData.stopLines & 0xf000) || (!m_states.IsCurrentlyDrawable(GoomDrawable::SCOPE)))
  {
    StopLinesRequest();
  }
}

void GoomControl::GoomControlImpl::StopLinesRequest()
{
  if ((!m_goomLine1.CanResetDestLine()) || (!m_goomLine2.CanResetDestLine()))
  {
    return;
  }

  float param1 = 0.0;
  float param2 = 0.0;
  float amplitude = 0.0;
  Pixel couleur{};
  LinesFx::LineType mode;
  ChooseGoomLine(&param1, &param2, &couleur, &mode, &amplitude, 1);
  couleur = GetBlackLineColor();

  m_goomLine1.ResetDestLine(mode, param1, amplitude, couleur);
  m_goomLine2.ResetDestLine(mode, param2, amplitude, couleur);
  m_goomData.stopLines &= 0x0fff;
}

void GoomControl::GoomControlImpl::ChooseGoomLine(float* const param1,
                                                  float* const param2,
                                                  Pixel* const couleur,
                                                  LinesFx::LineType* const mode,
                                                  float* const amplitude,
                                                  const int farVal)
{
  *amplitude = 1.0F;
  *mode = m_goomEvent.GetRandomLineTypeEvent();

  switch (*mode)
  {
    case LinesFx::LineType::CIRCLE:
      if (farVal)
      {
        *param1 = 0.47F;
        *param2 = *param1;
        *amplitude = 0.8F;
        break;
      }
      if (m_goomEvent.Happens(GoomEvent::CHANGE_LINE_CIRCLE_AMPLITUDE))
      {
        *param1 = 0.0F;
        *param2 = 0.0F;
        *amplitude = 3.0F;
      }
      else if (m_goomEvent.Happens(GoomEvent::CHANGE_LINE_CIRCLE_PARAMS))
      {
        *param1 = 0.40F * static_cast<float>(GetScreenHeight());
        *param2 = 0.22F * static_cast<float>(GetScreenHeight());
      }
      else
      {
        *param1 = static_cast<float>(GetScreenHeight()) * 0.35F;
        *param2 = *param1;
      }
      break;
    case LinesFx::LineType::H_LINE:
      if (m_goomEvent.Happens(GoomEvent::CHANGE_H_LINE_PARAMS) || farVal)
      {
        *param1 = static_cast<float>(GetScreenHeight()) / 7.0F;
        *param2 = (6.0F * static_cast<float>(GetScreenHeight())) / 7.0F;
      }
      else
      {
        *param1 = static_cast<float>(GetScreenHeight()) / 2.0F;
        *param2 = *param1;
        *amplitude = 2.0F;
      }
      break;
    case LinesFx::LineType::V_LINE:
      if (m_goomEvent.Happens(GoomEvent::CHANGE_V_LINE_PARAMS) || farVal)
      {
        *param1 = static_cast<float>(GetScreenWidth()) / 7.0F;
        *param2 = (6.0F * static_cast<float>(GetScreenWidth())) / 7.0F;
      }
      else
      {
        *param1 = static_cast<float>(GetScreenWidth()) / 2.0F;
        *param2 = *param1;
        *amplitude = 1.5F;
      }
      break;
    default:
      throw std::logic_error("Unknown LineTypes enum.");
  }

  *couleur = m_goomLine1.GetRandomLineColor();
}

/* arret aleatore.. changement de mode de ligne..
  */
void GoomControl::GoomControlImpl::StopRandomLineChangeMode()
{
  constexpr uint32_t DEC_LINE_MODE_CYCLES = 80;
  constexpr uint32_t UPDATE_LINE_MODE_CYCLES = 120;

  if (m_goomData.lineMode != m_goomData.drawLinesDuration)
  {
    --m_goomData.lineMode;
    if (-1 == m_goomData.lineMode)
    {
      m_goomData.lineMode = 0;
    }
  }
  else if ((0 == (m_cycle % DEC_LINE_MODE_CYCLES)) &&
           m_goomEvent.Happens(GoomEvent::REDUCE_LINE_MODE) && m_goomData.lineMode)
  {
    --m_goomData.lineMode;
  }

  if ((0 == (m_cycle % UPDATE_LINE_MODE_CYCLES)) &&
      m_goomEvent.Happens(GoomEvent::UPDATE_LINE_MODE) &&
#if __cplusplus <= 201402L
      (m_curGDrawables.find(GoomDrawable::SCOPE) != m_curGDrawables.end()))
#else
      m_curGDrawables.contains(GoomDrawable::SCOPE))
#endif
  {
    if (0 == m_goomData.lineMode)
    {
      m_goomData.lineMode = m_goomData.drawLinesDuration;
    }
    else if ((m_goomData.lineMode == m_goomData.drawLinesDuration) &&
             m_goomLine1.CanResetDestLine() && m_goomLine2.CanResetDestLine())
    {
      --m_goomData.lineMode;

      float param1 = 0.0;
      float param2 = 0.0;
      float amplitude = 0.0;
      Pixel color1{};
      LinesFx::LineType mode;
      ChooseGoomLine(&param1, &param2, &color1, &mode, &amplitude, m_goomData.stopLines);

      Pixel color2 = m_goomLine2.GetRandomLineColor();
      if (m_goomData.stopLines)
      {
        --m_goomData.stopLines;
        if (m_goomEvent.Happens(GoomEvent::CHANGE_LINE_TO_BLACK))
        {
          color1 = GetBlackLineColor();
          color2 = color1;
        }
      }

      LogDebug("goomData.lineMode = {} == {} = goomData.drawLinesDuration", m_goomData.lineMode,
               m_goomData.drawLinesDuration);
      m_goomLine1.ResetDestLine(mode, param1, amplitude, color1);
      m_goomLine2.ResetDestLine(mode, param2, amplitude, color2);
    }
  }
}

void GoomControl::GoomControlImpl::DisplayLines(const AudioSamples& soundData)
{
#if __cplusplus <= 201402L
  if (m_curGDrawables.find(GoomDrawable::LINES) == m_curGDrawables.end())
#else
  if (!m_curGDrawables.contains(GoomDrawable::LINES))
#endif
  {
    return;
  }

  LogDebug("curGDrawables lines is set.");

  m_goomLine2.SetPower(m_goomLine1.GetPower());

  const std::vector<int16_t>& audioSample = soundData.GetSample(0);
  const AudioSamples::MaxMinValues& soundMinMax = soundData.GetSampleMinMax(0);
  m_goomLine1.DrawLines(audioSample, soundMinMax);
  m_goomLine2.DrawLines(audioSample, soundMinMax);
  //  gmline2.drawLines(soundData.GetSample(1));

  constexpr uint32_t CHANGE_GOOM_LINE_CYCLES = 121;

  if ((9 == (m_cycle % CHANGE_GOOM_LINE_CYCLES)) &&
      m_goomEvent.Happens(GoomEvent::CHANGE_GOOM_LINE) &&
      ((0 == m_goomData.lineMode) || (m_goomData.lineMode == m_goomData.drawLinesDuration)) &&
      m_goomLine1.CanResetDestLine() && m_goomLine2.CanResetDestLine())
  {
    LogDebug("cycle % 121 etc.: goomInfo->cycle = {}, rand1_3 = ?", m_cycle);
    float param1 = 0.0;
    float param2 = 0.0;
    float amplitude = 0.0;
    Pixel color1{};
    LinesFx::LineType mode;
    ChooseGoomLine(&param1, &param2, &color1, &mode, &amplitude, m_goomData.stopLines);

    Pixel color2 = m_goomLine2.GetRandomLineColor();
    if (m_goomData.stopLines)
    {
      --m_goomData.stopLines;
      if (m_goomEvent.Happens(GoomEvent::CHANGE_LINE_TO_BLACK))
      {
        color1 = GetBlackLineColor();
        color2 = color1;
      }
    }
    m_goomLine1.ResetDestLine(mode, param1, amplitude, color1);
    m_goomLine2.ResetDestLine(mode, param2, amplitude, color2);
  }
}

void GoomControl::GoomControlImpl::DisplayLinesIfInAGoom(const AudioSamples& soundData)
{
  constexpr uint32_t DISPLAY_LINES_GOOM_NUM = 5;

  if ((m_goomData.lineMode != 0) ||
      (m_goomInfo->GetSoundInfo().GetTimeSinceLastGoom() < DISPLAY_LINES_GOOM_NUM))
  {
    DisplayLines(soundData);
  }
}

void GoomControl::GoomControlImpl::DisplayTitle(const std::string& songTitle,
                                                const std::string& message,
                                                const float fps)
{
  std::string msg{message};

  if (fps > 0.0)
  {
    const std::string text = std20::format("{.0f} fps", fps);
    msg.insert(0, text + "\n");
  }

  UpdateMessages(msg);

  if (!songTitle.empty())
  {
    m_goomData.title = songTitle;

    const auto xPos = static_cast<int>(0.085F * static_cast<float>(GetScreenWidth()));
    const auto yPos = static_cast<int>(0.300F * static_cast<float>(GetScreenHeight()));

    m_goomTitleDisplay =
        std::make_unique<GoomTitleDisplay>(xPos, yPos, GetFontDirectory(), m_goomTextOutput);
  }

  if ((m_goomTitleDisplay != nullptr) && (!m_goomTitleDisplay->IsFinished()))
  {
    if (m_goomTitleDisplay->IsFinalPhase())
    {
      m_goomTextOutput.SetBuffers({&m_imageBuffers.GetP1()});
    }
    else
    {
      m_goomTextOutput.SetBuffers({&m_imageBuffers.GetOutputBuff()});
    }
    m_goomTitleDisplay->Draw(m_goomData.title);
  }
}

/*
 * Met a jour l'affichage du message defilant
 */
void GoomControl::GoomControlImpl::UpdateMessages(const std::string& messages)
{
  if (messages.empty())
  {
    return;
  }

  constexpr int32_t MSG_FONT_SIZE = 10;
  constexpr int32_t VERTICAL_SPACING = 10;
  constexpr size_t LINE_HEIGHT = MSG_FONT_SIZE + VERTICAL_SPACING;
  constexpr int32_t X_POS = 50;
  constexpr int32_t Y_START = 50;

  const std::vector<std::string> msgLines = StringSplit(messages, "\n");
  const size_t numberOfLinesInMessage = msgLines.size();
  const size_t totalMessagesHeight = 20 + (LINE_HEIGHT * numberOfLinesInMessage);

  if (m_updateMessagesDisplay == nullptr)
  {
    const auto getFontColor = []([[maybe_unused]] const size_t textIndexOfChar,
                                 [[maybe_unused]] const float x, [[maybe_unused]] const float y,
                                 [[maybe_unused]] const float width,
                                 [[maybe_unused]] const float height) { return Pixel::WHITE; };
    const auto getOutlineFontColor =
        []([[maybe_unused]] const size_t textIndexOfChar, [[maybe_unused]] const float x,
           [[maybe_unused]] const float y, [[maybe_unused]] const float width,
           [[maybe_unused]] const float height) { return Pixel{0xFAFAFAFAU}; };
    m_updateMessagesDisplay = std::make_unique<TextDraw>(m_goomTextOutput);
    m_updateMessagesDisplay->SetFontFile(m_updateMessagesFontFile);
    m_updateMessagesDisplay->SetFontSize(MSG_FONT_SIZE);
    m_updateMessagesDisplay->SetOutlineWidth(1);
    m_updateMessagesDisplay->SetAlignment(TextDraw::TextAlignment::LEFT);
    m_updateMessagesDisplay->SetFontColorFunc(getFontColor);
    m_updateMessagesDisplay->SetOutlineFontColorFunc(getOutlineFontColor);
  }

  for (size_t i = 0; i < msgLines.size(); ++i)
  {
    const auto yPos = static_cast<int32_t>((Y_START + totalMessagesHeight) -
                                           ((numberOfLinesInMessage - i) * LINE_HEIGHT));
    m_updateMessagesDisplay->SetText(msgLines[i]);
    m_updateMessagesDisplay->Prepare();
    m_goomTextOutput.SetBuffers({&m_imageBuffers.GetOutputBuff()});
    m_updateMessagesDisplay->Draw(X_POS, yPos);
  }
}

#ifdef SHOW_STATE_TEXT_ON_SCREEN

void GoomControl::GoomControlImpl::DisplayStateText()
{
  std::string message = "";

  const FILTERS::ZoomFilterEffectsSettings& filterEffectsSettings =
      m_filterSettingsService.GetFilterSettings().filterEffectsSettings;
  const FILTERS::ZoomFilterBufferSettings& filterBufferSettings =
      m_filterSettingsService.GetFilterSettings().filterBufferSettings;

  message += std20::format("State: {}\n", m_states.GetCurrentStateName());
  message += std20::format("Filter Mode: {}\n", m_filterSettingsService.GetCurrentFilterMode());
  message +=
      std20::format("Previous Filter Mode: {}\n", m_filterSettingsService.GetPreviousFilterMode());

  message += std20::format("tranLerpFactor: {}\n", m_visualFx.zoomFilter_fx->GetTranLerpFactor());
  message += std20::format("tranLerpIncrement: {}\n", filterBufferSettings.tranLerpIncrement);
  message +=
      std20::format("tranLerpToMaxSwitchMult: {}\n", filterBufferSettings.tranLerpToMaxSwitchMult);

  message += GetNameValuesString(m_visualFx.zoomFilter_fx->GetNameValueParams("colors")) + "\n";
  message +=
      GetNameValuesString(m_visualFx.zoomFilter_fx->GetNameValueParams("ZoomEffects")) + "\n";

  message += std20::format("middleX: {}\n", filterEffectsSettings.zoomMidPoint.x);
  message += std20::format("middleY: {}\n", filterEffectsSettings.zoomMidPoint.y);

  message += std20::format("vitesse: {}\n", filterEffectsSettings.vitesse.GetVitesse());
  message += std20::format("previousZoomSpeed: {}\n", m_goomData.previousZoomSpeed);
  message += std20::format("reverse: {}\n", filterEffectsSettings.vitesse.GetReverseVitesse());
  message +=
      std20::format("relative speed: {}\n", filterEffectsSettings.vitesse.GetRelativeSpeed());

  message +=
      std20::format("updatesSinceLastChange: {}\n", m_goomData.updatesSinceLastZoomEffectsChange);

  UpdateMessages(message);
}

#endif

} // namespace GOOM
