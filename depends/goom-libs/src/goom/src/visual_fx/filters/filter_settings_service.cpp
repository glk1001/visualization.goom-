#include "filter_settings_service.h"

#include "extra_effects.h"
#include "filter_settings.h"
#include "goom_plugin_info.h"
#include "utils/enumutils.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#include <stdexcept>

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::NUM;
using UTILS::MATH::I_HALF;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::U_HALF;
using UTILS::MATH::U_QUARTER;
using UTILS::MATH::U_THREE_QUARTERS;
using UTILS::MATH::Weights;

template<class E, size_t N>
inline auto ToMap(const std::array<std::pair<E, float>, N>& stdArray) -> std::map<E, float>
{
  std::map<E, float> map{};
  for (const auto& element : stdArray)
  {
    map.emplace(element.first, element.second);
  }
  return map;
}

// For debugging:
static constexpr bool USE_FORCED_FILTER_MODE = false;

//static constexpr ZoomFilterMode FORCED_FILTER_MODE = ZoomFilterMode::AMULET_MODE;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE = ZoomFilterMode::CRYSTAL_BALL_MODE0;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::CRYSTAL_BALL_MODE1;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::DISTANCE_FIELD_MODE;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::HYPERCOS_MODE0;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::HYPERCOS_MODE1;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::HYPERCOS_MODE2;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::HYPERCOS_MODE3;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE = ZoomFilterMode::IMAGE_DISPLACEMENT_MODE;
static constexpr ZoomFilterMode FORCED_FILTER_MODE = ZoomFilterMode::NORMAL_MODE;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::SCRUNCH_MODE;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::SPEEDWAY_MODE0;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::SPEEDWAY_MODE1;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::SPEEDWAY_MODE2;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE = ZoomFilterMode::WAVE_MODE0;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::WAVE_MODE1;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::WATER_MODE;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::Y_ONLY_MODE;
// End debugging


// clang-format off
static constexpr float PROB_HIGH = 0.9F;
static constexpr float PROB_HALF = 0.5F;
static constexpr float PROB_LOW  = 0.1F;
static constexpr float PROB_ZERO = 0.0F;

static constexpr float PROB_CRYSTAL_BALL_IN_MIDDLE = 0.8F;
static constexpr float PROB_WAVE_IN_MIDDLE         = 0.5F;
static constexpr float PROB_CHANGE_SPEED           = 0.5F;
static constexpr float PROB_REVERSE_SPEED          = 0.5F;

static constexpr float AMULET_MODE_WEIGHT             = 10.0F;
static constexpr float CRYSTAL_BALL_MODE0_WEIGHT      =  4.0F;
static constexpr float CRYSTAL_BALL_MODE1_WEIGHT      =  2.0F;
static constexpr float DISTANCE_FIELD_MODE_WEIGHT     =  8.0F;
static constexpr float HYPERCOS_MODE0_WEIGHT          =  8.0F;
static constexpr float HYPERCOS_MODE1_WEIGHT          =  4.0F;
static constexpr float HYPERCOS_MODE2_WEIGHT          =  1.0F;
static constexpr float HYPERCOS_MODE3_WEIGHT          =  1.0F;
static constexpr float IMAGE_DISPLACEMENT_MODE_WEIGHT =  5.0F;
static constexpr float NORMAL_MODE_WEIGHT             = 10.0F;
static constexpr float SCRUNCH_MODE_WEIGHT            =  6.0F;
static constexpr float SPEEDWAY_MODE0_WEIGHT          =  3.0F;
static constexpr float SPEEDWAY_MODE1_WEIGHT          =  2.0F;
static constexpr float SPEEDWAY_MODE2_WEIGHT          =  2.0F;
static constexpr float WAVE_MODE0_WEIGHT              =  5.0F;
static constexpr float WAVE_MODE1_WEIGHT              =  4.0F;
static constexpr float WATER_MODE_WEIGHT              =  0.0F;
static constexpr float Y_ONLY_MODE_WEIGHT             =  4.0F;

