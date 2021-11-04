#include "planes.h"

#include "utils/randutils.h"
#include "utils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::GetRandInRange;
using UTILS::NameValuePairs;
using UTILS::NumberRange;
using UTILS::ProbabilityOf;
using UTILS::Weights;

// H Plane:
// @since June 2001
static constexpr float DEFAULT_HORIZONTAL_EFFECT_AMPLITUDE = 0.0025F;
constexpr NumberRange<float> HORIZONTAL_EFFECT_AMPLITUDE_RANGE = {0.0015F, 0.0035F};
// Spirally and interesting
// constexpr NumberRange<float> HORIZONTAL_EFFECT_AMPLITUDE_RANGE = {0.0015F, 0.0351F};

// V Plane:
static constexpr float DEFAULT_VERTICAL_EFFECT_AMPLITUDE = 0.0025F;
constexpr NumberRange<float> VERTICAL_EFFECT_AMPLITUDE_RANGE = {0.0015F, 0.0035F};
// constexpr NumberRange<float> VERTICAL_EFFECT_AMPLITUDE_RANGE = {0.0015F, 0.0351F};

constexpr float PROB_PLANE_AMPLITUDES_EQUAL = 12.0F / 16.0F;
constexpr float PROB_ZERO_HORIZONTAL_PLANE_EFFECT = 0.5F;

Planes::Planes() noexcept
  : m_params{0, DEFAULT_HORIZONTAL_EFFECT_AMPLITUDE, 0, DEFAULT_VERTICAL_EFFECT_AMPLITUDE}
{
}

void Planes::SetRandomParams(const V2dInt& zoomMidPoint, const uint32_t screenWidth)
{
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
      m_params.verticalEffect = GetRandInRange(-2, +3);
      m_params.horizontalEffect = GetRandInRange(-2, +3);
      break;
    case PlaneEffectEvents::EVENT2:
      m_params.verticalEffect = 0;
      m_params.horizontalEffect = GetRandInRange(-7, +8);
      break;
    case PlaneEffectEvents::EVENT3:
      m_params.verticalEffect = GetRandInRange(-5, +6);
      m_params.horizontalEffect = -m_params.verticalEffect + 1;
      break;
    case PlaneEffectEvents::EVENT4:
      m_params.horizontalEffect = static_cast<int>(GetRandInRange(5U, 13U));
      m_params.verticalEffect = -m_params.horizontalEffect + 1;
      break;
    case PlaneEffectEvents::EVENT5:
      m_params.verticalEffect = static_cast<int>(GetRandInRange(5U, 13U));
      m_params.horizontalEffect = -m_params.horizontalEffect + 1;
      break;
    case PlaneEffectEvents::EVENT6:
      m_params.horizontalEffect = 0;
      m_params.verticalEffect = GetRandInRange(-9, +10);
      break;
    case PlaneEffectEvents::EVENT7:
      m_params.horizontalEffect = GetRandInRange(-9, +10);
      m_params.verticalEffect = GetRandInRange(-9, +10);
      break;
    case PlaneEffectEvents::EVENT8:
      m_params.verticalEffect = 0;
      m_params.horizontalEffect = 0;
      break;
    default:
      throw std::logic_error("Unknown MiddlePointEvents enum.");
  }

  m_params.horizontalEffectAmplitude = GetRandInRange(HORIZONTAL_EFFECT_AMPLITUDE_RANGE);
  if (ProbabilityOf(PROB_PLANE_AMPLITUDES_EQUAL))
  {
    m_params.verticalEffectAmplitude = m_params.horizontalEffectAmplitude;
  }
  else
  {
    m_params.verticalEffectAmplitude = GetRandInRange(VERTICAL_EFFECT_AMPLITUDE_RANGE);
  }

  if ((1 == zoomMidPoint.x) || (zoomMidPoint.x == static_cast<int32_t>(screenWidth - 1)))
  {
    m_params.verticalEffect = 0;
    if (ProbabilityOf(PROB_ZERO_HORIZONTAL_PLANE_EFFECT))
    {
      m_params.horizontalEffect = 0;
    }
  }
}

auto Planes::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const std::string fullParamGroup = GetFullParamGroup({paramGroup, "planes"});
  return {
      GetPair(fullParamGroup, "horizontalEffect", m_params.horizontalEffect),
      GetPair(fullParamGroup, "horizontalEffectAmplitude", m_params.horizontalEffectAmplitude),
      GetPair(fullParamGroup, "verticalEffect", m_params.verticalEffect),
      GetPair(fullParamGroup, "verticalEffectAmplitude", m_params.verticalEffectAmplitude),
  };
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
