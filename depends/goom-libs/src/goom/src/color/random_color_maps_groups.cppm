module;

export module Goom.Color.RandomColorMapsGroups;

import Goom.Color.RandomColorMaps;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;

export namespace GOOM::COLOR
{

class RandomColorMapsGroups
{
public:
  explicit RandomColorMapsGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  enum class Groups : UnderlyingEnumType
  {
    ALL_MAPS_UNWEIGHTED,
    ALL_STANDARD_MAPS,
    ALL_SLIM_MAPS,
    MOSTLY_SEQUENTIAL_STANDARD_MAPS,
    MOSTLY_SEQUENTIAL_SLIM_MAPS,
    SLIGHTLY_DIVERGING_STANDARD_MAPS,
    SLIGHTLY_DIVERGING_SLIM_MAPS,
    DIVERGING_BLACK_STANDARD_MAPS,
    RED_STANDARD_MAPS,
    GREEN_STANDARD_MAPS,
    BLUE_STANDARD_MAPS,
    YELLOW_STANDARD_MAPS,
    ORANGE_STANDARD_MAPS,
    PURPLE_STANDARD_MAPS,
    CITIES_STANDARD_MAPS,
    SEASONS_STANDARD_MAPS,
    HEAT_STANDARD_MAPS,
    COLD_STANDARD_MAPS,
    PASTEL_STANDARD_MAPS,
    WES_ANDERSON_MAPS,
  };

  [[nodiscard]] auto GetWeightedRandomColorMapsForGroup(Groups randomColorMapsGroup) const noexcept
      -> WeightedRandomColorMaps;

  [[nodiscard]] static auto GetUnweightedRandomColorMaps(
      const UTILS::MATH::IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
};

[[nodiscard]] inline auto GetUnweightedRandomColorMaps(const UTILS::MATH::IGoomRand& goomRand,
                                                       const PixelChannelType defaultAlpha) noexcept
    -> WeightedRandomColorMaps
{
  return {RandomColorMapsGroups::GetUnweightedRandomColorMaps(goomRand), defaultAlpha};
}

inline auto RandomColorMapsGroups::GetUnweightedRandomColorMaps(
    const UTILS::MATH::IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  return RandomColorMapsGroups{goomRand}.GetWeightedRandomColorMapsForGroup(
      Groups::ALL_MAPS_UNWEIGHTED);
}

} // namespace GOOM::COLOR
