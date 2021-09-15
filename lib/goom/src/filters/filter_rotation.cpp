#include "filter_rotation.h"

#include "goomutils/goomrand.h"
#include "goomutils/name_value_pairs.h"

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

using UTILS::GetPair;
using UTILS::GetRandInRange;
using UTILS::NameValuePairs;
using UTILS::NumberRange;
using UTILS::ProbabilityOf;

constexpr float DEFAULT_ROTATE_SPEED = 0.0F;
constexpr NumberRange<float> ROTATE_SPEED_RANGE = {-0.5F, +0.5F};
constexpr float PROB_EQUAL_XY_ROTATION = 0.8F;

constexpr float DEFAULT_FACTOR = 1.0F;
constexpr NumberRange<float> FACTOR_RANGE = {0.3F, 1.0F};
constexpr float PROB_NO_FACTORS = 0.5F;
constexpr float PROB_EQUAL_FACTORS = 0.8F;


Rotation::Rotation() noexcept
  : m_params{DEFAULT_ROTATE_SPEED, DEFAULT_ROTATE_SPEED, DEFAULT_FACTOR,
             DEFAULT_FACTOR,       DEFAULT_FACTOR,       DEFAULT_FACTOR}
{
}

void Rotation::SetRandomParams()
{
  m_params.xRotateSpeed = GetRandInRange(ROTATE_SPEED_RANGE);
  m_params.yRotateSpeed = ProbabilityOf(PROB_EQUAL_XY_ROTATION)
                              ? m_params.xRotateSpeed
                              : GetRandInRange(ROTATE_SPEED_RANGE);

  if (ProbabilityOf(PROB_NO_FACTORS))
  {
    m_params.xxFactor = 1.0F;
    m_params.xyFactor = 1.0F;
    m_params.yxFactor = 1.0F;
    m_params.yyFactor = 1.0F;
  }
  else
  {
    m_params.xxFactor = GetRandInRange(FACTOR_RANGE);
    m_params.xyFactor = GetRandInRange(FACTOR_RANGE);

    if (ProbabilityOf(PROB_EQUAL_FACTORS))
    {
      m_params.yxFactor = m_params.xxFactor;
      m_params.yyFactor = m_params.xyFactor;
    }
    else
    {
      m_params.yxFactor = GetRandInRange(FACTOR_RANGE);
      m_params.yyFactor = GetRandInRange(FACTOR_RANGE);
    }
  }
}

auto Rotation::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  return {
      GetPair(paramGroup, "x rotate speed", m_params.xRotateSpeed),
      GetPair(paramGroup, "y rotate speed", m_params.yRotateSpeed),
      GetPair(paramGroup, "xx factor", m_params.xxFactor),
      GetPair(paramGroup, "xy factor", m_params.xyFactor),
      GetPair(paramGroup, "yx factor", m_params.yxFactor),
      GetPair(paramGroup, "yy factor", m_params.yyFactor),
  };
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