// TODO - When we get to use C++20 we can simplify things with
//        constexpr std::vector.
static constexpr std::array CRYSTAL_BALL_MODE0_MULTIPLIERS {
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::CRYSTAL_BALL_MODE0, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::CRYSTAL_BALL_MODE1, 0.0F},
};
static constexpr std::array CRYSTAL_BALL_MODE1_MULTIPLIERS {
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::CRYSTAL_BALL_MODE0, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::CRYSTAL_BALL_MODE1, 0.0F},
};
static constexpr std::array NORMAL_MODE_MULTIPLIERS {
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::NORMAL_MODE, 1.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::DISTANCE_FIELD_MODE, 2.0F},
};
static constexpr std::array HYPERCOS_MODE0_MULTIPLIERS {
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE0, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
};
static constexpr std::array HYPERCOS_MODE1_MULTIPLIERS {
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
};
static constexpr std::array HYPERCOS_MODE2_MULTIPLIERS {
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
};
static constexpr std::array HYPERCOS_MODE3_MULTIPLIERS {
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
};
static constexpr std::array SPEEDWAY_MODE0_MULTIPLIERS {
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::SPEEDWAY_MODE0, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::SPEEDWAY_MODE1, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::SPEEDWAY_MODE2, 0.0F},
};
static constexpr std::array SPEEDWAY_MODE1_MULTIPLIERS {
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::SPEEDWAY_MODE0, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::SPEEDWAY_MODE1, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::SPEEDWAY_MODE2, 0.0F},
};
static constexpr std::array SPEEDWAY_MODE2_MULTIPLIERS {
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::SPEEDWAY_MODE0, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::SPEEDWAY_MODE1, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::SPEEDWAY_MODE2, 0.0F},
};
static constexpr std::array WAVE_MODE0_MULTIPLIERS {
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::WAVE_MODE0, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::WAVE_MODE1, 0.0F},
};
static constexpr std::array WAVE_MODE1_MULTIPLIERS {
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::WAVE_MODE0, 0.0F},
  std::pair<ZoomFilterMode, float>{ZoomFilterMode::WAVE_MODE1, 0.0F},
};

static constexpr float AMULET_ROTATE_PROB      = PROB_HIGH;
static constexpr float CRYS_BALL0_ROTATE_PROB  = PROB_HIGH;
static constexpr float CRYS_BALL1_ROTATE_PROB  = PROB_HIGH;
static constexpr float DIST_FIELD_ROTATE_PROB  = PROB_HIGH;
static constexpr float HYPERCOS0_ROTATE_PROB   = PROB_LOW;
static constexpr float HYPERCOS1_ROTATE_PROB   = PROB_LOW;
static constexpr float HYPERCOS2_ROTATE_PROB   = PROB_LOW;
static constexpr float HYPERCOS3_ROTATE_PROB   = PROB_LOW;
static constexpr float IMAGE_DISPL_ROTATE_PROB = PROB_ZERO;
static constexpr float NORMAL_ROTATE_PROB      = PROB_ZERO;
static constexpr float SCRUNCH_ROTATE_PROB     = PROB_HALF;
static constexpr float SPEEDWAY0_ROTATE_PROB   = PROB_HALF;
static constexpr float SPEEDWAY1_ROTATE_PROB   = PROB_HIGH;
static constexpr float SPEEDWAY2_ROTATE_PROB   = PROB_HIGH;
static constexpr float WATER_ROTATE_PROB       = PROB_ZERO;
static constexpr float WAVE0_ROTATE_PROB       = PROB_HIGH;
static constexpr float WAVE1_ROTATE_PROB       = PROB_HIGH;
static constexpr float Y_ONLY_ROTATE_PROB      = PROB_HALF;

using Hyp = HypercosOverlay;
using ModeWeights = std::vector<std::pair<Hyp, float>>;

