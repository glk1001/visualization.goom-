#include "filter_settings_service.h"

#include "extra_effects_states.h"
#include "filter_consts.h"
#include "filter_settings.h"
#include "goom_config.h"
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


// For debugging:
static constexpr bool USE_FORCED_FILTER_MODE = ALL_FILTER_EFFECTS_TURNED_OFF;

//static constexpr ZoomFilterMode FORCED_FILTER_MODE = ZoomFilterMode::AMULET_MODE;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE = ZoomFilterMode::CRYSTAL_BALL_MODE0;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::CRYSTAL_BALL_MODE1;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE ZoomFilterMode::DISTANCE_FIELD_MODE;
//static constexpr ZoomFilterMode FORCED_FILTER_MODE = ZoomFilterMode::HYPERCOS_MODE0;
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

static constexpr float BOTTOM_MID_POINT_WEIGHT               =  3.0F;
static constexpr float RIGHT_MID_POINT_WEIGHT                =  2.0F;
static constexpr float LEFT_MID_POINT_WEIGHT                 =  2.0F;
static constexpr float CENTRE_MID_POINT_WEIGHT               = 18.0F;
static constexpr float TOP_LEFT_QUARTER_MID_POINT_WEIGHT     = 10.0F;
static constexpr float BOTTOM_RIGHT_QUARTER_MID_POINT_WEIGHT = 10.0F;
// clang-format on

// TODO - When we get to use C++20, then replace the below 'inline consts' with 'static constexpr'.

