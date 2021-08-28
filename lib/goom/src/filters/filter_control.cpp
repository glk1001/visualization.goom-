#include "filter_control.h"

#include "filter_data.h"
#include "goom/goom_plugin_info.h"
#include "goomutils/enumutils.h"
#include "goomutils/goomrand.h"
#include "goomutils/graphics/image_bitmaps.h"

#include <stdexcept>
#include <string>

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
    { ZoomFilterMode::AMULET_MODE,            4 },
    { ZoomFilterMode::CRYSTAL_BALL_MODE0,     4 },
    { ZoomFilterMode::CRYSTAL_BALL_MODE1,     2 },
    { ZoomFilterMode::HYPERCOS_MODE0,         6 },
    { ZoomFilterMode::HYPERCOS_MODE1,         5 },
    { ZoomFilterMode::HYPERCOS_MODE2,         3 },
    { ZoomFilterMode::HYPERCOS_MODE3,         3 },
    { ZoomFilterMode::IMAGE_DISPLACEMENT_MODE,5 },
    { ZoomFilterMode::NORMAL_MODE,            6 },
    { ZoomFilterMode::SCRUNCH_MODE,           6 },
    { ZoomFilterMode::SPEEDWAY_MODE,          6 },
    { ZoomFilterMode::WAVE_MODE0,             5 },
    { ZoomFilterMode::WAVE_MODE1,             4 },
    { ZoomFilterMode::WATER_MODE,             0 },
    { ZoomFilterMode::Y_ONLY_MODE,            4 },
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
    ZERO_H_PLANE_EFFECT,
    PLANE_AMP_EQUAL,
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
     { .event = FilterEventTypes::DISPL_CUTOFF_EQUAL,         .m =  8, .outOf = 16 },
     { .event = FilterEventTypes::ZERO_H_PLANE_EFFECT         .m =  8, .outOf = 16 },
     { .event = FilterEventTypes::PLANE_AMP_EQUAL,            .m = 12, .outOf = 16 },
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
   {/*.event = */ FilterEventTypes::ZERO_H_PLANE_EFFECT,       /*.m = */  8, /*.outOf = */ 16},
   {/*.event = */ FilterEventTypes::PLANE_AMP_EQUAL,           /*.m = */ 12, /*.outOf = */ 16},
}};
// clang-format on
//@formatter:on
#endif

inline auto FilterControl::FilterEvents::Happens(const FilterEventTypes event) -> bool
{
  const Event& weightedEvent = WEIGHTED_EVENTS[static_cast<size_t>(event)];
  return ProbabilityOfMInN(weightedEvent.m, weightedEvent.outOf);
}

FilterControl::FilterControl(const std::shared_ptr<const PluginInfo>& goomInfo) noexcept
  : m_goomInfo{goomInfo},
    m_midScreenPoint{m_goomInfo->GetScreenInfo().width / 2, m_goomInfo->GetScreenInfo().height / 2},
    m_filterEvents{spimpl::make_unique_impl<FilterEvents>()}
{
}

void FilterControl::Start()
{
  if (m_resourcesDirectory.empty())
  {
    throw std::logic_error("FilterControl::Start: Resources directory is empty.");
  }
}

void FilterControl::SetRandomFilterSettings()
{
  SetRandomFilterSettings(GetNewRandomMode());
}

void FilterControl::SetDefaultFilterSettings(const ZoomFilterMode mode)
{
  m_settingsHaveChanged = true;

  m_filterData.SetMode(mode);

  SetDefaultSettings();
}

void FilterControl::SetRandomFilterSettings(const ZoomFilterMode mode)
{
  m_settingsHaveChanged = true;

  m_filterData.SetMode(mode);

  SetDefaultSettings();

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
    case ZoomFilterMode::WAVE_MODE1:
      SetWaveModeSettings();
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

  m_filterData.hPlaneEffect = 0;
  m_filterData.hPlaneEffectAmplitude = ZoomFilterData::DEFAULT_H_PLANE_EFFECT_AMPLITUDE;

  m_filterData.vPlaneEffect = 0;
  m_filterData.vPlaneEffectAmplitude = ZoomFilterData::DEFAULT_V_PLANE_EFFECT_AMPLITUDE;

  m_filterData.hypercosOverlay = HypercosOverlay::NONE;
}

