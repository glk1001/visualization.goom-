#pragma once

#include "normalized_coords.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/mathutils.h"
#include "utils/name_value_pairs.h"

#include <cmath>
#include <string>

namespace GOOM::VISUAL_FX::FILTERS
{

class Rotation
{
public:
  explicit Rotation(const UTILS::MATH::IGoomRand& goomRand) noexcept;
  Rotation(const Rotation&) noexcept = delete;
  Rotation(Rotation&&) noexcept = delete;
  virtual ~Rotation() noexcept = default;
  auto operator=(const Rotation&) -> Rotation& = delete;
  auto operator=(Rotation&&) -> Rotation& = delete;

  virtual void SetRandomParams();

  void SetZero();
  [[nodiscard]] auto IsActive() const -> bool;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords;

  void Multiply(float factor);
  void Toggle();

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  struct Params
  {
    float xRotateSpeed;
    float yRotateSpeed;
    float sinAngle;
    float cosAngle;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  Params m_params;
};

inline auto Rotation::GetVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords
{
  float xRotateSpeed = m_params.xRotateSpeed;
  float yRotateSpeed = m_params.yRotateSpeed;
  float sinAngle = m_params.sinAngle;
  const float cosAngle = m_params.cosAngle;
  if (m_params.xRotateSpeed < 0.0F)
  {
    xRotateSpeed = -xRotateSpeed;
    yRotateSpeed = -yRotateSpeed;
    sinAngle = -sinAngle;
  }

  return {xRotateSpeed * ((cosAngle * velocity.GetX()) - (sinAngle * velocity.GetY())),
          yRotateSpeed * ((sinAngle * velocity.GetX()) + (cosAngle * velocity.GetY()))};
}

inline void Rotation::SetZero()
{
  m_params.xRotateSpeed = 0.0F;
  m_params.yRotateSpeed = 0.0F;
}

inline void Rotation::Multiply(const float factor)
{
  m_params.xRotateSpeed *= factor;
  m_params.yRotateSpeed *= factor;
}

inline void Rotation::Toggle()
{
  m_params.xRotateSpeed = -m_params.xRotateSpeed;
  m_params.yRotateSpeed = -m_params.yRotateSpeed;
}

inline auto Rotation::IsActive() const -> bool
{
  return (std::fabs(m_params.xRotateSpeed) > UTILS::MATH::SMALL_FLOAT) ||
         (std::fabs(m_params.yRotateSpeed) > UTILS::MATH::SMALL_FLOAT);
}

inline auto Rotation::GetParams() const -> const Params&
{
  return m_params;
}

inline void Rotation::SetParams(const Params& params)
{
  m_params = params;
}

} // namespace GOOM::VISUAL_FX::FILTERS