// clang-format off
inline const std::map<ZoomFilterMode, float> CRYSTAL_BALL_MODE0_MULTIPLIERS{
  {ZoomFilterMode::CRYSTAL_BALL_MODE0, 0.0F},
  {ZoomFilterMode::CRYSTAL_BALL_MODE1, 0.0F},
};
inline const std::map<ZoomFilterMode, float> CRYSTAL_BALL_MODE1_MULTIPLIERS {
  {ZoomFilterMode::CRYSTAL_BALL_MODE0, 0.0F},
  {ZoomFilterMode::CRYSTAL_BALL_MODE1, 0.0F},
};
inline const std::map<ZoomFilterMode, float> NORMAL_MODE_MULTIPLIERS {
  {ZoomFilterMode::NORMAL_MODE, 1.0F},
  {ZoomFilterMode::DISTANCE_FIELD_MODE, 2.0F},
};
inline const std::map<ZoomFilterMode, float> HYPERCOS_MODE0_MULTIPLIERS {
  {ZoomFilterMode::HYPERCOS_MODE0, 0.0F},
  {ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
  {ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
  {ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
};
inline const std::map<ZoomFilterMode, float> HYPERCOS_MODE1_MULTIPLIERS {
  {ZoomFilterMode::HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
  {ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
  {ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
  {ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
};
inline const std::map<ZoomFilterMode, float> HYPERCOS_MODE2_MULTIPLIERS {
  {ZoomFilterMode::HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
  {ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
  {ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
  {ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
};
inline const std::map<ZoomFilterMode, float> HYPERCOS_MODE3_MULTIPLIERS {
  {ZoomFilterMode::HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
  {ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
  {ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
  {ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
};
inline const std::map<ZoomFilterMode, float> SPEEDWAY_MODE0_MULTIPLIERS {
  {ZoomFilterMode::SPEEDWAY_MODE0, 0.0F},
  {ZoomFilterMode::SPEEDWAY_MODE1, 0.0F},
  {ZoomFilterMode::SPEEDWAY_MODE2, 0.0F},
};
inline const std::map<ZoomFilterMode, float> SPEEDWAY_MODE1_MULTIPLIERS {
  {ZoomFilterMode::SPEEDWAY_MODE0, 0.0F},
  {ZoomFilterMode::SPEEDWAY_MODE1, 0.0F},
  {ZoomFilterMode::SPEEDWAY_MODE2, 0.0F},
};
inline const std::map<ZoomFilterMode, float> SPEEDWAY_MODE2_MULTIPLIERS {
  {ZoomFilterMode::SPEEDWAY_MODE0, 0.0F},
  {ZoomFilterMode::SPEEDWAY_MODE1, 0.0F},
  {ZoomFilterMode::SPEEDWAY_MODE2, 0.0F},
};
inline const std::map<ZoomFilterMode, float> WAVE_MODE0_MULTIPLIERS {
  {ZoomFilterMode::WAVE_MODE0, 0.0F},
  {ZoomFilterMode::WAVE_MODE1, 0.0F},
};
inline const std::map<ZoomFilterMode, float> WAVE_MODE1_MULTIPLIERS {
  {ZoomFilterMode::WAVE_MODE0, 0.0F},
  {ZoomFilterMode::WAVE_MODE1, 0.0F},
};
// clang-format on

// clang-format off
inline const std::map<ZoomFilterMode, std::string_view> FILTER_MODE_NAMES{
  { ZoomFilterMode::AMULET_MODE,             "Amulet"},
  { ZoomFilterMode::CRYSTAL_BALL_MODE0,      "Crystal Ball Mode 0"},
  { ZoomFilterMode::CRYSTAL_BALL_MODE1,      "Crystal Ball Mode 1"},
  { ZoomFilterMode::DISTANCE_FIELD_MODE,     "Distance Field" },
  { ZoomFilterMode::HYPERCOS_MODE0,          "Hypercos Mode 0"},
  { ZoomFilterMode::HYPERCOS_MODE1,          "Hypercos Mode 1"},
  { ZoomFilterMode::HYPERCOS_MODE2,          "Hypercos Mode 2"},
  { ZoomFilterMode::HYPERCOS_MODE3,          "Hypercos Mode 3"},
  { ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, "Image Displacement"},
  { ZoomFilterMode::NORMAL_MODE,             "Normal"},
  { ZoomFilterMode::SCRUNCH_MODE,            "Scrunch"},
  { ZoomFilterMode::SPEEDWAY_MODE0,          "Speedway Mode 0"},
  { ZoomFilterMode::SPEEDWAY_MODE1,          "Speedway Mode 1"},
  { ZoomFilterMode::SPEEDWAY_MODE2,          "Speedway Mode 2"},
  { ZoomFilterMode::WATER_MODE,              "Water"},
  { ZoomFilterMode::WAVE_MODE0,              "Wave Mode 0"},
  { ZoomFilterMode::WAVE_MODE1,              "Wave Mode 1"},
  { ZoomFilterMode::Y_ONLY_MODE,             "Y Only"},
};

static constexpr float AMULET_PROB_ROTATE             = PROB_HIGH;
static constexpr float CRYSTAL_BALL0_PROB_ROTATE      = PROB_HIGH;
static constexpr float CRYSTAL_BALL1_PROB_ROTATE      = PROB_HIGH;
static constexpr float DISTANCE_FIELD_PROB_ROTATE     = PROB_HIGH;
static constexpr float HYPERCOS0_PROB_ROTATE          = PROB_LOW;
static constexpr float HYPERCOS1_PROB_ROTATE          = PROB_LOW;
static constexpr float HYPERCOS2_PROB_ROTATE          = PROB_LOW;
static constexpr float HYPERCOS3_PROB_ROTATE          = PROB_LOW;
static constexpr float IMAGE_DISPLACEMENT_PROB_ROTATE = PROB_ZERO;
static constexpr float NORMAL_PROB_ROTATE             = PROB_ZERO;
static constexpr float SCRUNCH_PROB_ROTATE            = PROB_HALF;
static constexpr float SPEEDWAY0_PROB_ROTATE          = PROB_HALF;
static constexpr float SPEEDWAY1_PROB_ROTATE          = PROB_HIGH;
static constexpr float SPEEDWAY2_PROB_ROTATE          = PROB_HIGH;
static constexpr float WATER_PROB_ROTATE              = PROB_ZERO;
static constexpr float WAVE0_PROB_ROTATE              = PROB_HIGH;
static constexpr float WAVE1_PROB_ROTATE              = PROB_HIGH;
static constexpr float Y_ONLY_PROB_ROTATE             = PROB_HALF;

static constexpr float DEFAULT_PROB_BLOCKY_WAVY_EFFECT    = 0.3F;
static constexpr float DEFAULT_PROB_IMAGE_VELOCITY_EFFECT = 0.1F;
static constexpr float DEFAULT_PROB_NOISE_EFFECT          = 0.1F;
static constexpr float DEFAULT_PROB_PLANE_EFFECT          = 0.8F;
static constexpr float DEFAULT_PROB_TAN_EFFECT            = 0.2F;

static constexpr float WAVE0_PROB_PLANE_EFFECT = 1.0F;
static constexpr float WAVE1_PROB_PLANE_EFFECT = 1.0F;

inline const std::map<ZoomFilterMode, ExtraEffectsProbabilities> EFFECTS_PROBABILITIES{
  { ZoomFilterMode::AMULET_MODE, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      AMULET_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::CRYSTAL_BALL_MODE0, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      CRYSTAL_BALL0_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::CRYSTAL_BALL_MODE1, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      CRYSTAL_BALL1_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::DISTANCE_FIELD_MODE, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      DISTANCE_FIELD_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::HYPERCOS_MODE0, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      HYPERCOS0_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::HYPERCOS_MODE1, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      HYPERCOS1_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::HYPERCOS_MODE2, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      HYPERCOS2_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::HYPERCOS_MODE3, {
    DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
    DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
    DEFAULT_PROB_NOISE_EFFECT,
    DEFAULT_PROB_PLANE_EFFECT,
    HYPERCOS3_PROB_ROTATE,
    DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      IMAGE_DISPLACEMENT_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::NORMAL_MODE, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      NORMAL_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::SCRUNCH_MODE, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      SCRUNCH_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::SPEEDWAY_MODE0, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      SPEEDWAY0_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::SPEEDWAY_MODE1, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      SPEEDWAY1_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::SPEEDWAY_MODE2, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      SPEEDWAY2_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::WATER_MODE, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      WATER_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::WAVE_MODE0, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      WAVE0_PROB_PLANE_EFFECT,
      WAVE0_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::WAVE_MODE1, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      WAVE1_PROB_PLANE_EFFECT,
      WAVE1_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
  { ZoomFilterMode::Y_ONLY_MODE, {
      DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
      DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
      DEFAULT_PROB_NOISE_EFFECT,
      DEFAULT_PROB_PLANE_EFFECT,
      Y_ONLY_PROB_ROTATE,
      DEFAULT_PROB_TAN_EFFECT
    }
  },
};

using Hyp = HypercosOverlay;
using ModeWeights = std::vector<std::pair<HypercosOverlay, float>>;

inline const std::map<ZoomFilterMode, ModeWeights> HYPERCOS_WEIGHTS{
  { ZoomFilterMode::AMULET_MODE,
    {{ {Hyp::NONE, 20.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::CRYSTAL_BALL_MODE0,
    {{ {Hyp::NONE,  5.0F}, {Hyp::MODE0, 10.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::CRYSTAL_BALL_MODE1,
    {{ {Hyp::NONE,  5.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1, 99.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::DISTANCE_FIELD_MODE,
    {{ {Hyp::NONE,  5.0F}, {Hyp::MODE0, 10.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::HYPERCOS_MODE0,
    {{ {Hyp::NONE,  0.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  0.0F}, {Hyp::MODE2,  0.0F}, {Hyp::MODE3,  0.0F} }}
  },
  { ZoomFilterMode::HYPERCOS_MODE1,
    {{ {Hyp::NONE,  0.0F}, {Hyp::MODE0,  0.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  0.0F}, {Hyp::MODE3,  0.0F} }}
  },
  { ZoomFilterMode::HYPERCOS_MODE2,
    {{ {Hyp::NONE,  0.0F}, {Hyp::MODE0,  0.0F}, {Hyp::MODE1,  0.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  0.0F} }}
  },
  { ZoomFilterMode::HYPERCOS_MODE3,
    {{ {Hyp::NONE,  0.0F}, {Hyp::MODE0,  0.0F}, {Hyp::MODE1,  0.0F}, {Hyp::MODE2,  0.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::IMAGE_DISPLACEMENT_MODE,
    {{ {Hyp::NONE, 99.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::NORMAL_MODE,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  0.0F} }}
  },
  { ZoomFilterMode::SCRUNCH_MODE,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::SPEEDWAY_MODE0,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::SPEEDWAY_MODE1,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::SPEEDWAY_MODE2,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::WATER_MODE,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::WAVE_MODE0,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::WAVE_MODE1,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::Y_ONLY_MODE,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
};
// clang-format on

auto FilterSettingsService::GetFilterModeData(
    const IGoomRand& goomRand,
    const std::string& resourcesDirectory,
    const CreateSpeedCoefficientsEffectFunc& createSpeedCoefficientsEffect)
    -> std::map<ZoomFilterMode, ZoomFilterModeInfo>
{
  Expects(FILTER_MODE_NAMES.size() == NUM<ZoomFilterMode>);
  Expects(EFFECTS_PROBABILITIES.size() == NUM<ZoomFilterMode>);
  Expects(HYPERCOS_WEIGHTS.size() == NUM<ZoomFilterMode>);

  std::map<ZoomFilterMode, ZoomFilterModeInfo> filterMap{};

  for (size_t i = 0; i < NUM<ZoomFilterMode>; ++i)
  {
    const auto filterMode = static_cast<ZoomFilterMode>(i);

    filterMap.try_emplace(
        filterMode, ZoomFilterModeInfo{
                        FILTER_MODE_NAMES.at(filterMode),
                        createSpeedCoefficientsEffect(filterMode, goomRand, resourcesDirectory),
                        EFFECTS_PROBABILITIES.at(filterMode),
                        Weights<Hyp>{goomRand, HYPERCOS_WEIGHTS.at(filterMode)},
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
    m_randomizedExtraEffects{std::make_unique<ExtraEffectsStates>(m_goomRand)},
    m_filterModeData{GetFilterModeData(m_goomRand,
                                       m_resourcesDirectory,
                                       createSpeedCoefficientsEffect)},
    m_filterSettings{{Vitesse{},
         HypercosOverlay::NONE,
         DEFAULT_MAX_SPEED_COEFF,
         nullptr,
         RotationAdjustments{},
         {DEFAULT_ZOOM_MID_X, DEFAULT_ZOOM_MID_Y},
         false,
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
            {ZoomFilterMode::CRYSTAL_BALL_MODE0, CRYSTAL_BALL_MODE0_MULTIPLIERS},
            {ZoomFilterMode::CRYSTAL_BALL_MODE1, CRYSTAL_BALL_MODE1_MULTIPLIERS},
            {ZoomFilterMode::NORMAL_MODE,        NORMAL_MODE_MULTIPLIERS},
            {ZoomFilterMode::HYPERCOS_MODE0,     HYPERCOS_MODE0_MULTIPLIERS},
            {ZoomFilterMode::HYPERCOS_MODE1,     HYPERCOS_MODE1_MULTIPLIERS},
            {ZoomFilterMode::HYPERCOS_MODE2,     HYPERCOS_MODE2_MULTIPLIERS},
            {ZoomFilterMode::HYPERCOS_MODE3,     HYPERCOS_MODE3_MULTIPLIERS},
            {ZoomFilterMode::SPEEDWAY_MODE0,     SPEEDWAY_MODE0_MULTIPLIERS},
            {ZoomFilterMode::SPEEDWAY_MODE1,     SPEEDWAY_MODE1_MULTIPLIERS},
            {ZoomFilterMode::SPEEDWAY_MODE2,     SPEEDWAY_MODE2_MULTIPLIERS},
            {ZoomFilterMode::WAVE_MODE0,         WAVE_MODE0_MULTIPLIERS},
            {ZoomFilterMode::WAVE_MODE1,         WAVE_MODE1_MULTIPLIERS},
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
  SetNewRandomFilter();
}

inline auto FilterSettingsService::GetSpeedCoefficientsEffect()
    -> std::shared_ptr<ISpeedCoefficientsEffect>&
{
  return m_filterModeData.at(m_filterMode).speedCoefficientsEffect;
}

void FilterSettingsService::NewCycle()
{
  m_randomizedExtraEffects->UpdateTimers();
}

void FilterSettingsService::NotifyUpdatedFilterEffectsSettings()
{
  m_filterEffectsSettingsHaveChanged = false;

  m_filterModeAtLastUpdate = m_filterMode;
  m_randomizedExtraEffects->CheckForPendingOffTimers();
}

void FilterSettingsService::SetRandomSettingsForNewFilterMode()
{
  SetDefaultSettings();
  SetRandomZoomMidpoint();
  SetFilterModeExtraEffects();
  UpdateFilterSettingsFromExtraEffects();
}

void FilterSettingsService::SetDefaultSettings()
{
  m_filterSettings.filterEffectsSettings.speedCoefficientsEffect = GetSpeedCoefficientsEffect();
  m_filterSettings.filterEffectsSettings.zoomMidpoint = m_screenMidpoint;
  m_filterSettings.filterEffectsSettings.vitesse.SetDefault();

  m_randomizedExtraEffects->SetDefaults();
}

inline void FilterSettingsService::SetFilterModeExtraEffects()
{
  SetRandomizedExtraEffects();
  SetWaveModeExtraEffects();
}

void FilterSettingsService::ResetRandomExtraEffects()
{
  const ZoomFilterModeInfo& modeInfo = m_filterModeData.at(m_filterMode);
  m_randomizedExtraEffects->ResetStandardStates(modeInfo.extraEffectsProbabilities);
  m_filterEffectsSettingsHaveChanged = true;
}

inline void FilterSettingsService::SetRandomizedExtraEffects()
{
  const ZoomFilterModeInfo& modeInfo = m_filterModeData.at(m_filterMode);

  m_randomizedExtraEffects->ResetAllStates(modeInfo.hypercosWeights.GetRandomWeighted(),
                                           modeInfo.extraEffectsProbabilities);

  m_filterSettings.filterEffectsSettings.rotationAdjustments.SetMultiplyFactor(
      modeInfo.extraEffectsProbabilities.rotateProbability,
      RotationAdjustments::AdjustmentType::AFTER_RANDOM);
}

void FilterSettingsService::SetWaveModeExtraEffects()
{
  if ((m_filterMode != ZoomFilterMode::WAVE_MODE0) && (m_filterMode != ZoomFilterMode::WAVE_MODE1))
  {
    return;
  }

  m_randomizedExtraEffects->TurnPlaneEffectOn();

  ZoomFilterEffectsSettings& filterEffectsSettings = m_filterSettings.filterEffectsSettings;
  filterEffectsSettings.vitesse.SetReverseVitesse(m_goomRand.ProbabilityOf(PROB_REVERSE_SPEED));
  if (m_goomRand.ProbabilityOf(PROB_CHANGE_SPEED))
  {
    filterEffectsSettings.vitesse.SetVitesse(
        I_HALF * (filterEffectsSettings.vitesse.GetVitesse() + Vitesse::DEFAULT_VITESSE));
  }
}

inline void FilterSettingsService::UpdateFilterSettingsFromExtraEffects()
{
  m_filterEffectsSettingsHaveChanged = true;
  m_randomizedExtraEffects->UpdateFilterSettingsFromStates(m_filterSettings);
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

  const bool allowEdgePoints =
      (m_filterMode != ZoomFilterMode::WAVE_MODE0) && (m_filterMode != ZoomFilterMode::WAVE_MODE1);
  SetAnyRandomZoomMidpoint(allowEdgePoints);
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

auto FilterSettingsService::GetWeightRandomMidPoint(const bool allowEdgePoints) const
    -> ZoomMidpointEvents
{
  ZoomMidpointEvents midPointEvent = m_zoomMidpointWeights.GetRandomWeighted();

  if (allowEdgePoints)
  {
    return midPointEvent;
  }

  while (IsEdgeMidPoint(midPointEvent))
  {
    midPointEvent = m_zoomMidpointWeights.GetRandomWeighted();
  }
  return midPointEvent;
}

inline auto FilterSettingsService::IsEdgeMidPoint(const ZoomMidpointEvents midPointEvent) -> bool
{
  return (midPointEvent == ZoomMidpointEvents::BOTTOM_MID_POINT) ||
         (midPointEvent == ZoomMidpointEvents::RIGHT_MID_POINT) ||
         (midPointEvent == ZoomMidpointEvents::LEFT_MID_POINT);
}

auto FilterSettingsService::SetAnyRandomZoomMidpoint(const bool allowEdgePoints) -> void
{
  switch (GetWeightRandomMidPoint(allowEdgePoints))
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
