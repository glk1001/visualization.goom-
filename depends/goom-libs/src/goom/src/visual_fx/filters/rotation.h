#pragma once

#include "normalized_coords.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

#include <cmath>
#include <string>

namespace GOOM::VISUAL_FX::FILTERS
{

class RotationAdjustments
{
public:
  RotationAdjustments() noexcept = default;

  [[nodiscard]] auto AreAdjustmentsPending() const -> bool;

  [[nodiscard]] auto IsSetToZero() const -> bool;
  void SetToZero();

  [[nodiscard]] auto IsToggle() const -> bool;
  void Toggle();

  [[nodiscard]] auto GetMultiplyfactor() const -> float;
  void SetMultiplyFactor(float value);

  void Reset();

private:
  bool m_setToZero = false;
  bool m_toggle = false;
  float m_multiplyFactor = 1.0F;
  bool m_adjustmentsPending = false;
};

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

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords;

  void ApplyAdjustments(const RotationAdjustments& rotationAdjustments);

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

  void SetZero();
  void Multiply(float factor);
  void Toggle();
};

inline auto RotationAdjustments::AreAdjustmentsPending() const -> bool
{
  return m_adjustmentsPending;
}

inline auto RotationAdjustments::IsSetToZero() const -> bool
{
  return m_setToZero;
}

inline void RotationAdjustments::SetToZero()
{
  m_setToZero = true;
  m_adjustmentsPending = true;
}

inline auto RotationAdjustments::IsToggle() const -> bool
{
  return m_toggle;
}

inline void RotationAdjustments::Toggle()
{
  m_toggle = true;
  m_adjustmentsPending = true;
}

inline auto RotationAdjustments::GetMultiplyfactor() const -> float
{
  return m_multiplyFactor;
}

inline void RotationAdjustments::SetMultiplyFactor(const float value)
{
  m_multiplyFactor = value;
  m_adjustmentsPending = true;
}

inline void RotationAdjustments::Reset()
{
  m_adjustmentsPending = false;
  m_setToZero = false;
  m_toggle = false;
  m_multiplyFactor = 1.0F;
}

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

inline void Rotation::ApplyAdjustments(const RotationAdjustments& rotationAdjustments)
{
  if (!rotationAdjustments.AreAdjustmentsPending())
  {
    return;
  }

  if (rotationAdjustments.IsSetToZero())
  {
    SetZero();
  }
  if (rotationAdjustments.IsToggle())
  {
    Toggle();
  }
  Multiply(rotationAdjustments.GetMultiplyfactor());
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

inline auto Rotation::GetParams() const -> const Params&
{
  return m_params;
}

inline void Rotation::SetParams(const Params& params)
{
  m_params = params;
}

} // namespace GOOM::VISUAL_FX::FILTERS
