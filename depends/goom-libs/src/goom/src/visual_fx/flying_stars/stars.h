#pragma once

#include "goom_config.h"
#include "point2d.h"
#include "star_colors.h"

namespace GOOM::VISUAL_FX::FLYING_STARS
{

class Star
{
public:
  struct Params
  {
    Point2dFlt currentPosition;
    Vec2dFlt velocity;
    Vec2dFlt acceleration;
    float tAge{};
    float tAgeInc{};
  };

  Star(const Params& params, const StarColors& starColors) noexcept;

  [[nodiscard]] auto GetStartPos() const noexcept -> Point2dFlt;
  [[nodiscard]] auto GetTAge() const noexcept -> float;
  [[nodiscard]] auto GetVelocity() const noexcept -> Vec2dFlt;
  [[nodiscard]] auto GetStarColors() const noexcept -> const StarColors&;

  [[nodiscard]] auto IsTooOld() const noexcept -> bool;

  auto Update() noexcept -> void;

private:
  Params m_params;
  StarColors m_starColors;
};

inline auto Star::GetStartPos() const noexcept -> Point2dFlt
{
  return m_params.currentPosition;
}

inline auto Star::GetTAge() const noexcept -> float
{
  return m_params.tAge;
}

inline auto Star::GetVelocity() const noexcept -> Vec2dFlt
{
  return m_params.velocity;
}

inline auto Star::GetStarColors() const noexcept -> const StarColors&
{
  return m_starColors;
}

inline auto Star::IsTooOld() const noexcept -> bool
{
  return m_params.tAge >= 1.0F;
}

} //namespace GOOM::VISUAL_FX::FLYING_STARS
