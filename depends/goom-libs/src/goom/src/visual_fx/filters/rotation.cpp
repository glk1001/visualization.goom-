#include "rotation.h"

#include "utils/mathutils.h"
#include "utils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>
#include <cmath>
#include <string>

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::IGoomRand;
using UTILS::m_pi;
using UTILS::NameValuePairs;

constexpr float DEFAULT_ROTATE_SPEED = 0.0F;
constexpr IGoomRand::NumberRange<float> ROTATE_SPEED_RANGE = {-0.5F, +0.5F};
constexpr float PROB_EQUAL_XY_ROTATE_SPEEDS = 0.8F;

constexpr float DEFAULT_ROTATE_ANGLE = m_pi / 4.0F;
constexpr IGoomRand::NumberRange<float> ANGLE_RANGE = {(1.0F / 8.0F) * m_pi, (3.0F / 8.0F) * m_pi};

Rotation::Rotation(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_params{DEFAULT_ROTATE_SPEED, DEFAULT_ROTATE_SPEED, std::sin(DEFAULT_ROTATE_ANGLE),
             std::cos(DEFAULT_ROTATE_ANGLE)}
{
}

void Rotation::SetRandomParams()
{
  const float xRotateSpeed = m_goomRand.GetRandInRange(ROTATE_SPEED_RANGE);
  float yRotateSpeed = m_goomRand.ProbabilityOf(PROB_EQUAL_XY_ROTATE_SPEEDS)
                           ? xRotateSpeed
                           : m_goomRand.GetRandInRange(ROTATE_SPEED_RANGE);

  if (((xRotateSpeed < 0.0F) && (yRotateSpeed > 0.0F)) ||
      ((xRotateSpeed > 0.0F) && (yRotateSpeed < 0.0F)))
  {
    yRotateSpeed = -yRotateSpeed;
  }

  const float angle = m_goomRand.GetRandInRange(ANGLE_RANGE);
  const float sinAngle = std::sin(angle);
  const float cosAngle = std::cos(angle);

  SetParams({xRotateSpeed, yRotateSpeed, sinAngle, cosAngle});
}

auto Rotation::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  if (!IsActive())
  {
    return {GetPair(paramGroup, "rotation", std::string{"None"})};
  }

  const std::string fullParamGroup = GetFullParamGroup({paramGroup, "rotate"});
  return {
      GetPair(fullParamGroup, "x speed", m_params.xRotateSpeed),
      GetPair(fullParamGroup, "y speed", m_params.yRotateSpeed),
      GetPair(fullParamGroup, "sinAngle", m_params.sinAngle),
      GetPair(fullParamGroup, "cosAngle", m_params.cosAngle),
  };
}

} // namespace GOOM::VISUAL_FX::FILTERS
