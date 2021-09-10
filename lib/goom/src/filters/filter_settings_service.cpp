#include "filter_settings_service.h"

#include "filter_amulet.h"
#include "filter_buffers_service.h"
#include "filter_colors_service.h"
#include "filter_crystal_ball.h"
#include "filter_image_displacements.h"
#include "filter_scrunch.h"
#include "filter_settings.h"
#include "filter_simple_speed_coefficients_effect.h"
#include "filter_speedway.h"
#include "filter_wave.h"
#include "filter_y_only.h"
#include "filter_zoom_vector.h"
#include "goom/goom_plugin_info.h"
#include "goomutils/enumutils.h"
#include "goomutils/goomrand.h"

#include <stdexcept>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

using UTILS::GetRandInRange;
using UTILS::NUM;
using UTILS::ProbabilityOf;
using UTILS::ProbabilityOfMInN;
using UTILS::Weights;

constexpr float PROB_HIGH = 0.9F;
constexpr float PROB_HALF = 0.5F;
constexpr float PROB_LOW = 0.1F;
constexpr float PROB_ZERO = 0.0F;

//@formatter:off
// clang-format off
const Weights<FilterSettingsService::ZoomFilterMode> FilterSettingsService::WEIGHTED_FILTER_EVENTS{{
    { ZoomFilterMode::AMULET_MODE,             8 },
    { ZoomFilterMode::CRYSTAL_BALL_MODE0,      4 },
    { ZoomFilterMode::CRYSTAL_BALL_MODE1,      2 },
    { ZoomFilterMode::HYPERCOS_MODE0,          6 },
    { ZoomFilterMode::HYPERCOS_MODE1,          5 },
    { ZoomFilterMode::HYPERCOS_MODE2,          3 },
    { ZoomFilterMode::HYPERCOS_MODE3,          3 },
    { ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, 500000000 },
    { ZoomFilterMode::NORMAL_MODE,             6 },
    { ZoomFilterMode::SCRUNCH_MODE,            6 },
    { ZoomFilterMode::SPEEDWAY_MODE0,          3 },
    { ZoomFilterMode::SPEEDWAY_MODE1,          3 },
    { ZoomFilterMode::WAVE_MODE0,              5 },
    { ZoomFilterMode::WAVE_MODE1,              4 },
    { ZoomFilterMode::WATER_MODE,              0 },
    { ZoomFilterMode::Y_ONLY_MODE,             4 },
}};

auto FilterSettingsService::GetFilterModeData(const std::string& resourcesDirectory) -> std::map<ZoomFilterMode, ZoomFilterModeInfo>
{
  return {
    { ZoomFilterMode::AMULET_MODE,             {"Amulet",              PROB_HIGH, std::make_shared<Amulet>()}},
    { ZoomFilterMode::CRYSTAL_BALL_MODE0,      {"Crystal Ball Mode 0", PROB_HIGH,  std::make_shared<CrystalBall>(CrystalBall::Modes::MODE0)}},
    { ZoomFilterMode::CRYSTAL_BALL_MODE1,      {"Crystal Ball Mode 1", PROB_HALF, std::make_shared<CrystalBall>(CrystalBall::Modes::MODE1)}},
    { ZoomFilterMode::HYPERCOS_MODE0,          {"Hypercos Mode 0",     PROB_LOW,  std::make_shared<SimpleSpeedCoefficientsEffect>()}},
    { ZoomFilterMode::HYPERCOS_MODE1,          {"Hypercos Mode 1",     PROB_LOW,  std::make_shared<SimpleSpeedCoefficientsEffect>()}},
    { ZoomFilterMode::HYPERCOS_MODE2,          {"Hypercos Mode 2",     PROB_LOW,  std::make_shared<SimpleSpeedCoefficientsEffect>()}},
    { ZoomFilterMode::HYPERCOS_MODE3,          {"Hypercos Mode 3",     PROB_LOW,  std::make_shared<SimpleSpeedCoefficientsEffect>()}},
    { ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, {"Image Displacement",  PROB_ZERO, std::make_shared<ImageDisplacements>(resourcesDirectory)}},
    { ZoomFilterMode::NORMAL_MODE,             {"Normal",              PROB_ZERO, std::make_shared<SimpleSpeedCoefficientsEffect>()}},
    { ZoomFilterMode::SCRUNCH_MODE,            {"Scrunch",             PROB_HALF, std::make_shared<Scrunch>()}},
    { ZoomFilterMode::SPEEDWAY_MODE0,          {"Speedway Mode 0",     PROB_LOW,  std::make_shared<Speedway>(Speedway::Modes::MODE0)}},
    { ZoomFilterMode::SPEEDWAY_MODE1,          {"Speedway Mode 1",     PROB_LOW,  std::make_shared<Speedway>(Speedway::Modes::MODE1)}},
    { ZoomFilterMode::WAVE_MODE0,              {"Wave Mode 0",         PROB_HIGH, std::make_shared<Wave>(Wave::Modes::MODE0) }},
    { ZoomFilterMode::WAVE_MODE1,              {"Wave Mode 1",         PROB_HIGH, std::make_shared<Wave>(Wave::Modes::MODE1)}},
    { ZoomFilterMode::WATER_MODE,              {"Water",               PROB_ZERO, std::make_shared<SimpleSpeedCoefficientsEffect>()}},
    { ZoomFilterMode::Y_ONLY_MODE,             {"Y Only",              PROB_HALF, std::make_shared<YOnly>()}},
  };
}
//@formatter:on
// clang-format on