void FilterControl::SetAmuletModeSettings()
{
  using EventTypes = FilterControl::FilterEvents::FilterEventTypes;

  SetRotate(PROB_HIGH);

  if (m_filterEvents->Happens(EventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode2Settings();
  }
}

void FilterControl::SetCrystalBall0ModeSettings()
{
  using EventTypes = FilterControl::FilterEvents::FilterEventTypes;

  SetRotate(PROB_LOW);

  if (m_filterEvents->Happens(EventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

// TODO - Fix duplication
void FilterControl::SetCrystalBall1ModeSettings()
{
  using EventTypes = FilterControl::FilterEvents::FilterEventTypes;

  SetRotate(PROB_HALF);

  if (m_filterEvents->Happens(EventTypes::HYPERCOS_EFFECT))
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
  using EventTypes = FilterControl::FilterEvents::FilterEventTypes;
  if (m_filterEvents->Happens(EventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

void FilterControl::SetNormalModeSettings()
{
}

void FilterControl::SetScrunchModeSettings()
{
  SetRotate(PROB_HALF);

  using EventTypes = FilterControl::FilterEvents::FilterEventTypes;
  if (m_filterEvents->Happens(EventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

void FilterControl::SetSpeedwayModeSettings()
{
  SetRotate(PROB_LOW);

  using EventTypes = FilterControl::FilterEvents::FilterEventTypes;
  if (m_filterEvents->Happens(EventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode0Settings();
  }
}

void FilterControl::SetWaterModeSettings()
{
}

void FilterControl::SetWaveModeSettings()
{
  SetRotate(PROB_HIGH);

  using EventTypes = FilterControl::FilterEvents::FilterEventTypes;

  m_filterData.vitesse.SetReverseVitesse(m_filterEvents->Happens(EventTypes::REVERSE_SPEED));

  if (m_filterEvents->Happens(EventTypes::CHANGE_SPEED))
  {
    m_filterData.vitesse.SetVitesse((m_filterData.vitesse.GetVitesse() + Vitesse::DEFAULT_VITESSE) /
                                    2);
  }

  if (m_filterEvents->Happens(EventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

void FilterControl::SetYOnlyModeSettings()
{
  SetRotate(PROB_HALF);

  using EventTypes = FilterControl::FilterEvents::FilterEventTypes;
  if (m_filterEvents->Happens(EventTypes::HYPERCOS_EFFECT))
  {
    SetHypercosMode1Settings();
  }
}

void FilterControl::SetRotate(const float probability)
{
  if (!ProbabilityOf(probability))
  {
    return;
  }

  m_filterData.rotateSpeed = probability * GetRandInRange(ZoomFilterData::MIN_ROTATE_SPEED,
                                                          ZoomFilterData::MAX_ROTATE_SPEED);
}

void FilterControl::ChangeMilieu()
{
  m_settingsHaveChanged = true;

  SetMiddlePoints();
  SetPlaneEffects();
}

void FilterControl::SetMiddlePoints()
{
  using EventTypes = FilterControl::FilterEvents::FilterEventTypes;

  if ((m_filterData.mode == ZoomFilterMode::WATER_MODE) ||
      (m_filterData.mode == ZoomFilterMode::WAVE_MODE0) ||
      (m_filterData.mode == ZoomFilterMode::AMULET_MODE))
  {
    m_filterData.zoomMidPoint = m_midScreenPoint;
    return;
  }

  if (((m_filterData.mode == ZoomFilterMode::CRYSTAL_BALL_MODE0) ||
       (m_filterData.mode == ZoomFilterMode::CRYSTAL_BALL_MODE1)) &&
      m_filterEvents->Happens(EventTypes::CRYSTAL_BALL_IN_MIDDLE))
  {
    m_filterData.zoomMidPoint = m_midScreenPoint;
    return;
    }
    if ((m_filterData.mode == ZoomFilterMode::WAVE_MODE1) &&
        m_filterEvents->Happens(EventTypes::WAVE_IN_MIDDLE))
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

void FilterControl::SetPlaneEffects()
{
  using EventTypes = FilterControl::FilterEvents::FilterEventTypes;

  // clang-format off
  // @formatter:off
  enum class PlaneEffectEvents { EVENT1, EVENT2, EVENT3, EVENT4, EVENT5, EVENT6, EVENT7, EVENT8 };
  static const Weights<PlaneEffectEvents> s_planeEffectWeights{{
     { PlaneEffectEvents::EVENT1,  1 },
     { PlaneEffectEvents::EVENT2,  1 },
     { PlaneEffectEvents::EVENT3,  4 },
     { PlaneEffectEvents::EVENT4,  1 },
     { PlaneEffectEvents::EVENT5,  1 },
     { PlaneEffectEvents::EVENT6,  1 },
     { PlaneEffectEvents::EVENT7,  1 },
     { PlaneEffectEvents::EVENT8,  2 },
  }};
  // clang-format on
  // @formatter:on

  switch (s_planeEffectWeights.GetRandomWeighted())
  {
    case PlaneEffectEvents::EVENT1:
      m_filterData.vPlaneEffect = GetRandInRange(-2, +3);
      m_filterData.hPlaneEffect = GetRandInRange(-2, +3);
      break;
    case PlaneEffectEvents::EVENT2:
      m_filterData.vPlaneEffect = 0;
      m_filterData.hPlaneEffect = GetRandInRange(-7, +8);
      break;
    case PlaneEffectEvents::EVENT3:
      m_filterData.vPlaneEffect = GetRandInRange(-5, +6);
      m_filterData.hPlaneEffect = -m_filterData.vPlaneEffect + 1;
      break;
    case PlaneEffectEvents::EVENT4:
      m_filterData.hPlaneEffect = static_cast<int>(GetRandInRange(5U, 13U));
      m_filterData.vPlaneEffect = -m_filterData.hPlaneEffect + 1;
      break;
    case PlaneEffectEvents::EVENT5:
      m_filterData.vPlaneEffect = static_cast<int>(GetRandInRange(5U, 13U));
      m_filterData.hPlaneEffect = -m_filterData.hPlaneEffect + 1;
      break;
    case PlaneEffectEvents::EVENT6:
      m_filterData.hPlaneEffect = 0;
      m_filterData.vPlaneEffect = GetRandInRange(-9, +10);
      break;
    case PlaneEffectEvents::EVENT7:
      m_filterData.hPlaneEffect = GetRandInRange(-9, +10);
      m_filterData.vPlaneEffect = GetRandInRange(-9, +10);
      break;
    case PlaneEffectEvents::EVENT8:
      m_filterData.vPlaneEffect = 0;
      m_filterData.hPlaneEffect = 0;
      break;
    default:
      throw std::logic_error("Unknown MiddlePointEvents enum.");
  }

  m_filterData.hPlaneEffectAmplitude = GetRandInRange(ZoomFilterData::MIN_H_PLANE_EFFECT_AMPLITUDE,
                                                      ZoomFilterData::MAX_H_PLANE_EFFECT_AMPLITUDE);
  if (m_filterEvents->Happens(FilterEvents::FilterEventTypes::PLANE_AMP_EQUAL))
  {
    m_filterData.vPlaneEffectAmplitude = m_filterData.hPlaneEffectAmplitude;
  }
  else
  {
    m_filterData.vPlaneEffectAmplitude = GetRandInRange(
        ZoomFilterData::MIN_V_PLANE_EFFECT_AMPLITUDE, ZoomFilterData::MAX_V_PLANE_EFFECT_AMPLITUDE);
  }

  if ((1 == m_filterData.zoomMidPoint.x) ||
      (m_filterData.zoomMidPoint.x == static_cast<int32_t>(m_goomInfo->GetScreenInfo().width - 1)))
  {
    m_filterData.vPlaneEffect = 0;
    if (m_filterEvents->Happens(EventTypes::ZERO_H_PLANE_EFFECT))
    {
      m_filterData.hPlaneEffect = 0;
    }
  }
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::DRAW
#endif
