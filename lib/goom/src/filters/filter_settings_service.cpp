#include "filter_settings_service.h"

#include "filter_amulet.h"
#include "filter_buffers_service.h"
#include "filter_colors_service.h"
#include "filter_crystal_ball.h"
#include "filter_hypercos.h"
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
    { ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, 5 },
    { ZoomFilterMode::NORMAL_MODE,             6 },
    { ZoomFilterMode::SCRUNCH_MODE,            6 },
    { ZoomFilterMode::SPEEDWAY_MODE,           6 },
    { ZoomFilterMode::WAVE_MODE0,              5 },
    { ZoomFilterMode::WAVE_MODE1,              4 },
    { ZoomFilterMode::WATER_MODE,              0 },
    { ZoomFilterMode::Y_ONLY_MODE,             4 },
}};

const std::map<FilterSettingsService::ZoomFilterMode, float> FilterSettingsService::ROTATE_PROBABILITIES{{
    { ZoomFilterMode::AMULET_MODE,             8 },
    { ZoomFilterMode::CRYSTAL_BALL_MODE0,      4 },
    { ZoomFilterMode::CRYSTAL_BALL_MODE1,      2 },
    { ZoomFilterMode::HYPERCOS_MODE0,          6 },
    { ZoomFilterMode::HYPERCOS_MODE1,          5 },
    { ZoomFilterMode::HYPERCOS_MODE2,          3 },
    { ZoomFilterMode::HYPERCOS_MODE3,          3 },
    { ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, 5 },
    { ZoomFilterMode::NORMAL_MODE,             6 },
    { ZoomFilterMode::SCRUNCH_MODE,            6 },
    { ZoomFilterMode::SPEEDWAY_MODE,           6 },
    { ZoomFilterMode::WAVE_MODE0,              5 },
    { ZoomFilterMode::WAVE_MODE1,              4 },
    { ZoomFilterMode::WATER_MODE,              0 },
    { ZoomFilterMode::Y_ONLY_MODE,             4 },
}};
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
    m_speedCoefficientsEffect{MakeSpeedCoefficientsEffects(resourcesDirectory)}
{
}

auto FilterSettingsService::GetFilterBuffersService() -> std::unique_ptr<FilterBuffersService>
{
  return std::make_unique<FilterBuffersService>(
      m_parallel, m_goomInfo,
      std::make_unique<FilterZoomVector>(m_goomInfo->GetScreenInfo().width, m_resourcesDirectory));
}

auto FilterSettingsService::GetFilterColorsService() -> std::unique_ptr<FilterColorsService>
{
  return std::make_unique<FilterColorsService>();
}

auto FilterSettingsService::MakeSpeedCoefficientsEffects(const std::string& resourcesDirectory)
    -> std::vector<std::shared_ptr<SpeedCoefficientsEffect>>
{
  std::vector<std::shared_ptr<SpeedCoefficientsEffect>> effects{};

  for (size_t mode = 0; mode < NUM<ZoomFilterMode>; ++mode)
  {
    effects.emplace_back(
        MakeSpeedCoefficientsEffect(static_cast<ZoomFilterMode>(mode), resourcesDirectory));
  }

  return effects;
}

auto FilterSettingsService::MakeSpeedCoefficientsEffect(const ZoomFilterMode mode,
                                                        const std::string& resourcesDirectory)
    -> std::shared_ptr<SpeedCoefficientsEffect>
{
  switch (mode)
  {
    case ZoomFilterMode::AMULET_MODE:
      return std::make_shared<Amulet>();
    case ZoomFilterMode::CRYSTAL_BALL_MODE0:
      return std::make_shared<CrystalBall>(CrystalBall::Modes::MODE0);
    case ZoomFilterMode::CRYSTAL_BALL_MODE1:
      return std::make_shared<CrystalBall>(CrystalBall::Modes::MODE1);
    case ZoomFilterMode::IMAGE_DISPLACEMENT_MODE:
      return std::make_shared<ImageDisplacements>(resourcesDirectory);
    case ZoomFilterMode::SCRUNCH_MODE:
      return std::make_shared<Scrunch>();
    case ZoomFilterMode::SPEEDWAY_MODE:
      return std::make_shared<Speedway>();
    case ZoomFilterMode::WAVE_MODE0:
      return std::make_shared<Wave>(Wave::Modes::MODE0);
    case ZoomFilterMode::WAVE_MODE1:
      return std::make_shared<Wave>(Wave::Modes::MODE1);
    case ZoomFilterMode::Y_ONLY_MODE:
      return std::make_shared<YOnly>();
    case ZoomFilterMode::HYPERCOS_MODE0:
    case ZoomFilterMode::HYPERCOS_MODE1:
    case ZoomFilterMode::HYPERCOS_MODE2:
    case ZoomFilterMode::HYPERCOS_MODE3:
    case ZoomFilterMode::NORMAL_MODE:
    case ZoomFilterMode::WATER_MODE:
      return std::make_shared<SimpleSpeedCoefficientsEffect>();
    default:
      throw std::logic_error("ZoomVectorEffects::SetFilterSettings: Unknown ZoomFilterMode.");
  }
}