class FilterSettingsService::FilterEvents
{
public:
  FilterEvents() noexcept = default;

  enum class FilterEventTypes
  {
    ROTATE = 0,
    CRYSTAL_BALL_IN_MIDDLE,
    WAVE_IN_MIDDLE,
    HYPERCOS_EFFECT,
    CHANGE_SPEED,
    REVERSE_SPEED,
    _NUM // unused and must be last
  };

  struct Event
  {
    FilterEventTypes event;
    // m out of n
    uint32_t m;
    uint32_t outOf;
  };

  static auto Happens(FilterEventTypes event) -> bool;

private:
  static constexpr size_t NUM_FILTER_EVENT_TYPES = NUM<FilterEventTypes>;

  //@formatter:off
  // clang-format off
#if __cplusplus <= 201402L
  static const std::array<Event, NUM_FILTER_EVENT_TYPES> WEIGHTED_EVENTS;
#else
  static constexpr std::array<Event, NUM_FILTER_EVENT_TYPES> WEIGHTED_EVENTS{{
     { .event = FilterEventTypes::ROTATE,                     .m =  8, .outOf = 16 },
     { .event = FilterEventTypes::CRYSTAL_BALL_IN_MIDDLE,     .m = 14, .outOf = 16 },
     { .event = FilterEventTypes::WAVE_IN_MIDDLE,             .m =  8, .outOf = 16 },
     { .event = FilterEventTypes::HYPERCOS_EFFECT,            .m =  8, .outOf = 16 },
     { .event = FilterEventTypes::CHANGE_SPEED,               .m =  8, .outOf = 16 },
     { .event = FilterEventTypes::REVERSE_SPEED,              .m =  8, .outOf = 16 },
  }};
#endif
  // clang-format on
  //@formatter:on
};

#if __cplusplus <= 201402L
//@formatter:off
// clang-format off
const std::array<FilterSettingsService::FilterEvents::Event,
                 FilterSettingsService::FilterEvents::NUM_FILTER_EVENT_TYPES>
    FilterSettingsService::FilterEvents::WEIGHTED_EVENTS{{
   {/*.event = */ FilterEventTypes::ROTATE,                    /*.m = */  8, /*.outOf = */ 16},
   {/*.event = */ FilterEventTypes::CRYSTAL_BALL_IN_MIDDLE,    /*.m = */ 14, /*.outOf = */ 16},
   {/*.event = */ FilterEventTypes::WAVE_IN_MIDDLE,            /*.m = */  8, /*.outOf = */ 16},
   {/*.event = */ FilterEventTypes::HYPERCOS_EFFECT,           /*.m = */  1, /*.outOf = */ 50},
   {/*.event = */ FilterEventTypes::CHANGE_SPEED,              /*.m = */  8, /*.outOf = */ 16},
   {/*.event = */ FilterEventTypes::REVERSE_SPEED,             /*.m = */  8, /*.outOf = */ 16},
}};
// clang-format on
//@formatter:on
#endif

inline auto FilterSettingsService::FilterEvents::Happens(const FilterEventTypes event) -> bool
{
  const Event& weightedEvent = WEIGHTED_EVENTS[static_cast<size_t>(event)];
  return ProbabilityOfMInN(weightedEvent.m, weightedEvent.outOf);
}