// TODO - Use 'constexpr' when we get to use C++20.
static const ModeWeights AMULET_WEIGHTS      = {{ {Hyp::NONE, 20.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }};
static const ModeWeights CRYS_BALL0_WEIGHTS  = {{ {Hyp::NONE,  5.0F}, {Hyp::MODE0, 10.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }};
static const ModeWeights CRYS_BALL1_WEIGHTS  = {{ {Hyp::NONE,  5.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1, 99.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }};
static const ModeWeights DIST_FIELD_WEIGHTS  = {{ {Hyp::NONE,  5.0F}, {Hyp::MODE0, 10.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }};
static const ModeWeights HYPERCOS0_WEIGHTS   = {{ {Hyp::NONE,  0.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  0.0F}, {Hyp::MODE2,  0.0F}, {Hyp::MODE3,  0.0F} }};
static const ModeWeights HYPERCOS1_WEIGHTS   = {{ {Hyp::NONE,  0.0F}, {Hyp::MODE0,  0.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  0.0F}, {Hyp::MODE3,  0.0F} }};
static const ModeWeights HYPERCOS2_WEIGHTS   = {{ {Hyp::NONE,  0.0F}, {Hyp::MODE0,  0.0F}, {Hyp::MODE1,  0.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  0.0F} }};
static const ModeWeights HYPERCOS3_WEIGHTS   = {{ {Hyp::NONE,  0.0F}, {Hyp::MODE0,  0.0F}, {Hyp::MODE1,  0.0F}, {Hyp::MODE2,  0.0F}, {Hyp::MODE3,  1.0F} }};
static const ModeWeights IMAGE_DISPL_WEIGHTS = {{ {Hyp::NONE, 99.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }};
static const ModeWeights NORMAL_WEIGHTS      = {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  0.0F} }};
static const ModeWeights SCRUNCH_WEIGHTS     = {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }};
static const ModeWeights SPEEDWAY0_WEIGHTS   = {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }};
static const ModeWeights SPEEDWAY1_WEIGHTS   = {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }};
static const ModeWeights SPEEDWAY2_WEIGHTS   = {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }};
static const ModeWeights WATER_WEIGHTS       = {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }};
static const ModeWeights WAVE0_WEIGHTS       = {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }};
static const ModeWeights WAVE1_WEIGHTS       = {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }};
static const ModeWeights Y_ONLY_WEIGHTS      = {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }};

static constexpr float BOTTOM_MID_POINT_WEIGHT               =  3.0F;
static constexpr float RIGHT_MID_POINT_WEIGHT                =  2.0F;
static constexpr float LEFT_MID_POINT_WEIGHT                 =  2.0F;
static constexpr float CENTRE_MID_POINT_WEIGHT               = 18.0F;
static constexpr float TOP_LEFT_QUARTER_MID_POINT_WEIGHT     = 10.0F;
static constexpr float BOTTOM_RIGHT_QUARTER_MID_POINT_WEIGHT = 10.0F;
// clang-format on

auto FilterSettingsService::GetFilterModeData(
    const IGoomRand& goomRand,
    const std::string& resourcesDirectory,
    const CreateSpeedCoefficientsEffectFunc& createSpeedCoefficientsEffect)
    -> std::map<ZoomFilterMode, ZoomFilterModeInfo>
{
  // clang-format off
  // TODO - Use 'constexpr' when we get to use C++20.
  static const std::array<FilterModeData, NUM<ZoomFilterMode>> s_ZOOM_FILTER_DATA = {{
      { ZoomFilterMode::AMULET_MODE, "Amulet", AMULET_ROTATE_PROB, AMULET_WEIGHTS },
      { ZoomFilterMode::CRYSTAL_BALL_MODE0, "Crystal Ball Mode 0", CRYS_BALL0_ROTATE_PROB, CRYS_BALL0_WEIGHTS },
      { ZoomFilterMode::CRYSTAL_BALL_MODE1, "Crystal Ball Mode 1", CRYS_BALL1_ROTATE_PROB, CRYS_BALL1_WEIGHTS },
      { ZoomFilterMode::DISTANCE_FIELD_MODE, "Distance Field", DIST_FIELD_ROTATE_PROB, DIST_FIELD_WEIGHTS },
      { ZoomFilterMode::HYPERCOS_MODE0, "Hypercos Mode 0", HYPERCOS0_ROTATE_PROB, HYPERCOS0_WEIGHTS},
      { ZoomFilterMode::HYPERCOS_MODE1, "Hypercos Mode 1", HYPERCOS1_ROTATE_PROB, HYPERCOS1_WEIGHTS},
      { ZoomFilterMode::HYPERCOS_MODE2, "Hypercos Mode 2", HYPERCOS2_ROTATE_PROB, HYPERCOS2_WEIGHTS},
      { ZoomFilterMode::HYPERCOS_MODE3, "Hypercos Mode 3", HYPERCOS3_ROTATE_PROB, HYPERCOS3_WEIGHTS},
      { ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, "Image Displacement", IMAGE_DISPL_ROTATE_PROB, IMAGE_DISPL_WEIGHTS},
      { ZoomFilterMode::NORMAL_MODE, "Normal", NORMAL_ROTATE_PROB, NORMAL_WEIGHTS},
      { ZoomFilterMode::SCRUNCH_MODE, "Scrunch", SCRUNCH_ROTATE_PROB, SCRUNCH_WEIGHTS},
      { ZoomFilterMode::SPEEDWAY_MODE0, "Speedway Mode 0", SPEEDWAY0_ROTATE_PROB, SPEEDWAY0_WEIGHTS},
      { ZoomFilterMode::SPEEDWAY_MODE1, "Speedway Mode 1", SPEEDWAY1_ROTATE_PROB, SPEEDWAY1_WEIGHTS},
      { ZoomFilterMode::SPEEDWAY_MODE2, "Speedway Mode 2", SPEEDWAY2_ROTATE_PROB, SPEEDWAY2_WEIGHTS},
      { ZoomFilterMode::WATER_MODE, "Water", WATER_ROTATE_PROB, WATER_WEIGHTS},
      { ZoomFilterMode::WAVE_MODE0, "Wave Mode 0", WAVE0_ROTATE_PROB, WAVE0_WEIGHTS},
      { ZoomFilterMode::WAVE_MODE1, "Wave Mode 1", WAVE1_ROTATE_PROB, WAVE1_WEIGHTS},
      { ZoomFilterMode::Y_ONLY_MODE, "Y Only", Y_ONLY_ROTATE_PROB, Y_ONLY_WEIGHTS},
  }};
  // clang-format on

  std::map<ZoomFilterMode, ZoomFilterModeInfo> filterMap{};

  for (const auto& filterModeData : s_ZOOM_FILTER_DATA)
  {
    filterMap.try_emplace(
        filterModeData.filterMode,
        ZoomFilterModeInfo{
            std::string(filterModeData.name),
            createSpeedCoefficientsEffect(filterModeData.filterMode, goomRand, resourcesDirectory),
            filterModeData.rotateProb,
            Weights<Hyp>{goomRand, filterModeData.modeWeights},
    });
  }

  return filterMap;
}

