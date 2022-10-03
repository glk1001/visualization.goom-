#include "planes.h"

#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

using STD20::pi;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::MATH::IGoomRand;

static constexpr auto SMALL_EFFECTS_RANGE          = IGoomRand::NumberRange<int32_t>{-2, +2 + 1};
static constexpr auto MEDIUM_EFFECTS_RANGE         = IGoomRand::NumberRange<int32_t>{-5, +5 + 1};
static constexpr auto LARGE_EFFECTS_RANGE          = IGoomRand::NumberRange<int32_t>{-7, +7 + 1};
static constexpr auto VERY_LARGE_EFFECTS_RANGE     = IGoomRand::NumberRange<int32_t>{-9, +9 + 1};
static constexpr auto VERY_LARGE_POS_EFFECTS_RANGE = IGoomRand::NumberRange<int32_t>{+5, +12 + 1};

static constexpr auto PROB_ZERO_HORIZONTAL_FOR_VERY_LARGE_RANGE = 0.2F;
static constexpr auto PROB_ZERO_VERTICAL_FOR_LARGE_RANGE        = 0.2F;
static constexpr auto PROB_OPPOSITES_FOR_SMALL_EFFECTS          = 0.1F;
static constexpr auto PROB_OPPOSITES_FOR_MEDIUM_EFFECTS         = 0.9F;

// H Plane:
// @since June 2001
// clang-format off
static constexpr auto
  HORIZONTAL_EFFECTS_MULTIPLIER_RANGE            = IGoomRand::NumberRange<float>{0.0015F, 0.0035F};
static constexpr auto
  HORIZONTAL_EFFECTS_SPIRALLING_MULTIPLIER_RANGE = IGoomRand::NumberRange<float>{0.0015F, 0.0351F};
// clang-format on

static constexpr auto DEFAULT_HORIZONTAL_SWIRL_FREQ = 1.0F;
static constexpr auto HORIZONTAL_SWIRL_FREQ_RANGE   = IGoomRand::NumberRange<float>{0.1F, 5.01F};
static constexpr auto DEFAULT_HORIZONTAL_SWIRL_AMPLITUDE = 1.0F;
static constexpr auto HORIZONTAL_SWIRL_AMPLITUDE_RANGE = IGoomRand::NumberRange<float>{0.1F, 5.01F};

// V Plane:
// clang-format off
static constexpr auto
  VERTICAL_EFFECTS_AMPLITUDE_RANGE            = IGoomRand::NumberRange<float>{0.0015F, 0.0035F};
static constexpr auto
  VERTICAL_EFFECTS_SPIRALLING_AMPLITUDE_RANGE = IGoomRand::NumberRange<float>{0.0015F, 0.0351F};
// clang-format on

static constexpr auto DEFAULT_VERTICAL_SWIRL_FREQ = 1.0F;
static constexpr auto VERTICAL_SWIRL_FREQ_RANGE   = IGoomRand::NumberRange<float>{0.1F, 30.01F};
static constexpr auto DEFAULT_VERTICAL_SWIRL_AMPLITUDE = 1.0F;
static constexpr auto VERTICAL_SWIRL_AMPLITUDE_RANGE = IGoomRand::NumberRange<float>{0.1F, 30.01F};

static constexpr auto PROB_PLANE_AMPLITUDES_EQUAL       = 0.75F;
static constexpr auto PROB_ZERO_HORIZONTAL_PLANE_EFFECT = 0.50F;
static constexpr auto PROB_MUCH_SPIRALLING              = 0.20F;
static constexpr auto PROB_NO_SWIRL                     = 0.95F;
static constexpr auto PROB_SWIRL_AMPLITUDES_EQUAL       = 0.70F;
static constexpr auto PROB_SWIRL_FREQ_EQUAL             = 0.70F;

static constexpr auto SMALL_PLANE_EFFECTS_WEIGHT                           = 1.0F;
static constexpr auto MEDIUM_EFFECTS_WEIGHT                                = 4.0F;
static constexpr auto LARGE_EFFECTS_WEIGHT                                 = 1.0F;
static constexpr auto VERY_LARGE_EFFECTS_WEIGHT                            = 1.0F;
static constexpr auto POSITIVE_VERTICAL_NEGATIVE_HORIZONTAL_EFFECTS_WEIGHT = 1.0F;
static constexpr auto POSITIVE_HORIZONTAL_NEGATIVE_VERTICAL_EFFECTS_WEIGHT = 1.0F;
static constexpr auto ZERO_EFFECTS_WEIGHT                                  = 2.0F;

