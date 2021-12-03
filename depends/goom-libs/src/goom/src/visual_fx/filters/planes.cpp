#include "planes.h"

#include "utils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>

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

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::IGoomRand;
using UTILS::NameValuePairs;
using UTILS::Weights;

// H Plane:
// @since June 2001
static constexpr float DEFAULT_HORIZONTAL_EFFECT_AMPLITUDE = 0.0025F;
constexpr IGoomRand::NumberRange<float> HORIZONTAL_EFFECT_AMPLITUDE_RANGE = {0.0015F, 0.0035F};
// Spirally and interesting
// constexpr NumberRange<float> HORIZONTAL_EFFECT_AMPLITUDE_RANGE = {0.0015F, 0.0351F};

// V Plane:
static constexpr float DEFAULT_VERTICAL_EFFECT_AMPLITUDE = 0.0025F;
constexpr IGoomRand::NumberRange<float> VERTICAL_EFFECT_AMPLITUDE_RANGE = {0.0015F, 0.0035F};
// constexpr NumberRange<float> VERTICAL_EFFECT_AMPLITUDE_RANGE = {0.0015F, 0.0351F};

constexpr float PROB_PLANE_AMPLITUDES_EQUAL = 12.0F / 16.0F;
constexpr float PROB_ZERO_HORIZONTAL_PLANE_EFFECT = 0.5F;

Planes::Planes(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_params{0, DEFAULT_HORIZONTAL_EFFECT_AMPLITUDE, 0, DEFAULT_VERTICAL_EFFECT_AMPLITUDE},
    // clang-format off
    m_planeEffectWeights{
      m_goomRand,
      {
          { PlaneEffectEvents::EVENT1,  1 },
          { PlaneEffectEvents::EVENT2,  1 },
          { PlaneEffectEvents::EVENT3,  4 },
          { PlaneEffectEvents::EVENT4,  1 },
          { PlaneEffectEvents::EVENT5,  1 },
          { PlaneEffectEvents::EVENT6,  1 },
          { PlaneEffectEvents::EVENT7,  1 },
          { PlaneEffectEvents::EVENT8,  2 },
      }
    }
// clang-format on
{
}

void Planes::SetRandomParams(const V2dInt& zoomMidPoint, const uint32_t screenWidth)
{
  switch (m_planeEffectWeights.GetRandomWeighted())
  {
    case PlaneEffectEvents::EVENT1:
      m_params.verticalEffect = m_goomRand.GetRandInRange(-2, +3);
      m_params.horizontalEffect = m_goomRand.GetRandInRange(-2, +3);
      break;
    case PlaneEffectEvents::EVENT2:
      m_params.verticalEffect = 0;
      m_params.horizontalEffect = m_goomRand.GetRandInRange(-7, +8);
      break;
    case PlaneEffectEvents::EVENT3:
      m_params.verticalEffect = m_goomRand.GetRandInRange(-5, +6);
      m_params.horizontalEffect = -m_params.verticalEffect + 1;
      break;
    case PlaneEffectEvents::EVENT4:
      m_params.horizontalEffect = static_cast<int>(m_goomRand.GetRandInRange(5U, 13U));
      m_params.verticalEffect = -m_params.horizontalEffect + 1;
      break;
    case PlaneEffectEvents::EVENT5:
      m_params.verticalEffect = static_cast<int>(m_goomRand.GetRandInRange(5U, 13U));
      m_params.horizontalEffect = -m_params.horizontalEffect + 1;
      break;
    case PlaneEffectEvents::EVENT6:
      m_params.horizontalEffect = 0;
      m_params.verticalEffect = m_goomRand.GetRandInRange(-9, +10);
      break;
    case PlaneEffectEvents::EVENT7:
      m_params.horizontalEffect = m_goomRand.GetRandInRange(-9, +10);
      m_params.verticalEffect = m_goomRand.GetRandInRange(-9, +10);
      break;
    case PlaneEffectEvents::EVENT8:
      m_params.verticalEffect = 0;
      m_params.horizontalEffect = 0;
      break;
    default:
      throw std::logic_error("Unknown MiddlePointEvents enum.");
  }

  m_params.horizontalEffectAmplitude = m_goomRand.GetRandInRange(HORIZONTAL_EFFECT_AMPLITUDE_RANGE);
  if (m_goomRand.ProbabilityOf(PROB_PLANE_AMPLITUDES_EQUAL))
  {
    m_params.verticalEffectAmplitude = m_params.horizontalEffectAmplitude;
  }
  else
  {
    m_params.verticalEffectAmplitude = m_goomRand.GetRandInRange(VERTICAL_EFFECT_AMPLITUDE_RANGE);
  }

  if ((1 == zoomMidPoint.x) || (zoomMidPoint.x == static_cast<int32_t>(screenWidth - 1)))
  {
    m_params.verticalEffect = 0;
    if (m_goomRand.ProbabilityOf(PROB_ZERO_HORIZONTAL_PLANE_EFFECT))
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
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::FILTERS
#endif