using FilterEventTypes = FilterSettingsService::FilterEvents::FilterEventTypes;

FilterSettingsService::FilterSettingsService(UTILS::Parallel& parallel,
                                             const std::shared_ptr<const PluginInfo>& goomInfo,
                                             const std::string& resourcesDirectory) noexcept
  : m_parallel{parallel},
    m_goomInfo{goomInfo},
    m_midScreenPoint{m_goomInfo->GetScreenInfo().width / 2, m_goomInfo->GetScreenInfo().height / 2},
    m_resourcesDirectory{resourcesDirectory},
    m_filterEvents{spimpl::make_unique_impl<FilterEvents>()},
    m_filterModeData{GetFilterModeData(m_resourcesDirectory)},
    m_filterSettings{{HypercosOverlay::NONE,
                      nullptr,
                      Vitesse{},
                      {DEFAULT_MIDDLE_X, DEFAULT_MIDDLE_Y},
                      false,
                      false,
                      DEFAULT_ROTATE_SPEED,
                      false,
                      1.0F},
                     {DEFAULT_TRAN_LERP_INCREMENT, DEFAULT_SWITCH_MULT},
                     {false, Pixel::BLACK}}
{
}

auto FilterSettingsService::GetFilterBuffersService() -> std::unique_ptr<FilterBuffersService>
{
  return std::make_unique<FilterBuffersService>(
      m_parallel, m_goomInfo,
      std::make_unique<FilterZoomVector>(m_goomInfo->GetScreenInfo().width));
}

auto FilterSettingsService::GetFilterColorsService() -> std::unique_ptr<FilterColorsService>
{
  return std::make_unique<FilterColorsService>();
}

auto FilterSettingsService::GetCurrentFilterMode() const -> const std::string&
{
  return m_filterModeData.at(m_filterMode).name;
}

auto FilterSettingsService::GetPreviousFilterMode() const -> const std::string&
{
  return m_filterModeData.at(m_previousFilterMode).name;
}

void FilterSettingsService::SetFilterModeSettings()
{
  switch (m_filterMode)
  {
    case ZoomFilterMode::AMULET_MODE:
      SetAmuletModeSettings();
      break;
    case ZoomFilterMode::CRYSTAL_BALL_MODE0:
      SetCrystalBallMode0Settings();
      break;
    case ZoomFilterMode::CRYSTAL_BALL_MODE1:
      SetCrystalBallMode1Settings();
      break;
    case ZoomFilterMode::HYPERCOS_MODE0:
      SetHypercosMode0Settings();
      break;
    case ZoomFilterMode::HYPERCOS_MODE1:
      SetHypercosMode1Settings();
      break;
    case ZoomFilterMode::HYPERCOS_MODE2:
      SetHypercosMode2Settings();
      break;
    case ZoomFilterMode::HYPERCOS_MODE3:
      SetHypercosMode3Settings();
      break;
    case ZoomFilterMode::IMAGE_DISPLACEMENT_MODE:
      SetImageDisplacementModeSettings();
      break;
    case ZoomFilterMode::NORMAL_MODE:
      SetNormalModeSettings();
      break;
    case ZoomFilterMode::SCRUNCH_MODE:
      SetScrunchModeSettings();
      break;
    case ZoomFilterMode::SPEEDWAY_MODE0:
      SetSpeedwayMode0Settings();
      break;
    case ZoomFilterMode::SPEEDWAY_MODE1:
      SetSpeedwayMode1Settings();
      break;
    case ZoomFilterMode::WATER_MODE:
      SetWaterModeSettings();
      break;
    case ZoomFilterMode::WAVE_MODE0:
      SetWaveMode0Settings();
      break;
    case ZoomFilterMode::WAVE_MODE1:
      SetWaveMode1Settings();
      break;
    case ZoomFilterMode::Y_ONLY_MODE:
      SetYOnlyModeSettings();
      break;
    default:
      throw std::logic_error("ZoomFilterMode not implemented.");
  }
}

auto FilterSettingsService::GetNewRandomMode() const -> ZoomFilterMode
{
  uint32_t numTries = 0;
  constexpr uint32_t MAX_TRIES = 20;

  while (true)
  {
    const auto newMode = WEIGHTED_FILTER_EVENTS.GetRandomWeighted();
    if (newMode != m_filterMode)
    {
      return newMode;
    }
    ++numTries;
    if (numTries >= MAX_TRIES)
    {
      break;
    }
  }

  return m_filterMode;
}