auto FilterSettingsService::GetCurrentFilterMode() const -> std::string
{
  return GetFilterModeString(m_filterMode);
}

auto FilterSettingsService::GetPreviousFilterMode() const -> std::string
{
  return GetFilterModeString(m_previousFilterMode);
}

auto FilterSettingsService::GetFilterModeString(ZoomFilterMode filterMode) -> std::string
{
  switch (filterMode)
  {
    case ZoomFilterMode::AMULET_MODE:
      return "Amulet";
    case ZoomFilterMode::CRYSTAL_BALL_MODE0:
      return "Crystal Ball Mode 0";
    case ZoomFilterMode::CRYSTAL_BALL_MODE1:
      return "Crystal Ball Mode 1";
    case ZoomFilterMode::HYPERCOS_MODE0:
      return "Hypercos Mode 0";
    case ZoomFilterMode::HYPERCOS_MODE1:
      return "Hypercos Mode 1";
    case ZoomFilterMode::HYPERCOS_MODE2:
      return "Hypercos Mode 2";
    case ZoomFilterMode::HYPERCOS_MODE3:
      return "Hypercos Mode 3";
    case ZoomFilterMode::IMAGE_DISPLACEMENT_MODE:
      return "Image Displacement";
    case ZoomFilterMode::NORMAL_MODE:
      return "Normal";
    case ZoomFilterMode::SCRUNCH_MODE:
      return "Scrunch";
    case ZoomFilterMode::SPEEDWAY_MODE:
      return "Speedway";
    case ZoomFilterMode::WATER_MODE:
      return "Water";
    case ZoomFilterMode::WAVE_MODE0:
      return "Wave Mode 0";
    case ZoomFilterMode::WAVE_MODE1:
      return "Wave Mode 1";
    case ZoomFilterMode::Y_ONLY_MODE:
      return "Y Only";
    default:
      return "Unknown";
  }
}

