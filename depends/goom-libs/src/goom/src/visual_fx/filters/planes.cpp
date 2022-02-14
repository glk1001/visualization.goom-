#include "planes.h"

#include "utils/enumutils.h"
#include "utils/goom_rand_base.h"
#include "utils/mathutils.h"
#include "utils/name_value_pairs.h"

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::IGoomRand;
using UTILS::m_pi;
using UTILS::NameValuePairs;
using UTILS::NUM;

// clang-format off
constexpr IGoomRand::NumberRange<int32_t> SMALL_EFFECTS_RANGE               = {-2,  +2 + 1};
constexpr IGoomRand::NumberRange<int32_t> MEDIUM_EFFECTS_RANGE              = {-5,  +5 + 1};
constexpr IGoomRand::NumberRange<int32_t> LARGE_EFFECTS_RANGE               = {-7,  +7 + 1};
constexpr IGoomRand::NumberRange<int32_t> VERY_LARGE_EFFECTS_RANGE          = {-9,  +9 + 1};
constexpr IGoomRand::NumberRange<int32_t> VERY_LARGE_POSITIVE_EFFECTS_RANGE = {+5, +12 + 1};

constexpr float PROB_ZERO_HORIZONTAL_FOR_VERY_LARGE_RANGE = 0.2F;
constexpr float PROB_ZERO_VERTICAL_FOR_LARGE_RANGE        = 0.2F;
constexpr float PROB_HORIZONTAL_OPPOSITE_TO_VERTICAL      = 0.9F;
// clang-format on

// H Plane:
// @since June 2001
// clang-format off
constexpr IGoomRand::NumberRange<float> HORIZONTAL_EFFECTS_MULTIPLIER_RANGE = {0.0015F, 0.0035F};
constexpr IGoomRand::NumberRange<float>
                             HORIZONTAL_EFFECTS_SPIRALLING_MULTIPLIER_RANGE = {0.0015F, 0.0351F};
// clang-format on

constexpr float DEFAULT_HORIZONTAL_SWIRL_FREQ = 1.0F;
constexpr IGoomRand::NumberRange<float> HORIZONTAL_SWIRL_FREQ_RANGE = {0.1F, 30.01F};
constexpr float DEFAULT_HORIZONTAL_SWIRL_AMPLITUDE = 1.0F;
constexpr IGoomRand::NumberRange<float> HORIZONTAL_SWIRL_AMPLITUDE_RANGE = {0.1F, 30.01F};

// V Plane:
// clang-format off
constexpr IGoomRand::NumberRange<float> VERTICAL_EFFECTS_AMPLITUDE_RANGE = {0.0015F, 0.0035F};
constexpr IGoomRand::NumberRange<float>
                             VERTICAL_EFFECTS_SPIRALLING_AMPLITUDE_RANGE = {0.0015F, 0.0351F};
// clang-format on

constexpr float DEFAULT_VERTICAL_SWIRL_FREQ = 1.0F;
constexpr IGoomRand::NumberRange<float> VERTICAL_SWIRL_FREQ_RANGE = {0.1F, 30.01F};
constexpr float DEFAULT_VERTICAL_SWIRL_AMPLITUDE = 1.0F;
constexpr IGoomRand::NumberRange<float> VERTICAL_SWIRL_AMPLITUDE_RANGE = {0.1F, 30.01F};

constexpr float PROB_PLANE_AMPLITUDES_EQUAL = 0.75F;
constexpr float PROB_ZERO_HORIZONTAL_PLANE_EFFECT = 0.5F;
constexpr float PROB_MUCH_SPIRALLING = 0.2F;
constexpr float PROB_NO_SWIRL = 0.95F;
constexpr float PROB_SWIRL_AMPLITUDES_EQUAL = 0.7F;
constexpr float PROB_SWIRL_FREQ_EQUAL = 0.7F;