void FilterSettingsService::Start()
{
  SetRandomFilterSettings();
  SetMiddlePoints();
}

inline auto FilterSettingsService::GetSpeedCoefficientsEffect()
    -> std::shared_ptr<ISpeedCoefficientsEffect>&
{
  return m_filterModeData.at(m_filterMode).speedCoefficientsEffect;
}

void FilterSettingsService::SetRandomFilterSettings(const ZoomFilterMode mode)
{
  m_filterEffectsSettingsHaveChanged = true;

  m_previousFilterMode = m_filterMode;
  m_filterMode = mode;

  SetDefaultSettings();

  SetFilterModeSettings();
}

void FilterSettingsService::SetDefaultSettings()
{
  m_filterSettings.filterEffectsSettings.zoomMidPoint = m_midScreenPoint;

  m_filterSettings.filterEffectsSettings.vitesse.SetDefault();

  m_filterSettings.filterEffectsSettings.tanEffect = ProbabilityOfMInN(1, 10);
  m_filterSettings.filterEffectsSettings.planeEffect = ProbabilityOfMInN(8, 10);
  m_filterSettings.filterEffectsSettings.rotateSpeed = 0.0;
  m_filterSettings.filterEffectsSettings.noisify = false;
  m_filterSettings.filterEffectsSettings.noiseFactor = 1.0;

  m_filterSettings.filterColorSettings.blockyWavy = false;
  m_filterSettings.filterColorSettings.clippedColor = Pixel::BLACK;

  m_filterSettings.filterEffectsSettings.hypercosOverlay = HypercosOverlay::NONE;
  m_filterSettings.filterEffectsSettings.speedCoefficientsEffect = GetSpeedCoefficientsEffect();
}

inline void FilterSettingsService::SetAmuletModeSettings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::AMULET_MODE].rotateProbability);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode2Settings();
  }
}

inline void FilterSettingsService::SetCrystalBallMode0Settings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::CRYSTAL_BALL_MODE0].rotateProbability);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

// TODO - Fix duplication
inline void FilterSettingsService::SetCrystalBallMode1Settings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::CRYSTAL_BALL_MODE1].rotateProbability);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode2Settings();
  }
}

inline void FilterSettingsService::SetHypercosMode0Settings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::HYPERCOS_MODE0].rotateProbability);

  m_filterSettings.filterEffectsSettings.hypercosOverlay = HypercosOverlay::MODE0;
}

inline void FilterSettingsService::SetHypercosMode1Settings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::HYPERCOS_MODE1].rotateProbability);

  m_filterSettings.filterEffectsSettings.hypercosOverlay = HypercosOverlay::MODE1;
}

inline void FilterSettingsService::SetHypercosMode2Settings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::HYPERCOS_MODE2].rotateProbability);

  m_filterSettings.filterEffectsSettings.hypercosOverlay = HypercosOverlay::MODE2;
}

inline void FilterSettingsService::SetHypercosMode3Settings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::HYPERCOS_MODE3].rotateProbability);

  m_filterSettings.filterEffectsSettings.hypercosOverlay = HypercosOverlay::MODE3;
}

inline void FilterSettingsService::SetImageDisplacementModeSettings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::IMAGE_DISPLACEMENT_MODE].rotateProbability);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

inline void FilterSettingsService::SetNormalModeSettings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::NORMAL_MODE].rotateProbability);
}

inline void FilterSettingsService::SetScrunchModeSettings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::SCRUNCH_MODE].rotateProbability);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

inline void FilterSettingsService::SetSpeedwayMode0Settings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::SPEEDWAY_MODE0].rotateProbability);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode0Settings();
  }
}

inline void FilterSettingsService::SetSpeedwayMode1Settings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::SPEEDWAY_MODE1].rotateProbability);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode0Settings();
  }
}

inline void FilterSettingsService::SetWaterModeSettings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::WATER_MODE].rotateProbability);
}

inline void FilterSettingsService::SetWaveMode0Settings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::WAVE_MODE0].rotateProbability);
  SetWaveModeSettings();
}

inline void FilterSettingsService::SetWaveMode1Settings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::WAVE_MODE1].rotateProbability);
  SetWaveModeSettings();
}

