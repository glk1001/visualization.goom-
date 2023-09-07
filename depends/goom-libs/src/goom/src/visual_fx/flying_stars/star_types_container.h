#pragma once

#include "goom/goom_config.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "star_types_base.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace GOOM
{
class PluginInfo;

namespace COLOR
{
class WeightedRandomColorMaps;
}
}

namespace GOOM::VISUAL_FX::FLYING_STARS
{

class StarType;

class StarTypesContainer
{
public:
  StarTypesContainer(const PluginInfo& goomInfo, const UTILS::MATH::IGoomRand& goomRand) noexcept;
  StarTypesContainer(const StarTypesContainer&) noexcept = delete;
  StarTypesContainer(StarTypesContainer&&) noexcept      = delete;
  ~StarTypesContainer() noexcept;
  auto operator=(const StarTypesContainer&) noexcept -> StarTypesContainer& = delete;
  auto operator=(StarTypesContainer&&) noexcept -> StarTypesContainer&      = delete;

  [[nodiscard]] auto GetRandomStarType() noexcept -> IStarType&;

  auto SetWeightedColorMaps(uint32_t starTypeId,
                            const COLOR::WeightedRandomColorMaps& weightedMainColorMaps,
                            const COLOR::WeightedRandomColorMaps& weightedLowColorMaps) noexcept
      -> void;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;

  auto ChangeColorMode() noexcept -> void;
  auto SetColorMapMode(IStarType::ColorMapMode colorMapMode) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  static const auto NUM_STAR_TYPES = 3U;

private:
  enum class AvailableStarTypes : UnderlyingEnumType
  {
    FIREWORKS,
    RAIN,
    FOUNTAIN,
    _num // unused, and marks the enum end
  };
  static_assert(NUM_STAR_TYPES == UTILS::NUM<AvailableStarTypes>);
  std::array<std::unique_ptr<StarType>, NUM_STAR_TYPES> m_starTypesList;
  static constexpr float STAR_TYPES_FIREWORKS_WEIGHT = 10.0F;
  static constexpr float STAR_TYPES_FOUNTAIN_WEIGHT  = 07.0F;
  static constexpr float STAR_TYPES_RAIN_WEIGHT      = 07.0F;
  UTILS::MATH::Weights<AvailableStarTypes> m_weightedStarTypes;
};

} //namespace GOOM::VISUAL_FX::FLYING_STARS
