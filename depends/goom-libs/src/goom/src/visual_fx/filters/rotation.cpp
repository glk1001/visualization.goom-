#include "rotation.h"

#include "utils/mathutils.h"
#include "utils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>
#include <cmath>
#include <string>

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
using UTILS::m_pi;
using UTILS::NameValuePairs;

constexpr float DEFAULT_ROTATE_SPEED = 0.0F;
constexpr IGoomRand::NumberRange<float> ROTATE_SPEED_RANGE = {-0.5F, +0.5F};
constexpr float PROB_EQUAL_XY_ROTATE_SPEEDS = 0.8F;

constexpr float DEFAULT_ROTATE_ANGLE = m_pi / 4.0F;
constexpr IGoomRand::NumberRange<float> ANGLE_RANGE = {(1.0F / 8.0F) * m_pi, (3.0F / 8.0F) * m_pi};

Rotation::Rotation(IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_params{DEFAULT_ROTATE_SPEED, DEFAULT_ROTATE_SPEED, std::sin(DEFAULT_ROTATE_ANGLE),
             std::cos(DEFAULT_ROTATE_ANGLE)}
{
}

void Rotation::SetRandomParams()
{
  m_params.xRotateSpeed = m_goomRand.GetRandInRange(ROTATE_SPEED_RANGE);
  m_params.yRotateSpeed = m_goomRand.ProbabilityOf(PROB_EQUAL_XY_ROTATE_SPEEDS)
                              ? m_params.xRotateSpeed
                              : m_goomRand.GetRandInRange(ROTATE_SPEED_RANGE);

  if ((m_params.xRotateSpeed < 0.0F) && (m_params.yRotateSpeed > 0.0F))
  {
    m_params.yRotateSpeed = -m_params.yRotateSpeed;
  }
  else if ((m_params.xRotateSpeed > 0.0F) && (m_params.yRotateSpeed < 0.0F))
  {
    m_params.yRotateSpeed = -m_params.yRotateSpeed;
  }

  const float angle = m_goomRand.GetRandInRange(ANGLE_RANGE);
  m_params.sinAngle = std::sin(angle);
  m_params.cosAngle = std::cos(angle);
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

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::FILTERS
#endif
