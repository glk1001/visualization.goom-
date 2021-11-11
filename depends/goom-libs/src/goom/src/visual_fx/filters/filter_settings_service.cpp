#include "filter_settings_service.h"

#include "amulet.h"
#include "crystal_ball.h"
#include "filter_buffers_service.h"
#include "filter_colors_service.h"
#include "filter_settings.h"
#include "filter_zoom_vector.h"
#include "goom_plugin_info.h"
#include "image_speed_coeffs.h"
#include "scrunch.h"
#include "simple_speed_coefficients_effect.h"
#include "speedway.h"
#include "utils/enumutils.h"
#include "utils/goom_rand_base.h"
#include "wave.h"
#include "y_only.h"

#include <stdexcept>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace FILTERS
{
#else
namespace GOOM::VISUAL_FX::FILTERS
{
#endif

using UTILS::IGoomRand;
using UTILS::NUM;
using UTILS::Weights;

constexpr float PROB_HIGH = 0.9F;
constexpr float PROB_HALF = 0.5F;
constexpr float PROB_LOW = 0.1F;
constexpr float PROB_ZERO = 0.0F;

// clang-format off
auto FilterSettingsService::GetFilterModeData(const std::string& resourcesDirectory) const
      -> std::map<ZoomFilterMode, ZoomFilterModeInfo>
{
  using Hyp = HypercosOverlay;

  const auto amuletRotateProb     = PROB_HIGH;
  const auto crysBall0RotateProb  = PROB_HIGH;
  const auto crysBall1RotateProb  = PROB_HIGH;
  const auto hypercos0RotateProb  = PROB_LOW;
  const auto hypercos1RotateProb  = PROB_LOW;
  const auto hypercos2RotateProb  = PROB_LOW;
  const auto hypercos3RotateProb  = PROB_LOW;
  const auto imageDisplRotateProb = PROB_ZERO;
  const auto normalRotateProb     = PROB_ZERO;
  const auto scrunchRotateProb    = PROB_HALF;
  const auto speedway0RotateProb  = PROB_LOW;
  const auto speedway1RotateProb  = PROB_LOW;
  const auto waterRotateProb      = PROB_ZERO;
  const auto wave0RotateProb      = PROB_LOW;
  const auto wave1RotateProb      = PROB_HALF;
  const auto yOnlyRotateProb      = PROB_HALF;

  const auto amuletWeights     = Weights<Hyp>{m_goomRand, {{Hyp::NONE, 20}, {Hyp::MODE0,  1}, {Hyp::MODE1,  5}, {Hyp::MODE2,  1}, {Hyp::MODE3,  1}}};
  const auto crysBall0Weights  = Weights<Hyp>{m_goomRand, {{Hyp::NONE,  5}, {Hyp::MODE0, 10}, {Hyp::MODE1,  1}, {Hyp::MODE2,  1}, {Hyp::MODE3,  1}}};
  const auto crysBall1Weights  = Weights<Hyp>{m_goomRand, {{Hyp::NONE,  5}, {Hyp::MODE0,  1}, {Hyp::MODE1, 99}, {Hyp::MODE2,  1}, {Hyp::MODE3,  1}}};
  const auto hypercos0Weights  = Weights<Hyp>{m_goomRand, {{Hyp::NONE,  0}, {Hyp::MODE0,  1}, {Hyp::MODE1,  0}, {Hyp::MODE2,  0}, {Hyp::MODE3,  0}}};
  const auto hypercos1Weights  = Weights<Hyp>{m_goomRand, {{Hyp::NONE,  0}, {Hyp::MODE0,  0}, {Hyp::MODE1,  1}, {Hyp::MODE2,  0}, {Hyp::MODE3,  0}}};
  const auto hypercos2Weights  = Weights<Hyp>{m_goomRand, {{Hyp::NONE,  0}, {Hyp::MODE0,  0}, {Hyp::MODE1,  0}, {Hyp::MODE2,  1}, {Hyp::MODE3,  0}}};
  const auto hypercos3Weights  = Weights<Hyp>{m_goomRand, {{Hyp::NONE,  0}, {Hyp::MODE0,  0}, {Hyp::MODE1,  0}, {Hyp::MODE2,  0}, {Hyp::MODE3,  1}}};
  const auto imageDisplWeights = Weights<Hyp>{m_goomRand, {{Hyp::NONE, 99}, {Hyp::MODE0,  1}, {Hyp::MODE1,  5}, {Hyp::MODE2,  1}, {Hyp::MODE3,  1}}};
  const auto normalWeights     = Weights<Hyp>{m_goomRand, {{Hyp::NONE, 10}, {Hyp::MODE0,  5}, {Hyp::MODE1,  1}, {Hyp::MODE2,  1}, {Hyp::MODE3,  1}}};
  const auto scrunchWeights    = Weights<Hyp>{m_goomRand, {{Hyp::NONE, 10}, {Hyp::MODE0,  1}, {Hyp::MODE1,  5}, {Hyp::MODE2,  1}, {Hyp::MODE3,  1}}};
  const auto speedway0Weights  = Weights<Hyp>{m_goomRand, {{Hyp::NONE, 10}, {Hyp::MODE0,  5}, {Hyp::MODE1,  1}, {Hyp::MODE2,  1}, {Hyp::MODE3,  1}}};
  const auto speedway1Weights  = Weights<Hyp>{m_goomRand, {{Hyp::NONE, 10}, {Hyp::MODE0,  5}, {Hyp::MODE1,  1}, {Hyp::MODE2,  1}, {Hyp::MODE3,  1}}};
  const auto waterWeights      = Weights<Hyp>{m_goomRand, {{Hyp::NONE, 10}, {Hyp::MODE0,  1}, {Hyp::MODE1,  1}, {Hyp::MODE2,  1}, {Hyp::MODE3,  1}}};
  const auto wave0Weights      = Weights<Hyp>{m_goomRand, {{Hyp::NONE, 10}, {Hyp::MODE0,  5}, {Hyp::MODE1,  1}, {Hyp::MODE2,  1}, {Hyp::MODE3,  1}}};
  const auto wave1Weights      = Weights<Hyp>{m_goomRand, {{Hyp::NONE, 10}, {Hyp::MODE0,  1}, {Hyp::MODE1,  5}, {Hyp::MODE2,  1}, {Hyp::MODE3,  1}}};
  const auto yOnlyWeights      = Weights<Hyp>{m_goomRand, {{Hyp::NONE, 10}, {Hyp::MODE0,  1}, {Hyp::MODE1,  5}, {Hyp::MODE2,  1}, {Hyp::MODE3,  1}}};

  return {
    { ZoomFilterMode::AMULET_MODE, {
        "Amulet",
        std::make_shared<Amulet>(m_goomRand),
        amuletRotateProb,
        amuletWeights
      }
    },
    { ZoomFilterMode::CRYSTAL_BALL_MODE0, {
        "Crystal Ball Mode 0", 
        std::make_shared<CrystalBall>(CrystalBall::Modes::MODE0, m_goomRand),
        crysBall0RotateProb,
        crysBall0Weights
      }
    },
    { ZoomFilterMode::CRYSTAL_BALL_MODE1, {
        "Crystal Ball Mode 1", 
        std::make_shared<CrystalBall>(CrystalBall::Modes::MODE1, m_goomRand),
        crysBall1RotateProb,
        crysBall1Weights
      }
    },
    { ZoomFilterMode::HYPERCOS_MODE0, {
        "Hypercos Mode 0",  
        std::make_shared<SimpleSpeedCoefficientsEffect>(),
        hypercos0RotateProb, 
        hypercos0Weights
      }
    },
    { ZoomFilterMode::HYPERCOS_MODE1, {
        "Hypercos Mode 1",  
        std::make_shared<SimpleSpeedCoefficientsEffect>(),
        hypercos1RotateProb, 
        hypercos1Weights
      }
    },
    { ZoomFilterMode::HYPERCOS_MODE2, {
        "Hypercos Mode 2",  
        std::make_shared<SimpleSpeedCoefficientsEffect>(),
        hypercos2RotateProb, 
        hypercos2Weights
      }
    },
    { ZoomFilterMode::HYPERCOS_MODE3, {
        "Hypercos Mode 3",  
        std::make_shared<SimpleSpeedCoefficientsEffect>(),
        hypercos3RotateProb, 
        hypercos3Weights
      }
    },
    { ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, {
        "Image Displacement",  
        std::make_shared<ImageSpeedCoefficients>(resourcesDirectory, m_goomRand),
        imageDisplRotateProb, 
        imageDisplWeights
      }
    },
    { ZoomFilterMode::NORMAL_MODE, {
        "Normal",  
        std::make_shared<SimpleSpeedCoefficientsEffect>(),
        normalRotateProb, 
        normalWeights
      }
    },
    { ZoomFilterMode::SCRUNCH_MODE, {
        "Scrunch",  
        std::make_shared<Scrunch>(m_goomRand),
        scrunchRotateProb, 
        scrunchWeights
      }
    },
    { ZoomFilterMode::SPEEDWAY_MODE0, {
        "Speedway Mode 0",  
        std::make_shared<Speedway>(Speedway::Modes::MODE0, m_goomRand),
        speedway0RotateProb, 
        speedway0Weights
      }
    },
    { ZoomFilterMode::SPEEDWAY_MODE1, {
        "Speedway Mode 1",  
        std::make_shared<Speedway>(Speedway::Modes::MODE1, m_goomRand),
        speedway1RotateProb, 
        speedway1Weights
      }
    },
    { ZoomFilterMode::WATER_MODE, {
        "Water",  
        std::make_shared<SimpleSpeedCoefficientsEffect>(),
        waterRotateProb, 
        waterWeights
      }
    },
    { ZoomFilterMode::WAVE_MODE0, {
        "Wave Mode 0",  
        std::make_shared<Wave>(Wave::Modes::MODE0, m_goomRand),
        wave0RotateProb, 
        wave0Weights
      }
    },
    { ZoomFilterMode::WAVE_MODE1, {
        "Wave Mode 1",  
        std::make_shared<Wave>(Wave::Modes::MODE1, m_goomRand),
        wave1RotateProb, 
        wave1Weights
      }
    },
    { ZoomFilterMode::Y_ONLY_MODE, {
        "Y Only",  
        std::make_shared<YOnly>(m_goomRand),
        yOnlyRotateProb, 
        yOnlyWeights
      }
    },
  };
}
// clang-format on

class FilterSettingsService::FilterEvents
{
public:
  explicit FilterEvents(IGoomRand& goomRand) noexcept;

  enum class FilterEventTypes
  {
    ROTATE = 0,
    CRYSTAL_BALL_IN_MIDDLE,
    WAVE_IN_MIDDLE,
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

  [[nodiscard]] auto Happens(FilterEventTypes event) const -> bool;

private:
  IGoomRand& m_goomRand;
  static constexpr size_t NUM_FILTER_EVENT_TYPES = NUM<FilterEventTypes>;

  //@formatter:off
  // clang-format off
#if __cplusplus <= 201703L
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

#if __cplusplus <= 201703L
// clang-format off
const std::array<FilterSettingsService::FilterEvents::Event,
                 FilterSettingsService::FilterEvents::NUM_FILTER_EVENT_TYPES>
  FilterSettingsService::FilterEvents::WEIGHTED_EVENTS{{
      {/*.event = */ FilterEventTypes::ROTATE,                    /*.m = */  8, /*.outOf = */ 16},
      {/*.event = */ FilterEventTypes::CRYSTAL_BALL_IN_MIDDLE,    /*.m = */ 14, /*.outOf = */ 16},
      {/*.event = */ FilterEventTypes::WAVE_IN_MIDDLE,            /*.m = */  8, /*.outOf = */ 16},
      {/*.event = */ FilterEventTypes::CHANGE_SPEED,              /*.m = */  8, /*.outOf = */ 16},
      {/*.event = */ FilterEventTypes::REVERSE_SPEED,             /*.m = */  8, /*.outOf = */ 16},
  }};
// clang-format on
#endif

inline FilterSettingsService::FilterEvents::FilterEvents(IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand}
{
}

inline auto FilterSettingsService::FilterEvents::Happens(const FilterEventTypes event) const -> bool
{
  const Event& weightedEvent = WEIGHTED_EVENTS.at(static_cast<size_t>(event));
  return m_goomRand.ProbabilityOfMInN(weightedEvent.m, weightedEvent.outOf);
}

using FilterEventTypes = FilterSettingsService::FilterEvents::FilterEventTypes;

FilterSettingsService::FilterSettingsService(UTILS::Parallel& parallel,
                                             const PluginInfo& goomInfo,
                                             IGoomRand& goomRand,
                                             const std::string& resourcesDirectory) noexcept
  : m_parallel{parallel},
    m_goomInfo{goomInfo},
    m_goomRand{goomRand},
    m_screenMidPoint{m_goomInfo.GetScreenInfo().width / 2, m_goomInfo.GetScreenInfo().height / 2},
    m_resourcesDirectory{resourcesDirectory},
    m_filterEvents{spimpl::make_unique_impl<FilterEvents>(m_goomRand)},
    m_weightedFilterEvents{goomRand,
                           {
                               {ZoomFilterMode::AMULET_MODE, 8},
                               {ZoomFilterMode::CRYSTAL_BALL_MODE0, 4},
                               {ZoomFilterMode::CRYSTAL_BALL_MODE1, 2},
                               {ZoomFilterMode::HYPERCOS_MODE0, 6},
                               {ZoomFilterMode::HYPERCOS_MODE1, 5},
                               {ZoomFilterMode::HYPERCOS_MODE2, 3},
                               {ZoomFilterMode::HYPERCOS_MODE3, 3},
                               {ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, 5},
                               {ZoomFilterMode::NORMAL_MODE, 6},
                               {ZoomFilterMode::SCRUNCH_MODE, 6},
                               {ZoomFilterMode::SPEEDWAY_MODE0, 3},
                               {ZoomFilterMode::SPEEDWAY_MODE1, 3},
                               {ZoomFilterMode::WAVE_MODE0, 5},
                               {ZoomFilterMode::WAVE_MODE1, 4},
                               {ZoomFilterMode::WATER_MODE, 0},
                               {ZoomFilterMode::Y_ONLY_MODE, 4},
                           }},
    m_filterModeData{GetFilterModeData(m_resourcesDirectory)},
    m_filterSettings{{Vitesse{},
                      HypercosOverlay::NONE,
                      DEFAULT_MAX_SPEED_COEFF,
                      nullptr,
                      nullptr,
                      {DEFAULT_ZOOM_MID_X, DEFAULT_ZOOM_MID_Y},
                      false,
                      false,
                      false,
                      false},
                     {DEFAULT_TRAN_LERP_INCREMENT, DEFAULT_SWITCH_MULT},
                     {false, Pixel::BLACK}},
    m_zoomMidPointWeights{m_goomRand,
                          {
                              {ZoomMidPointEvents::EVENT1, 3},
                              {ZoomMidPointEvents::EVENT2, 2},
                              {ZoomMidPointEvents::EVENT3, 2},
                              {ZoomMidPointEvents::EVENT4, 18},
                              {ZoomMidPointEvents::EVENT5, 10},
                              {ZoomMidPointEvents::EVENT6, 10},
                          }}
{
}

auto FilterSettingsService::GetFilterBuffersService() -> std::unique_ptr<FilterBuffersService>
{
  return std::make_unique<FilterBuffersService>(
      m_parallel, m_goomInfo,
      std::make_unique<FilterZoomVector>(m_goomInfo.GetScreenInfo().width, m_resourcesDirectory,
                                         m_goomRand));
}

auto FilterSettingsService::GetFilterColorsService() const -> std::unique_ptr<FilterColorsService>
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

auto FilterSettingsService::GetNewRandomMode() const -> ZoomFilterMode
{
  uint32_t numTries = 0;
  constexpr uint32_t MAX_TRIES = 20;

  while (true)
  {
    const auto newMode = m_weightedFilterEvents.GetRandomWeighted();
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
}

inline auto FilterSettingsService::GetSpeedCoefficientsEffect()
    -> std::shared_ptr<ISpeedCoefficientsEffect>&
{
  return m_filterModeData.at(m_filterMode).speedCoefficientsEffect;
}

inline auto FilterSettingsService::GetRotation() const -> std::shared_ptr<Rotation>
{
  return std::make_shared<Rotation>(m_goomRand);
}

void FilterSettingsService::SetRandomSettingsForNewFilterMode()
{
  SetDefaultSettings();
  SetRandomEffects();
  SetRandomZoomMidPoint();
  SetFilterModeSettings();
}

void FilterSettingsService::SetDefaultSettings()
{
  m_filterSettings.filterEffectsSettings.zoomMidPoint = m_screenMidPoint;

  m_filterSettings.filterEffectsSettings.vitesse.SetDefault();

  m_filterSettings.filterEffectsSettings.hypercosOverlay = HypercosOverlay::NONE;
  m_filterSettings.filterEffectsSettings.imageVelocityEffect = false;
  m_filterSettings.filterEffectsSettings.tanEffect = false;
  m_filterSettings.filterEffectsSettings.planeEffect = false;
  m_filterSettings.filterEffectsSettings.noiseEffect = false;

  m_filterSettings.filterColorSettings.blockyWavy = false;
  m_filterSettings.filterColorSettings.clippedColor = Pixel::BLACK;
}

void FilterSettingsService::SetRandomEffects()
{
  m_filterSettings.filterEffectsSettings.imageVelocityEffect = m_goomRand.ProbabilityOfMInN(1, 10);
  m_filterSettings.filterEffectsSettings.tanEffect = m_goomRand.ProbabilityOfMInN(1, 10);
  m_filterSettings.filterEffectsSettings.planeEffect = m_goomRand.ProbabilityOfMInN(8, 10);
}

inline void FilterSettingsService::SetFilterModeSettings()
{
  m_filterSettings.filterEffectsSettings.speedCoefficientsEffect = GetSpeedCoefficientsEffect();
  m_filterSettings.filterEffectsSettings.rotation = GetRotation();

  const ZoomFilterModeInfo& modeInfo = m_filterModeData.at(m_filterMode);
  SetRotate(modeInfo.rotateProbability);
  m_filterSettings.filterEffectsSettings.hypercosOverlay =
      modeInfo.hypercosWeights.GetRandomWeighted();

  if ((m_filterMode == ZoomFilterMode::WAVE_MODE0) || (m_filterMode == ZoomFilterMode::WAVE_MODE1))
  {
    SetWaveModeSettings();
  }
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
}

inline void FilterSettingsService::SetRotate(const float rotateProbability)
{
  if (!m_goomRand.ProbabilityOf(rotateProbability))
  {
    return;
  }

  m_filterEffectsSettingsHaveChanged = true;

  m_filterSettings.filterEffectsSettings.rotation->SetRandomParams();
  m_filterSettings.filterEffectsSettings.rotation->Multiply(rotateProbability);
}

void FilterSettingsService::SetMaxSpeedCoeff()
{
  m_filterSettings.filterEffectsSettings.maxSpeedCoeff =
      m_goomRand.GetRandInRange(0.5F, 1.0F) * MAX_MAX_SPEED_COEFF;
}

void FilterSettingsService::SetRandomZoomMidPoint()
{
  if ((m_filterMode == ZoomFilterMode::WATER_MODE) ||
      (m_filterMode == ZoomFilterMode::WAVE_MODE0) || (m_filterMode == ZoomFilterMode::AMULET_MODE))
  {
    m_filterSettings.filterEffectsSettings.zoomMidPoint = m_screenMidPoint;
    return;
  }

  if (((m_filterMode == ZoomFilterMode::CRYSTAL_BALL_MODE0) ||
       (m_filterMode == ZoomFilterMode::CRYSTAL_BALL_MODE1)) &&
      m_filterEvents->Happens(FilterEventTypes::CRYSTAL_BALL_IN_MIDDLE))
  {
    m_filterSettings.filterEffectsSettings.zoomMidPoint = m_screenMidPoint;
    return;
    }
    if ((m_filterMode == ZoomFilterMode::WAVE_MODE1) &&
        m_filterEvents->Happens(FilterEventTypes::WAVE_IN_MIDDLE))
    {
      m_filterSettings.filterEffectsSettings.zoomMidPoint = m_screenMidPoint;
      return;
    }

    switch (m_zoomMidPointWeights.GetRandomWeighted())
    {
      case ZoomMidPointEvents::EVENT1:
        m_filterSettings.filterEffectsSettings.zoomMidPoint = {
            m_goomInfo.GetScreenInfo().width / 2, m_goomInfo.GetScreenInfo().height - 1};
        break;
      case ZoomMidPointEvents::EVENT2:
        m_filterSettings.filterEffectsSettings.zoomMidPoint.x =
            static_cast<int32_t>(m_goomInfo.GetScreenInfo().width - 1);
        break;
      case ZoomMidPointEvents::EVENT3:
        m_filterSettings.filterEffectsSettings.zoomMidPoint.x = 1;
        break;
      case ZoomMidPointEvents::EVENT4:
        m_filterSettings.filterEffectsSettings.zoomMidPoint = m_screenMidPoint;
        break;
      case ZoomMidPointEvents::EVENT5:
        m_filterSettings.filterEffectsSettings.zoomMidPoint = {
            m_goomInfo.GetScreenInfo().width / 4, m_goomInfo.GetScreenInfo().height / 4};
        break;
      case ZoomMidPointEvents::EVENT6:
        m_filterSettings.filterEffectsSettings.zoomMidPoint = {
            (3 * m_goomInfo.GetScreenInfo().width) / 4,
            (3 * m_goomInfo.GetScreenInfo().height) / 4};
        break;
      default:
        throw std::logic_error("Unknown ZoomMidPointEvents enum.");
    }
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::DRAW
#endif
