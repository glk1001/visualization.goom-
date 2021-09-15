#ifndef VISUALIZATION_GOOM_FILTER_ROTATION_H
#define VISUALIZATION_GOOM_FILTER_ROTATION_H

#include "filter_normalized_coords.h"
#include "goomutils/mathutils.h"
#include "goomutils/name_value_pairs.h"

#include <cmath>
#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class Rotation
{
public:
  Rotation() noexcept;

  void SetRandomParams();

  void SetZero();
  [[nodiscard]] auto IsActive() const -> bool;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords;

  void Multiply(const float factor);
  void Toggle();

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  struct Params
  {
    float xRotateSpeed;
    float yRotateSpeed;
    float xxFactor;
    float xyFactor;
    float yxFactor;
    float yyFactor;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  Params m_params;
};

inline auto Rotation::GetVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords
{
  if (m_params.xRotateSpeed < 0.0F)
  {
    return {-m_params.xRotateSpeed *
                ((m_params.xxFactor * velocity.GetX()) - (m_params.xyFactor * velocity.GetY())),
            -m_params.yRotateSpeed *
                ((m_params.yxFactor * velocity.GetX()) + (m_params.yyFactor * velocity.GetY()))};
  }

  return {m_params.xRotateSpeed *
              ((m_params.xyFactor * velocity.GetY()) - (m_params.xxFactor * velocity.GetX())),
          m_params.yRotateSpeed *
              ((m_params.yyFactor * velocity.GetY()) + (m_params.yxFactor * velocity.GetX()))};
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
  return (std::fabs(m_params.xRotateSpeed) > UTILS::SMALL_FLOAT) ||
         (std::fabs(m_params.yRotateSpeed) > UTILS::SMALL_FLOAT);
}

inline auto Rotation::GetParams() const -> const Params&
{
  return m_params;
}

inline void Rotation::SetParams(const Params& params)
{
  m_params = params;
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_FILTER_ROTATION_H