FilterSettingsService::FilterSettingsService(const PluginInfo& goomInfo,
                                             const IGoomRand& goomRand,
                                             const std::string& resourcesDirectory,
                                             const CreateSpeedCoefficientsEffectFunc&
                                                 createSpeedCoefficientsEffect)
  : m_goomInfo{goomInfo},
    m_goomRand{goomRand},
    m_screenMidpoint{U_HALF * m_goomInfo.GetScreenInfo().width,
                     U_HALF * m_goomInfo.GetScreenInfo().height},
    m_resourcesDirectory{resourcesDirectory},
    m_extraEffects{std::make_unique<ExtraEffects>(m_goomRand)},
    m_filterModeData{GetFilterModeData(m_goomRand,
                                       m_resourcesDirectory,
                                       createSpeedCoefficientsEffect)},
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
    },
    // clang-format off
    m_weightedFilterEvents{
        m_goomRand,
        {
            {ZoomFilterMode::AMULET_MODE,             AMULET_MODE_WEIGHT},
            {ZoomFilterMode::CRYSTAL_BALL_MODE0,      CRYSTAL_BALL_MODE0_WEIGHT},
            {ZoomFilterMode::CRYSTAL_BALL_MODE1,      CRYSTAL_BALL_MODE1_WEIGHT},
            {ZoomFilterMode::DISTANCE_FIELD_MODE,     DISTANCE_FIELD_MODE_WEIGHT},
            {ZoomFilterMode::HYPERCOS_MODE0,          HYPERCOS_MODE0_WEIGHT},
            {ZoomFilterMode::HYPERCOS_MODE1,          HYPERCOS_MODE1_WEIGHT},
            {ZoomFilterMode::HYPERCOS_MODE2,          HYPERCOS_MODE2_WEIGHT},
            {ZoomFilterMode::HYPERCOS_MODE3,          HYPERCOS_MODE3_WEIGHT},
            {ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, IMAGE_DISPLACEMENT_MODE_WEIGHT},
            {ZoomFilterMode::NORMAL_MODE,             NORMAL_MODE_WEIGHT},
            {ZoomFilterMode::SCRUNCH_MODE,            SCRUNCH_MODE_WEIGHT},
            {ZoomFilterMode::SPEEDWAY_MODE0,          SPEEDWAY_MODE0_WEIGHT},
            {ZoomFilterMode::SPEEDWAY_MODE1,          SPEEDWAY_MODE1_WEIGHT},
            {ZoomFilterMode::SPEEDWAY_MODE2,          SPEEDWAY_MODE2_WEIGHT},
            {ZoomFilterMode::WAVE_MODE0,              WAVE_MODE0_WEIGHT},
            {ZoomFilterMode::WAVE_MODE1,              WAVE_MODE1_WEIGHT},
            {ZoomFilterMode::WATER_MODE,              WATER_MODE_WEIGHT},
            {ZoomFilterMode::Y_ONLY_MODE,             Y_ONLY_MODE_WEIGHT},
        },
        {
            {ZoomFilterMode::CRYSTAL_BALL_MODE0, ToMap(CRYSTAL_BALL_MODE0_MULTIPLIERS)},
            {ZoomFilterMode::CRYSTAL_BALL_MODE1, ToMap(CRYSTAL_BALL_MODE1_MULTIPLIERS)},
            {ZoomFilterMode::NORMAL_MODE,        ToMap(NORMAL_MODE_MULTIPLIERS)},
            {ZoomFilterMode::HYPERCOS_MODE0,     ToMap(HYPERCOS_MODE0_MULTIPLIERS)},
            {ZoomFilterMode::HYPERCOS_MODE1,     ToMap(HYPERCOS_MODE1_MULTIPLIERS)},
            {ZoomFilterMode::HYPERCOS_MODE2,     ToMap(HYPERCOS_MODE2_MULTIPLIERS)},
            {ZoomFilterMode::HYPERCOS_MODE3,     ToMap(HYPERCOS_MODE3_MULTIPLIERS)},
            {ZoomFilterMode::SPEEDWAY_MODE0,     ToMap(SPEEDWAY_MODE0_MULTIPLIERS)},
            {ZoomFilterMode::SPEEDWAY_MODE1,     ToMap(SPEEDWAY_MODE1_MULTIPLIERS)},
            {ZoomFilterMode::SPEEDWAY_MODE2,     ToMap(SPEEDWAY_MODE2_MULTIPLIERS)},
            {ZoomFilterMode::WAVE_MODE0,         ToMap(WAVE_MODE0_MULTIPLIERS)},
            {ZoomFilterMode::WAVE_MODE1,         ToMap(WAVE_MODE1_MULTIPLIERS)},
        },
    },
    m_zoomMidpointWeights{
        m_goomRand,
        {
            {ZoomMidpointEvents::BOTTOM_MID_POINT,               BOTTOM_MID_POINT_WEIGHT},
            {ZoomMidpointEvents::RIGHT_MID_POINT,                RIGHT_MID_POINT_WEIGHT},
            {ZoomMidpointEvents::LEFT_MID_POINT,                 LEFT_MID_POINT_WEIGHT},
            {ZoomMidpointEvents::CENTRE_MID_POINT,               CENTRE_MID_POINT_WEIGHT},
            {ZoomMidpointEvents::TOP_LEFT_QUARTER_MID_POINT,     TOP_LEFT_QUARTER_MID_POINT_WEIGHT},
            {ZoomMidpointEvents::BOTTOM_RIGHT_QUARTER_MID_POINT, BOTTOM_RIGHT_QUARTER_MID_POINT_WEIGHT},
        }
    }
