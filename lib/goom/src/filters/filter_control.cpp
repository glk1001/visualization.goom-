#include "filter_control.h"

#include "filter_amulet.h"
#include "filter_buffers_service.h"
#include "filter_crystal_ball.h"
#include "filter_hypercos.h"
#include "filter_image_displacements.h"
#include "filter_scrunch.h"
#include "filter_simple_speed_coefficients_effect.h"
#include "filter_speedway.h"
#include "filter_wave.h"
#include "filter_y_only.h"
#include "filter_zoom_colors.h"
#include "filter_zoom_vector.h"
#include "goom/filter_data.h"
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
const Weights<ZoomFilterMode> FilterControl::WEIGHTED_FILTER_EVENTS{{
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

class FilterControl::FilterEvents
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
const std::array<FilterControl::FilterEvents::Event, FilterControl::FilterEvents::NUM_FILTER_EVENT_TYPES>
    FilterControl::FilterEvents::WEIGHTED_EVENTS{{
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

inline auto FilterControl::FilterEvents::Happens(const FilterEventTypes event) -> bool
{
  const Event& weightedEvent = WEIGHTED_EVENTS[static_cast<size_t>(event)];
  return ProbabilityOfMInN(weightedEvent.m, weightedEvent.outOf);
}

using FilterEventTypes = FilterControl::FilterEvents::FilterEventTypes;

FilterControl::FilterControl(UTILS::Parallel& p,
                             const std::shared_ptr<const PluginInfo>& goomInfo,
                             const std::string& resourcesDirectory) noexcept
  : m_goomInfo{goomInfo},
    m_midScreenPoint{m_goomInfo->GetScreenInfo().width / 2, m_goomInfo->GetScreenInfo().height / 2},
    m_zoomFilterBuffersService{p, m_goomInfo,
                               std::make_unique<FilterZoomVector>(resourcesDirectory)},
    m_filterEvents{spimpl::make_unique_impl<FilterEvents>()},
    m_speedCoefficientsEffect{MakeSpeedCoefficientsEffects(resourcesDirectory)}
{
}

inline auto FilterControl::GetSpeedCoefficientsEffect(const ZoomFilterMode mode)
    -> std::shared_ptr<SpeedCoefficientsEffect>&
{
  return m_speedCoefficientsEffect[static_cast<size_t>(mode)];
}

auto FilterControl::MakeSpeedCoefficientsEffects(const std::string& resourcesDirectory)
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

auto FilterControl::MakeSpeedCoefficientsEffect(const ZoomFilterMode mode,
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

auto FilterControl::GetZoomFilterBuffersService() -> ZoomFilterBuffersService&
{
  return m_zoomFilterBuffersService;
}

auto FilterControl::GetZoomFilterColors() -> ZoomFilterColors&
{
  return m_filterColors;
}

void FilterControl::Start()
{
  SetRandomFilterSettings();
  SetMiddlePoints();

  UpdateFilterSettings();
}

void FilterControl::UpdateFilterSettings()
{
  m_zoomFilterBuffersService.SetFilterSettings(m_filterData);

  m_zoomFilterBuffersService.SetSpeedCoefficientsEffect(
      GetSpeedCoefficientsEffect(m_filterData.mode));

  m_filterColors.SetBlockWavy(m_filterData.blockyWavy);

  m_settingsHaveChanged = false;
}

void FilterControl::SetRandomFilterSettings(const ZoomFilterMode mode)
{
  m_settingsHaveChanged = true;

  m_filterData.SetMode(mode);

  SetDefaultSettings();

  SetFilterModeSettings();
}

void FilterControl::SetFilterModeSettings()
{
  switch (m_filterData.mode)
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

auto FilterControl::GetNewRandomMode() const -> ZoomFilterMode
{
  uint32_t numTries = 0;
  constexpr uint32_t MAX_TRIES = 20;

  while (true)
  {
    const auto newMode = WEIGHTED_FILTER_EVENTS.GetRandomWeighted();
    if (newMode != m_filterData.mode)
    {
      return newMode;
    }
    ++numTries;
    if (numTries >= MAX_TRIES)
    {
      break;
    }
  }

  return m_filterData.mode;
}

void FilterControl::SetDefaultSettings()
{
  m_filterData.zoomMidPoint = m_midScreenPoint;

  m_filterData.vitesse.SetDefault();

  m_filterData.tanEffect = ProbabilityOfMInN(1, 10);
  m_filterData.rotateSpeed = 0.0;
  m_filterData.noisify = false;
  m_filterData.noiseFactor = 1.0;
  m_filterData.blockyWavy = false;

  m_filterData.hypercosOverlay = HypercosOverlay::NONE;
}

void FilterControl::SetAmuletModeSettings()
{
  SetRotate(PROB_HIGH);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode2Settings();
  }
}

void FilterControl::SetCrystalBall0ModeSettings()
{
  SetRotate(PROB_LOW);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

// TODO - Fix duplication
void FilterControl::SetCrystalBall1ModeSettings()
{
  SetRotate(PROB_HALF);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode2Settings();
  }
}

void FilterControl::SetHypercosMode0Settings()
{
  SetRotate(PROB_LOW);

  m_filterData.hypercosOverlay = HypercosOverlay::MODE0;
}

void FilterControl::SetHypercosMode1Settings()
{
  SetRotate(PROB_LOW);

  m_filterData.hypercosOverlay = HypercosOverlay::MODE1;
}

void FilterControl::SetHypercosMode2Settings()
{
  SetRotate(PROB_LOW);

  m_filterData.hypercosOverlay = HypercosOverlay::MODE2;
}

void FilterControl::SetHypercosMode3Settings()
{
  SetRotate(PROB_LOW);

  m_filterData.hypercosOverlay = HypercosOverlay::MODE3;
}

void FilterControl::SetImageDisplacementModeSettings()
{
  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

void FilterControl::SetNormalModeSettings()
{
  // No extra settings required.
}

void FilterControl::SetScrunchModeSettings()
{
  SetRotate(PROB_HALF);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

void FilterControl::SetSpeedwayModeSettings()
{
  SetRotate(PROB_LOW);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode0Settings();
  }
}

void FilterControl::SetWaterModeSettings()
{
  // Maybe one day
}

void FilterControl::SetWaveMode0Settings()
{
  SetWaveModeSettings();
}

void FilterControl::SetWaveMode1Settings()
{
  SetWaveModeSettings();
}

void FilterControl::SetWaveModeSettings()
{
  SetRotate(PROB_HIGH);

  m_filterData.vitesse.SetReverseVitesse(m_filterEvents->Happens(FilterEventTypes::REVERSE_SPEED));

  if (m_filterEvents->Happens(FilterEventTypes::CHANGE_SPEED))
  {
    m_filterData.vitesse.SetVitesse((m_filterData.vitesse.GetVitesse() + Vitesse::DEFAULT_VITESSE) /
                                    2);
  }

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

void FilterControl::SetYOnlyModeSettings()
{
  SetRotate(PROB_HALF);

  if (m_filterEvents->Happens(FilterEventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

void FilterControl::SetRotate(const float rotateProbability)
{
  if (!ProbabilityOf(rotateProbability))
  {
    return;
  }

  m_settingsHaveChanged = true;

  m_filterData.rotateSpeed = rotateProbability * GetRandInRange(ZoomFilterData::MIN_ROTATE_SPEED,
                                                                ZoomFilterData::MAX_ROTATE_SPEED);
}

void FilterControl::SetRandomMiddlePoints()
{
  if ((m_filterData.mode == ZoomFilterMode::WATER_MODE) ||
      (m_filterData.mode == ZoomFilterMode::WAVE_MODE0) ||
      (m_filterData.mode == ZoomFilterMode::AMULET_MODE))
  {
    m_filterData.zoomMidPoint = m_midScreenPoint;
    return;
  }

  if (((m_filterData.mode == ZoomFilterMode::CRYSTAL_BALL_MODE0) ||
       (m_filterData.mode == ZoomFilterMode::CRYSTAL_BALL_MODE1)) &&
      m_filterEvents->Happens(FilterEventTypes::CRYSTAL_BALL_IN_MIDDLE))
  {
    m_filterData.zoomMidPoint = m_midScreenPoint;
    return;
    }
    if ((m_filterData.mode == ZoomFilterMode::WAVE_MODE1) &&
        m_filterEvents->Happens(FilterEventTypes::WAVE_IN_MIDDLE))
    {
      m_filterData.zoomMidPoint = m_midScreenPoint;
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
      m_filterData.zoomMidPoint = {m_goomInfo->GetScreenInfo().width / 2,
                                   m_goomInfo->GetScreenInfo().height - 1};
      break;
    case MiddlePointEvents::EVENT2:
      m_filterData.zoomMidPoint.x = static_cast<int32_t>(m_goomInfo->GetScreenInfo().width - 1);
      break;
    case MiddlePointEvents::EVENT3:
      m_filterData.zoomMidPoint.x = 1;
      break;
    case MiddlePointEvents::EVENT4:
      m_filterData.zoomMidPoint = m_midScreenPoint;
      break;
    case MiddlePointEvents::EVENT5:
      m_filterData.zoomMidPoint = {m_goomInfo->GetScreenInfo().width / 4,
                                   m_goomInfo->GetScreenInfo().height / 4};
      break;
    case MiddlePointEvents::EVENT6:
      m_filterData.zoomMidPoint = {(3 * m_goomInfo->GetScreenInfo().width) / 4,
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