Planes::Planes(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
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
          { PlaneEffectEvents::ZERO_EFFECTS,       ZERO_EFFECTS_WEIGHT },
          { PlaneEffectEvents::SMALL_EFFECTS,      SMALL_PLANE_EFFECTS_WEIGHT },
          { PlaneEffectEvents::MEDIUM_EFFECTS,     MEDIUM_EFFECTS_WEIGHT },
          { PlaneEffectEvents::LARGE_EFFECTS,      LARGE_EFFECTS_WEIGHT },
          { PlaneEffectEvents::VERY_LARGE_EFFECTS, VERY_LARGE_EFFECTS_WEIGHT },
          { PlaneEffectEvents::POS_VERTICAL_NEG_HORIZONTAL_VERY_LARGE_EFFECTS,
                                             POSITIVE_VERTICAL_NEGATIVE_HORIZONTAL_EFFECTS_WEIGHT },
          { PlaneEffectEvents::POS_HORIZONTAL_NEG_VERTICAL_VERY_LARGE_EFFECTS,
                                             POSITIVE_HORIZONTAL_NEGATIVE_VERTICAL_EFFECTS_WEIGHT },
      }
    }
{
}

auto Planes::SetRandomParams(const Point2dInt& zoomMidpoint, const uint32_t screenWidth) -> void
{
  SetParams(GetRandomParams(
      m_goomRand, m_planeEffectWeights.GetRandomWeighted(), zoomMidpoint, screenWidth));
}

auto Planes::GetRandomParams(const IGoomRand& goomRand,
                             const PlaneEffectEvents planeEffectsEvent,
                             const Point2dInt& zoomMidpoint,
                             const uint32_t screenWidth) -> Params
{
  const auto muchSpiralling = goomRand.ProbabilityOf(PROB_MUCH_SPIRALLING);

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
  const auto intAmplitudes = GetRandomIntAmplitudes(goomRand, planeEffectsEvent);
  const auto adjustedIntAmplitudes =
      GetAdjustedIntAmplitudes(goomRand, intAmplitudes, zoomMidpoint, screenWidth);

  const auto effectMultipliers = GetRandomEffectMultipliers(goomRand, muchSpiralling);

  return GetRandomPlaneEffects(adjustedIntAmplitudes, effectMultipliers);
}