// clang-format on
{
}

FilterSettingsService::~FilterSettingsService() noexcept = default;

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
  if constexpr (USE_FORCED_FILTER_MODE)
  {
    return FORCED_FILTER_MODE;
  }
  return m_weightedFilterEvents.GetRandomWeighted(m_filterMode);
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

void FilterSettingsService::NewCycle()
{
  m_extraEffects->UpdateTimers();
}

void FilterSettingsService::NotifyUpdatedFilterEffectsSettings()
{
  m_filterEffectsSettingsHaveChanged = false;

  m_filterModeAtLastUpdate = m_filterMode;
  m_extraEffects->EffectsUpdatesActivated();
}

void FilterSettingsService::SetRandomExtraEffects()
{
  SetExtraEffects();
  m_filterEffectsSettingsHaveChanged = true;
}

void FilterSettingsService::SetRandomSettingsForNewFilterMode()
{
  SetDefaultSettings();
  SetExtraEffects();
  SetRandomZoomMidpoint();
  SetFilterModeExtraEffects();
  UpdateFilterSettings();
}

inline void FilterSettingsService::UpdateFilterSettings()
{
  m_filterEffectsSettingsHaveChanged = true;
  m_extraEffects->UpdateFilterSettings(m_filterSettings);
}

void FilterSettingsService::SetDefaultSettings()
{
  m_filterSettings.filterEffectsSettings.speedCoefficientsEffect = GetSpeedCoefficientsEffect();
  m_filterSettings.filterEffectsSettings.zoomMidpoint = m_screenMidpoint;
  m_filterSettings.filterEffectsSettings.vitesse.SetDefault();

  m_extraEffects->SetDefaults();
}

