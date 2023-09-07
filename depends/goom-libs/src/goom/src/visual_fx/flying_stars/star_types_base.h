#pragma once

#include "goom/point2d.h"

namespace GOOM::VISUAL_FX::FLYING_STARS
{

class StarColorsMaker;

class IStarType
{
public:
  IStarType() noexcept                                    = default;
  IStarType(const IStarType&) noexcept                    = delete;
  IStarType(IStarType&&) noexcept                         = delete;
  virtual ~IStarType() noexcept                           = default;
  auto operator=(const IStarType&) noexcept -> IStarType& = delete;
  auto operator=(IStarType&&) noexcept -> IStarType&      = delete;

  struct SetupParams
  {
    Point2dInt startPos;
    float gravity{};
    float sideWind{};
    float nominalPathLength{};
    float starTAgeInc{};
  };
  [[nodiscard]] virtual auto GetRandomizedSetupParams(float defaultPathLength) const noexcept
      -> SetupParams = 0;
  [[nodiscard]] virtual auto GetRandomizedStarPathAngle(const Point2dInt& startPos) const noexcept
      -> float = 0;

  enum class ColorMapMode
  {
    ONE_MAP_PER_ANGLE,
    ONE_MAP_FOR_ALL_ANGLES,
    ALL_MAPS_RANDOM,
    _num // unused, and marks the enum end
  };

  [[nodiscard]] virtual auto GetStarColorsMaker() const noexcept -> const StarColorsMaker& = 0;
  virtual auto UpdateFixedColorMapNames() noexcept -> void                                 = 0;
  virtual auto UpdateWindAndGravity() noexcept -> void                                     = 0;
};

} //namespace GOOM::VISUAL_FX::FLYING_STARS