inline auto Planes::GetRandomPlaneEffects(const IntAmplitudes& adjustedIntAmplitudes,
                                          const Amplitudes& effectMultipliers) -> PlaneEffects
{
  auto planeEffects = PlaneEffects{};

  if (0 == adjustedIntAmplitudes.x)
  {
    planeEffects.horizontalEffectActive = false;
    planeEffects.amplitudes.x           = 0.0F;
  }
  else
  {
    planeEffects.horizontalEffectActive = true;
    planeEffects.amplitudes.x = effectMultipliers.x * static_cast<float>(adjustedIntAmplitudes.x);
  }
  if (0 == adjustedIntAmplitudes.y)
  {
    planeEffects.verticalEffectActive = false;
    planeEffects.amplitudes.y         = 0.0F;
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
  auto intAmplitudes = IntAmplitudes{};

  switch (planeEffectsEvent)
  {
    case PlaneEffectEvents::ZERO_EFFECTS:
      intAmplitudes.x = 0;
      intAmplitudes.y = 0;
      break;
    case PlaneEffectEvents::SMALL_EFFECTS:
      intAmplitudes.x = goomRand.GetRandInRange(SMALL_EFFECTS_RANGE);
      intAmplitudes.y = goomRand.ProbabilityOf(PROB_OPPOSITES_FOR_SMALL_EFFECTS)
                            ? (-intAmplitudes.x + 1)
                            : goomRand.GetRandInRange(SMALL_EFFECTS_RANGE);
      break;
    case PlaneEffectEvents::MEDIUM_EFFECTS:
      intAmplitudes.x = goomRand.GetRandInRange(MEDIUM_EFFECTS_RANGE);
      intAmplitudes.y = goomRand.ProbabilityOf(PROB_OPPOSITES_FOR_MEDIUM_EFFECTS)
                            ? (-intAmplitudes.x + 1)
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
    case PlaneEffectEvents::POS_VERTICAL_NEG_HORIZONTAL_VERY_LARGE_EFFECTS:
      intAmplitudes.y = goomRand.GetRandInRange(VERY_LARGE_POS_EFFECTS_RANGE);
      intAmplitudes.x = -intAmplitudes.y + 1;
      break;
    case PlaneEffectEvents::POS_HORIZONTAL_NEG_VERTICAL_VERY_LARGE_EFFECTS:
      intAmplitudes.x = goomRand.GetRandInRange(VERY_LARGE_POS_EFFECTS_RANGE);
      intAmplitudes.y = -intAmplitudes.x + 1;
      break;
    default:
      FailFast();
  }

  return intAmplitudes;
}

auto Planes::GetAdjustedIntAmplitudes(const IGoomRand& goomRand,
                                      const IntAmplitudes& intAmplitudes,
                                      const Point2dInt& zoomMidpoint,
                                      const uint32_t screenWidth) -> IntAmplitudes
{
  auto adjustedIntAmplitudes = intAmplitudes;

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
  auto effectMultipliers = Amplitudes{};

  effectMultipliers.x =
      muchSpiralling ? goomRand.GetRandInRange(HORIZONTAL_EFFECTS_SPIRALLING_MULTIPLIER_RANGE)
                     : goomRand.GetRandInRange(HORIZONTAL_EFFECTS_MULTIPLIER_RANGE);

  if (goomRand.ProbabilityOf(PROB_PLANE_AMPLITUDES_EQUAL))
  {
    effectMultipliers.y = effectMultipliers.x;
  }
  else
  {
    effectMultipliers.y = muchSpiralling
                              ? goomRand.GetRandInRange(VERTICAL_EFFECTS_SPIRALLING_AMPLITUDE_RANGE)
                              : goomRand.GetRandInRange(VERTICAL_EFFECTS_AMPLITUDE_RANGE);
  }

  return effectMultipliers;
}

auto Planes::GetRandomSwirlEffects(const UTILS::MATH::IGoomRand& goomRand,
                                   const bool muchSpiralling) -> PlaneSwirlEffects
{
  if (muchSpiralling || goomRand.ProbabilityOf(PROB_NO_SWIRL))
  {
    return GetZeroSwirlEffects();
  }

  return GetNonzeroRandomSwirlEffects(goomRand);
}

inline auto Planes::GetZeroSwirlEffects() -> PlaneSwirlEffects
{
  return {
      PlaneSwirlType::NONE,
      {0.0F, 0.0F},
      {0.0F, 0.0F},
  };
}

inline auto Planes::GetNonzeroRandomSwirlEffects(const UTILS::MATH::IGoomRand& goomRand)
    -> PlaneSwirlEffects
{
  auto swirlEffects = PlaneSwirlEffects{};

  swirlEffects.swirlType =
      static_cast<PlaneSwirlType>(goomRand.GetRandInRange(1U, NUM<PlaneSwirlType>));

  swirlEffects.frequencies.x = goomRand.GetRandInRange(HORIZONTAL_SWIRL_FREQ_RANGE);
  swirlEffects.frequencies.y = goomRand.ProbabilityOf(PROB_SWIRL_FREQ_EQUAL)
                                   ? swirlEffects.frequencies.x
                                   : goomRand.GetRandInRange(VERTICAL_SWIRL_FREQ_RANGE);

  swirlEffects.amplitudes.x = goomRand.GetRandInRange(HORIZONTAL_SWIRL_AMPLITUDE_RANGE);
  swirlEffects.amplitudes.y = goomRand.ProbabilityOf(PROB_SWIRL_AMPLITUDES_EQUAL)
                                  ? swirlEffects.amplitudes.x
                                  : goomRand.GetRandInRange(VERTICAL_SWIRL_AMPLITUDE_RANGE);

  return swirlEffects;
}

auto Planes::GetHorizontalPlaneVelocity(const NormalizedCoords& coords,
                                        const NormalizedCoords& velocity) const -> float
{
  const auto yCoordValue = coords.GetY();
  const auto horizontalSwirlOffset =
      m_params.swirlEffects.amplitudes.x * GetHorizontalSwirlOffsetFactor(yCoordValue);

  return velocity.GetX() +
         m_params.planeEffects.amplitudes.x * (yCoordValue + horizontalSwirlOffset);
}

auto Planes::GetVerticalPlaneVelocity(const NormalizedCoords& coords,
                                      const NormalizedCoords& velocity) const -> float
{
  const auto xCoordValue = coords.GetX();
  const auto verticalSwirlOffset =
      m_params.swirlEffects.amplitudes.y * GetVerticalSwirlOffsetFactor(xCoordValue);

  return velocity.GetY() + m_params.planeEffects.amplitudes.y * (xCoordValue + verticalSwirlOffset);
}

auto Planes::GetHorizontalSwirlOffsetFactor(const float coordValue) const -> float
{
  const auto swirlFreq = m_params.swirlEffects.frequencies.x;

  switch (m_params.swirlEffects.swirlType)
  {
    case PlaneSwirlType::NONE:
      return 0.0F;
    case PlaneSwirlType::SIN_CURL_SWIRL:
    case PlaneSwirlType::SIN_COS_CURL_SWIRL:
      // 'sin' is for horizontal
      return std::sin(swirlFreq * coordValue);
    case PlaneSwirlType::COS_CURL_SWIRL:
    case PlaneSwirlType::COS_SIN_CURL_SWIRL:
      // 'cos' is for horizontal
      return std::cos(swirlFreq * coordValue);
    case PlaneSwirlType::SIN_OF_COS_SWIRL:
      // 'sin' is for horizontal
      return std::sin(pi * std::cos(swirlFreq * coordValue));
    case PlaneSwirlType::COS_OF_SIN_SWIRL:
      // 'cos' is for horizontal
      return std::cos(pi * std::sin(swirlFreq * coordValue));
    default:
      FailFast();
      return 0.0F;
  }
}

auto Planes::GetVerticalSwirlOffsetFactor(const float coordValue) const -> float
{
  const auto swirlFreq = m_params.swirlEffects.frequencies.y;

  switch (m_params.swirlEffects.swirlType)
  {
    case PlaneSwirlType::NONE:
      return 0.0F;
    case PlaneSwirlType::SIN_CURL_SWIRL:
    case PlaneSwirlType::SIN_COS_CURL_SWIRL:
      // 'cos' is for vertical
      return std::cos(swirlFreq * coordValue);
    case PlaneSwirlType::COS_CURL_SWIRL:
    case PlaneSwirlType::COS_SIN_CURL_SWIRL:
      // 'sin' is for vertical
      return std::sin(swirlFreq * coordValue);
    case PlaneSwirlType::SIN_OF_COS_SWIRL:
      // 'cos' is for vertical
      return std::cos(pi * std::cos(swirlFreq * coordValue));
    case PlaneSwirlType::COS_OF_SIN_SWIRL:
      // 'sin' is for vertical
      return std::sin(pi * std::sin(swirlFreq * coordValue));
    default:
      FailFast();
      return 0.0F;
  }
}

auto Planes::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({paramGroup, "planes"});
  return {
      GetPair(fullParamGroup,
              "planeEffects.amplitudes",
              Point2dFlt{m_params.planeEffects.amplitudes.x, m_params.planeEffects.amplitudes.y}),
      GetPair(fullParamGroup,
              "swirlEffects.swirlType",
              static_cast<int32_t>(m_params.swirlEffects.swirlType)),
      GetPair(fullParamGroup,
              "swirlEffects.frequencies",
              Point2dFlt{m_params.swirlEffects.frequencies.x, m_params.swirlEffects.frequencies.y}),
      GetPair(fullParamGroup,
              "swirlEffects.amplitudes",
              Point2dFlt{m_params.swirlEffects.amplitudes.x, m_params.swirlEffects.amplitudes.y}),
  };
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