inline void FilterSettingsService::SetExtraEffects()
{
  m_extraEffects->UpdateEffects();
}

inline void FilterSettingsService::SetFilterModeExtraEffects()
{
  const ZoomFilterModeInfo& modeInfo = m_filterModeData.at(m_filterMode);

  m_extraEffects->SetHypercosOverlayEffect(modeInfo.hypercosWeights.GetRandomWeighted());
  m_extraEffects->SetRotate(modeInfo.rotateProbability);

  if ((m_filterMode == ZoomFilterMode::WAVE_MODE0) || (m_filterMode == ZoomFilterMode::WAVE_MODE1))
  {
    SetWaveModeExtraEffects();
  }
}

void FilterSettingsService::SetWaveModeExtraEffects()
{
  m_extraEffects->TurnPlaneEffectOn();

  m_filterSettings.filterEffectsSettings.vitesse.SetReverseVitesse(
      m_goomRand.ProbabilityOf(PROB_REVERSE_SPEED));

  if (m_goomRand.ProbabilityOf(PROB_CHANGE_SPEED))
  {
    m_filterSettings.filterEffectsSettings.vitesse.SetVitesse(
        I_HALF *
        (m_filterSettings.filterEffectsSettings.vitesse.GetVitesse() + Vitesse::DEFAULT_VITESSE));
  }
}

void FilterSettingsService::SetMaxSpeedCoeff()
{
  static constexpr float MIN_SPEED_FACTOR = 0.5F;
  static constexpr float MAX_SPEED_FACTOR = 1.0F;
  m_filterSettings.filterEffectsSettings.maxSpeedCoeff =
      m_goomRand.GetRandInRange(MIN_SPEED_FACTOR, MAX_SPEED_FACTOR) * MAX_MAX_SPEED_COEFF;
}

void FilterSettingsService::SetRandomZoomMidpoint()
{
  if (IsZoomMidpointInTheMiddle())
  {
    m_filterSettings.filterEffectsSettings.zoomMidpoint = m_screenMidpoint;
    return;
  }

  SetAnyRandomZoomMidpoint();
}

auto FilterSettingsService::IsZoomMidpointInTheMiddle() const -> bool
{
  if ((m_filterMode == ZoomFilterMode::WATER_MODE) || (m_filterMode == ZoomFilterMode::AMULET_MODE))
  {
    return true;
  }

  if (((m_filterMode == ZoomFilterMode::CRYSTAL_BALL_MODE0) ||
       (m_filterMode == ZoomFilterMode::CRYSTAL_BALL_MODE1)) &&
      m_goomRand.ProbabilityOf(PROB_CRYSTAL_BALL_IN_MIDDLE))
  {
    return true;
  }

  if (((m_filterMode == ZoomFilterMode::WAVE_MODE0) ||
       (m_filterMode == ZoomFilterMode::WAVE_MODE1)) &&
      m_goomRand.ProbabilityOf(PROB_WAVE_IN_MIDDLE))
  {
    return true;
  }

  return false;
}

void FilterSettingsService::SetAnyRandomZoomMidpoint()
{
  switch (m_zoomMidpointWeights.GetRandomWeighted())
  {
    case ZoomMidpointEvents::BOTTOM_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          U_HALF * m_goomInfo.GetScreenInfo().width, m_goomInfo.GetScreenInfo().height - 1};
      break;
    case ZoomMidpointEvents::RIGHT_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint.x =
          static_cast<int32_t>(m_goomInfo.GetScreenInfo().width - 1);
      break;
    case ZoomMidpointEvents::LEFT_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint.x = 1;
      break;
    case ZoomMidpointEvents::CENTRE_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = m_screenMidpoint;
      break;
    case ZoomMidpointEvents::TOP_LEFT_QUARTER_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          U_QUARTER * m_goomInfo.GetScreenInfo().width,
          U_QUARTER * m_goomInfo.GetScreenInfo().height};
      break;
    case ZoomMidpointEvents::BOTTOM_RIGHT_QUARTER_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          U_THREE_QUARTERS * m_goomInfo.GetScreenInfo().width,
          U_THREE_QUARTERS * m_goomInfo.GetScreenInfo().height};
      break;
    default:
      throw std::logic_error("Unknown ZoomMidpointEvents enum.");
  }
}

} // namespace GOOM::VISUAL_FX::FILTERS