void FilterSettingsService::SetWaveModeSettings()
{
  m_filterSettings.filterEffectsSettings.vitesse.SetReverseVitesse(
      m_filterEvents->Happens(FilterEventTypes::REVERSE_SPEED));

  if (m_filterEvents->Happens(FilterEventTypes::CHANGE_SPEED))
  {
    m_filterSettings.filterEffectsSettings.vitesse.SetVitesse(
        (m_filterSettings.filterEffectsSettings.vitesse.GetVitesse() + Vitesse::DEFAULT_VITESSE) /
        2);
  }

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

inline void FilterSettingsService::SetYOnlyModeSettings()
{
  SetRotate(m_filterModeData[ZoomFilterMode::Y_ONLY_MODE].rotateProbability);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

inline void FilterSettingsService::SetRotate(const float rotateProbability)
{
  if (!ProbabilityOf(rotateProbability))
  {
    return;
  }

  m_filterEffectsSettingsHaveChanged = true;

  m_filterSettings.filterEffectsSettings.rotateSpeed =
      rotateProbability * GetRandInRange(MIN_ROTATE_SPEED, MAX_ROTATE_SPEED);
}

void FilterSettingsService::SetRandomMiddlePoints()
{
  if ((m_filterMode == ZoomFilterMode::WATER_MODE) ||
      (m_filterMode == ZoomFilterMode::WAVE_MODE0) || (m_filterMode == ZoomFilterMode::AMULET_MODE))
  {
    m_filterSettings.filterEffectsSettings.zoomMidPoint = m_midScreenPoint;
    return;
  }

  if (((m_filterMode == ZoomFilterMode::CRYSTAL_BALL_MODE0) ||
       (m_filterMode == ZoomFilterMode::CRYSTAL_BALL_MODE1)) &&
      m_filterEvents->Happens(FilterEventTypes::CRYSTAL_BALL_IN_MIDDLE))
  {
    m_filterSettings.filterEffectsSettings.zoomMidPoint = m_midScreenPoint;
    return;
    }
    if ((m_filterMode == ZoomFilterMode::WAVE_MODE1) &&
        m_filterEvents->Happens(FilterEventTypes::WAVE_IN_MIDDLE))
    {
      m_filterSettings.filterEffectsSettings.zoomMidPoint = m_midScreenPoint;
      return;
    }

  // clang-format off
  // @formatter:off
  enum class MiddlePointEvents { EVENT1, EVENT2, EVENT3, EVENT4, EVENT5, EVENT6 };
  static const Weights<MiddlePointEvents> s_middlePointWeights{{
     { MiddlePointEvents::EVENT1,  3 },
     { MiddlePointEvents::EVENT2,  2 },
     { MiddlePointEvents::EVENT3,  2 },
     { MiddlePointEvents::EVENT4, 18 },
     { MiddlePointEvents::EVENT5, 10 },
     { MiddlePointEvents::EVENT6, 10 },
  }};
  // @formatter:on
  // clang-format on

  switch (s_middlePointWeights.GetRandomWeighted())
  {
    case MiddlePointEvents::EVENT1:
      m_filterSettings.filterEffectsSettings.zoomMidPoint = {
          m_goomInfo->GetScreenInfo().width / 2, m_goomInfo->GetScreenInfo().height - 1};
      break;
    case MiddlePointEvents::EVENT2:
      m_filterSettings.filterEffectsSettings.zoomMidPoint.x =
          static_cast<int32_t>(m_goomInfo->GetScreenInfo().width - 1);
      break;
    case MiddlePointEvents::EVENT3:
      m_filterSettings.filterEffectsSettings.zoomMidPoint.x = 1;
      break;
    case MiddlePointEvents::EVENT4:
      m_filterSettings.filterEffectsSettings.zoomMidPoint = m_midScreenPoint;
      break;
    case MiddlePointEvents::EVENT5:
      m_filterSettings.filterEffectsSettings.zoomMidPoint = {
          m_goomInfo->GetScreenInfo().width / 4, m_goomInfo->GetScreenInfo().height / 4};
      break;
    case MiddlePointEvents::EVENT6:
      m_filterSettings.filterEffectsSettings.zoomMidPoint = {
          (3 * m_goomInfo->GetScreenInfo().width) / 4,
          (3 * m_goomInfo->GetScreenInfo().height) / 4};
      break;
    default:
      throw std::logic_error("Unknown MiddlePointEvents enum.");
  }
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::DRAW
#endif