void FilterSettingsService::SetFilterModeSettings()
{
  switch (m_filterMode)
  {
    case ZoomFilterMode::AMULET_MODE:
      SetAmuletModeSettings();
      break;
    case ZoomFilterMode::CRYSTAL_BALL_MODE0:
      SetCrystalBall0ModeSettings();
      break;
    case ZoomFilterMode::CRYSTAL_BALL_MODE1:
      SetCrystalBall1ModeSettings();
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
    case ZoomFilterMode::SPEEDWAY_MODE:
      SetSpeedwayModeSettings();
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
    -> std::shared_ptr<SpeedCoefficientsEffect>&
{
  return m_speedCoefficientsEffect[static_cast<size_t>(m_filterMode)];
}

void FilterSettingsService::SetRandomFilterSettings(const ZoomFilterMode mode)
{
  m_settingsHaveChanged = true;

  m_previousFilterMode = m_filterMode;
  m_filterMode = mode;

  SetDefaultSettings();

  SetFilterModeSettings();
}

void FilterSettingsService::SetDefaultSettings()
{
  m_filterSettings.zoomMidPoint = m_midScreenPoint;

  m_filterSettings.vitesse.SetDefault();

  m_filterSettings.tanEffect = ProbabilityOfMInN(1, 10);
  m_filterSettings.planeEffect = ProbabilityOfMInN(8, 10);
  m_filterSettings.rotateSpeed = 0.0;
  m_filterSettings.noisify = false;
  m_filterSettings.noiseFactor = 1.0;
  m_filterSettings.blockyWavy = false;

  m_filterSettings.hypercosOverlay = HypercosOverlay::NONE;
  m_filterSettings.speedCoefficientsEffect = GetSpeedCoefficientsEffect();
}

inline void FilterSettingsService::SetAmuletModeSettings()
{
  SetRotate(PROB_HIGH);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode2Settings();
  }
}

inline void FilterSettingsService::SetCrystalBall0ModeSettings()
{
  SetRotate(PROB_LOW);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

// TODO - Fix duplication
inline void FilterSettingsService::SetCrystalBall1ModeSettings()
{
  SetRotate(PROB_HALF);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode2Settings();
  }
}

inline void FilterSettingsService::SetHypercosMode0Settings()
{
  SetRotate(PROB_LOW);

  m_filterSettings.hypercosOverlay = HypercosOverlay::MODE0;
}

inline void FilterSettingsService::SetHypercosMode1Settings()
{
  SetRotate(PROB_LOW);

  m_filterSettings.hypercosOverlay = HypercosOverlay::MODE1;
}

inline void FilterSettingsService::SetHypercosMode2Settings()
{
  SetRotate(PROB_LOW);

  m_filterSettings.hypercosOverlay = HypercosOverlay::MODE2;
}

inline void FilterSettingsService::SetHypercosMode3Settings()
{
  SetRotate(PROB_LOW);

  m_filterSettings.hypercosOverlay = HypercosOverlay::MODE3;
}

inline void FilterSettingsService::SetImageDisplacementModeSettings()
{
  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

inline void FilterSettingsService::SetNormalModeSettings()
{
  // No extra settings required.
}

inline void FilterSettingsService::SetScrunchModeSettings()
{
  SetRotate(PROB_HALF);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

inline void FilterSettingsService::SetSpeedwayModeSettings()
{
  SetRotate(PROB_LOW);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode0Settings();
  }
}

inline void FilterSettingsService::SetWaterModeSettings()
{
  // Maybe one day
}

inline void FilterSettingsService::SetWaveMode0Settings()
{
  SetWaveModeSettings();
}

inline void FilterSettingsService::SetWaveMode1Settings()
{
  SetWaveModeSettings();
}

void FilterSettingsService::SetWaveModeSettings()
{
  SetRotate(PROB_HIGH);

  m_filterSettings.vitesse.SetReverseVitesse(
      m_filterEvents->Happens(FilterEventTypes::REVERSE_SPEED));

  if (m_filterEvents->Happens(FilterEventTypes::CHANGE_SPEED))
  {
    m_filterSettings.vitesse.SetVitesse(
        (m_filterSettings.vitesse.GetVitesse() + Vitesse::DEFAULT_VITESSE) / 2);
  }

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

inline void FilterSettingsService::SetYOnlyModeSettings()
{
  SetRotate(PROB_HALF);

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

  m_settingsHaveChanged = true;

  m_filterSettings.rotateSpeed =
      rotateProbability *
      GetRandInRange(ZoomFilterSettings::MIN_ROTATE_SPEED, ZoomFilterSettings::MAX_ROTATE_SPEED);
}

void FilterSettingsService::SetRandomMiddlePoints()
{
  if ((m_filterMode == ZoomFilterMode::WATER_MODE) ||
      (m_filterMode == ZoomFilterMode::WAVE_MODE0) || (m_filterMode == ZoomFilterMode::AMULET_MODE))
  {
    m_filterSettings.zoomMidPoint = m_midScreenPoint;
    return;
  }

  if (((m_filterMode == ZoomFilterMode::CRYSTAL_BALL_MODE0) ||
       (m_filterMode == ZoomFilterMode::CRYSTAL_BALL_MODE1)) &&
      m_filterEvents->Happens(FilterEventTypes::CRYSTAL_BALL_IN_MIDDLE))
  {
    m_filterSettings.zoomMidPoint = m_midScreenPoint;
    return;
    }
    if ((m_filterMode == ZoomFilterMode::WAVE_MODE1) &&
        m_filterEvents->Happens(FilterEventTypes::WAVE_IN_MIDDLE))
    {
      m_filterSettings.zoomMidPoint = m_midScreenPoint;
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
      m_filterSettings.zoomMidPoint = {m_goomInfo->GetScreenInfo().width / 2,
                                       m_goomInfo->GetScreenInfo().height - 1};
      break;
    case MiddlePointEvents::EVENT2:
      m_filterSettings.zoomMidPoint.x = static_cast<int32_t>(m_goomInfo->GetScreenInfo().width - 1);
      break;
    case MiddlePointEvents::EVENT3:
      m_filterSettings.zoomMidPoint.x = 1;
      break;
    case MiddlePointEvents::EVENT4:
      m_filterSettings.zoomMidPoint = m_midScreenPoint;
      break;
    case MiddlePointEvents::EVENT5:
      m_filterSettings.zoomMidPoint = {m_goomInfo->GetScreenInfo().width / 4,
                                       m_goomInfo->GetScreenInfo().height / 4};
      break;
    case MiddlePointEvents::EVENT6:
      m_filterSettings.zoomMidPoint = {(3 * m_goomInfo->GetScreenInfo().width) / 4,
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