Planes::Planes(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    // clang-format off
    m_params{
        {
            false, false,
            {0.0F,  0.0F},
        },
        {
            PlaneSwirlType::NONE,
            {
                DEFAULT_HORIZONTAL_SWIRL_FREQ,
                DEFAULT_VERTICAL_SWIRL_FREQ,
            },
            {
                DEFAULT_HORIZONTAL_SWIRL_AMPLITUDE,
                DEFAULT_VERTICAL_SWIRL_AMPLITUDE,
            },
        }
    },
    m_planeEffectWeights{
      m_goomRand,
      {
          { PlaneEffectEvents::SMALL_EFFECTS,                                  1.0F },
          { PlaneEffectEvents::MEDIUM_EFFECTS,                                 4.0F },
          { PlaneEffectEvents::LARGE_EFFECTS,                                  1.0F },
          { PlaneEffectEvents::VERY_LARGE_EFFECTS,                             1.0F },
          { PlaneEffectEvents::POSITIVE_VERTICAL_NEGATIVE_HORIZONTAL_EFFECTS,  1.0F },
          { PlaneEffectEvents::POSITIVE_HORIZONTAL_NEGATIVE_VERTICAL_EFFECTS,  1.0F },
          { PlaneEffectEvents::ZERO_EFFECTS,                                   2.0F },
      }
    }
// clang-format on
{
}

void Planes::SetRandomParams(const Point2dInt& zoomMidpoint, const uint32_t screenWidth)
{
  SetParams(GetRandomParams(m_goomRand, m_planeEffectWeights.GetRandomWeighted(), zoomMidpoint,
                            screenWidth));
}

auto Planes::GetRandomParams(const IGoomRand& goomRand,
                             const PlaneEffectEvents planeEffectsEvent,
                             const Point2dInt& zoomMidpoint,
                             const uint32_t screenWidth) -> Params
{
  const bool muchSpiralling = goomRand.ProbabilityOf(PROB_MUCH_SPIRALLING);

  return {
      GetRandomPlaneEffects(goomRand, planeEffectsEvent, muchSpiralling, zoomMidpoint, screenWidth),
      GetRandomSwirlEffects(goomRand, muchSpiralling)};
}

auto Planes::GetRandomPlaneEffects(const IGoomRand& goomRand,
                                   const PlaneEffectEvents planeEffectsEvent,
                                   const bool muchSpiralling,
                                   const Point2dInt& zoomMidpoint,
                                   const uint32_t screenWidth) -> PlaneEffects
{
  const IntAmplitudes intAmplitudes = GetRandomIntAmplitudes(goomRand, planeEffectsEvent);

  const IntAmplitudes adjustedIntAmplitudes =
      GetAdjustedIntAmplitudes(goomRand, intAmplitudes, zoomMidpoint, screenWidth);

  const Amplitudes effectMultipliers = GetRandomEffectMultipliers(goomRand, muchSpiralling);

  PlaneEffects planeEffects{};
  if (0 == adjustedIntAmplitudes.x)
  {
    planeEffects.horizontalEffectActive = false;
    planeEffects.amplitudes.x = 0.0F;
  }
  else
  {
    planeEffects.horizontalEffectActive = true;
    planeEffects.amplitudes.x = effectMultipliers.x * static_cast<float>(adjustedIntAmplitudes.x);
  }
  if (0 == adjustedIntAmplitudes.y)
  {
    planeEffects.verticalEffectActive = false;
    planeEffects.amplitudes.y = 0.0F;
  }
  else
  {
    planeEffects.verticalEffectActive = true;
    planeEffects.amplitudes.y = effectMultipliers.y * static_cast<float>(adjustedIntAmplitudes.y);
  }

  return planeEffects;
}

auto Planes::GetRandomIntAmplitudes(const IGoomRand& goomRand,
                                    const PlaneEffectEvents planeEffectsEvent) -> IntAmplitudes
{
  IntAmplitudes intAmplitudes{};

  switch (planeEffectsEvent)
  {
    case PlaneEffectEvents::ZERO_EFFECTS:
      intAmplitudes.x = 0;
      intAmplitudes.y = 0;
      break;
    case PlaneEffectEvents::SMALL_EFFECTS:
      intAmplitudes.x = goomRand.GetRandInRange(SMALL_EFFECTS_RANGE);
      intAmplitudes.y = goomRand.GetRandInRange(SMALL_EFFECTS_RANGE);
      break;
    case PlaneEffectEvents::MEDIUM_EFFECTS:
      intAmplitudes.y = goomRand.GetRandInRange(MEDIUM_EFFECTS_RANGE);
      intAmplitudes.x = goomRand.ProbabilityOf(PROB_HORIZONTAL_OPPOSITE_TO_VERTICAL)
                           ? (-intAmplitudes.y + 1)
                           : goomRand.GetRandInRange(MEDIUM_EFFECTS_RANGE);
      break;
    case PlaneEffectEvents::LARGE_EFFECTS:
      intAmplitudes.x = goomRand.GetRandInRange(LARGE_EFFECTS_RANGE);
      intAmplitudes.y = goomRand.ProbabilityOf(PROB_ZERO_VERTICAL_FOR_LARGE_RANGE)
                           ? 0
                           : goomRand.GetRandInRange(LARGE_EFFECTS_RANGE);
      break;
    case PlaneEffectEvents::VERY_LARGE_EFFECTS:
      intAmplitudes.x = goomRand.ProbabilityOf(PROB_ZERO_HORIZONTAL_FOR_VERY_LARGE_RANGE)
                           ? 0
                           : goomRand.GetRandInRange(VERY_LARGE_EFFECTS_RANGE);
      intAmplitudes.y = goomRand.GetRandInRange(VERY_LARGE_EFFECTS_RANGE);
      break;
    case PlaneEffectEvents::POSITIVE_VERTICAL_NEGATIVE_HORIZONTAL_EFFECTS:
      intAmplitudes.y = goomRand.GetRandInRange(VERY_LARGE_POSITIVE_EFFECTS_RANGE);
      intAmplitudes.x = -intAmplitudes.y + 1;
      break;
    case PlaneEffectEvents::POSITIVE_HORIZONTAL_NEGATIVE_VERTICAL_EFFECTS:
      intAmplitudes.x = goomRand.GetRandInRange(VERY_LARGE_POSITIVE_EFFECTS_RANGE);
      intAmplitudes.y = -intAmplitudes.x + 1;
      break;
    default:
      throw std::logic_error("Unknown PlaneEffectEvents enum.");
  }

  return intAmplitudes;
}

auto Planes::GetAdjustedIntAmplitudes(const IGoomRand& goomRand,
                                      const IntAmplitudes& intAmplitudes,
                                      const Point2dInt& zoomMidpoint,
                                      const uint32_t screenWidth) -> IntAmplitudes
{
  IntAmplitudes adjustedIntAmplitudes = intAmplitudes;

  if ((1 == zoomMidpoint.x) || (zoomMidpoint.x == static_cast<int32_t>(screenWidth - 1)))
  {
    adjustedIntAmplitudes.y = 0;
    if (goomRand.ProbabilityOf(PROB_ZERO_HORIZONTAL_PLANE_EFFECT))
    {
      adjustedIntAmplitudes.x = 0;
    }
  }

  return adjustedIntAmplitudes;
}

auto Planes::GetRandomEffectMultipliers(const IGoomRand& goomRand, const bool muchSpiralling)
    -> Amplitudes
{
  Amplitudes effectMultipliers{};

  effectMultipliers.x =
      muchSpiralling ? goomRand.GetRandInRange(HORIZONTAL_EFFECTS_SPIRALLING_MULTIPLIER_RANGE)
                     : goomRand.GetRandInRange(HORIZONTAL_EFFECTS_MULTIPLIER_RANGE);

  if (goomRand.ProbabilityOf(PROB_PLANE_AMPLITUDES_EQUAL))
  {
    effectMultipliers.y = effectMultipliers.x;
  }
  else
  {
    effectMultipliers.y =
        muchSpiralling ? goomRand.GetRandInRange(VERTICAL_EFFECTS_SPIRALLING_AMPLITUDE_RANGE)
                       : goomRand.GetRandInRange(VERTICAL_EFFECTS_AMPLITUDE_RANGE);
  }

  return effectMultipliers;
}

auto Planes::GetRandomSwirlEffects(const UTILS::IGoomRand& goomRand, const bool muchSpiralling)
    -> PlaneSwirlEffects
{
  PlaneSwirlEffects swirlEffects{};

  if (muchSpiralling || goomRand.ProbabilityOf(PROB_NO_SWIRL))
  {
    swirlEffects.swirlType = PlaneSwirlType::NONE;
    swirlEffects.frequencies.x = 0.0F;
    swirlEffects.frequencies.y = 0.0F;
    swirlEffects.amplitudes.x = 0.0F;
    swirlEffects.amplitudes.y = 0.0F;

    return swirlEffects;
  }

  swirlEffects.swirlType =
      static_cast<PlaneSwirlType>(goomRand.GetRandInRange(1U, NUM<PlaneSwirlType>));

  swirlEffects.frequencies.x = goomRand.GetRandInRange(HORIZONTAL_SWIRL_FREQ_RANGE);
  if (goomRand.ProbabilityOf(PROB_SWIRL_FREQ_EQUAL))
  {
    swirlEffects.frequencies.y = swirlEffects.frequencies.x;
  }
  else
  {
    swirlEffects.frequencies.y = goomRand.GetRandInRange(VERTICAL_SWIRL_FREQ_RANGE);
  }

  swirlEffects.amplitudes.x = goomRand.GetRandInRange(HORIZONTAL_SWIRL_AMPLITUDE_RANGE);
  if (goomRand.ProbabilityOf(PROB_SWIRL_AMPLITUDES_EQUAL))
  {
    swirlEffects.amplitudes.y = swirlEffects.amplitudes.x;
  }
  else
  {
    swirlEffects.amplitudes.y = goomRand.GetRandInRange(VERTICAL_SWIRL_AMPLITUDE_RANGE);
  }

  return swirlEffects;
}

auto Planes::GetHorizontalPlaneVelocity(const NormalizedCoords& coords) const -> float
{
  const float coordValue = coords.GetY();
  const float coordSwirlOffset =
      GetHorizontalSwirlOffsetFactor(coordValue) * m_params.swirlEffects.amplitudes.x;

  return m_params.planeEffects.amplitudes.x * (coordValue + coordSwirlOffset);
}

auto Planes::GetVerticalPlaneVelocity(const NormalizedCoords& coords) const -> float
{
  const float coordValue = coords.GetX();
  const float coordSwirlOffset =
      GetVerticalSwirlOffsetFactor(coordValue) * m_params.swirlEffects.amplitudes.y;

  return m_params.planeEffects.amplitudes.y * (coordValue + coordSwirlOffset);
}

auto Planes::GetHorizontalSwirlOffsetFactor(const float coordValue) const -> float
{
  const float swirlFreq = m_params.swirlEffects.frequencies.x;

  switch (m_params.swirlEffects.swirlType)
  {
    case PlaneSwirlType::NONE:
      return 0.0F;
    case PlaneSwirlType::SIN_CURL_SWIRL:
    case PlaneSwirlType::SIN_COS_CURL_SWIRL:
      // 'sin' for horizontal
      return std::sin(swirlFreq * coordValue);
    case PlaneSwirlType::COS_CURL_SWIRL:
    case PlaneSwirlType::COS_SIN_CURL_SWIRL:
      // 'cos' for horizontal
      return std::cos(swirlFreq * coordValue);
    case PlaneSwirlType::SIN_OF_COS_SWIRL:
      // 'sin' for horizontal
      return std::sin(m_pi * std::cos(swirlFreq * coordValue));
    case PlaneSwirlType::COS_OF_SIN_SWIRL:
      // 'cos' for horizontal
      return std::cos(m_pi * std::sin(swirlFreq * coordValue));
    default:
      throw std::logic_error("Unknown horizontal plane swirl type");
  }
}

auto Planes::GetVerticalSwirlOffsetFactor(const float coordValue) const -> float
{
  const float swirlFreq = m_params.swirlEffects.frequencies.y;

  switch (m_params.swirlEffects.swirlType)
  {
    case PlaneSwirlType::NONE:
      return 0.0F;
    case PlaneSwirlType::SIN_CURL_SWIRL:
    case PlaneSwirlType::SIN_COS_CURL_SWIRL:
      // 'cos' for vertical
      return std::cos(swirlFreq * coordValue);
    case PlaneSwirlType::COS_CURL_SWIRL:
    case PlaneSwirlType::COS_SIN_CURL_SWIRL:
      // 'sin' for vertical
      return std::sin(swirlFreq * coordValue);
    case PlaneSwirlType::SIN_OF_COS_SWIRL:
      // 'cos' for vertical
      return std::cos(m_pi * std::cos(swirlFreq * coordValue));
    case PlaneSwirlType::COS_OF_SIN_SWIRL:
      // 'sin' for vertical
      return std::sin(m_pi * std::sin(swirlFreq * coordValue));
    default:
      throw std::logic_error("Unknown vertical plane swirl type");
  }
}

auto Planes::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const std::string fullParamGroup = GetFullParamGroup({paramGroup, "planes"});
  return {
      GetPair(fullParamGroup, "planeEffects.amplitudes.x", m_params.planeEffects.amplitudes.x),
      GetPair(fullParamGroup, "planeEffects.amplitudes.y", m_params.planeEffects.amplitudes.y),
      GetPair(fullParamGroup, "swirlEffects.swirlType",
              static_cast<int32_t>(m_params.swirlEffects.swirlType)),
      GetPair(fullParamGroup, "swirlEffects.frequencies.x", m_params.swirlEffects.frequencies.x),
      GetPair(fullParamGroup, "swirlEffects.frequencies.y", m_params.swirlEffects.frequencies.y),
      GetPair(fullParamGroup, "swirlEffects.amplitudes.x", m_params.swirlEffects.amplitudes.x),
      GetPair(fullParamGroup, "swirlEffects.amplitudes.y", m_params.swirlEffects.amplitudes.y),
  };
}

} // namespace GOOM::VISUAL_FX::FILTERS
