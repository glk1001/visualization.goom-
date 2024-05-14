#include "random_color_maps_groups.h"

#include "color_maps.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "random_color_maps.h"

import Goom.Utils.Math.GoomRandBase;

namespace GOOM::COLOR
{

using COLOR::ColorMapGroup;
using COLOR::WeightedRandomColorMaps;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Weights;

namespace
{

constexpr auto DEFAULT_ALPHA = MAX_ALPHA;

namespace IMPL
{

auto GetAllMapsUnweighted(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "AllMapsUnweighted";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::ALL, 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetAllStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "AllStandardMaps";

  static constexpr auto PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT = 1.0F;
  static constexpr auto SEQUENTIAL_WEIGHT                      = 1.0F;
  static constexpr auto SEQUENTIAL2_WEIGHT                     = 1.0F;
  static constexpr auto CYCLIC_WEIGHT                          = 1.0F;
  static constexpr auto DIVERGING_WEIGHT                       = 1.0F;
  static constexpr auto DIVERGING_BLACK_WEIGHT                 = 1.0F;
  static constexpr auto QUALITATIVE_WEIGHT                     = 1.0F;
  static constexpr auto MISC_WEIGHT                            = 1.0F;

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL,
                             PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT},
                             {ColorMapGroup::SEQUENTIAL, SEQUENTIAL_WEIGHT},
                             {ColorMapGroup::SEQUENTIAL2, SEQUENTIAL2_WEIGHT},
                             {ColorMapGroup::CYCLIC, CYCLIC_WEIGHT},
                             {ColorMapGroup::DIVERGING, DIVERGING_WEIGHT},
                             {ColorMapGroup::DIVERGING_BLACK, DIVERGING_BLACK_WEIGHT},
                             {ColorMapGroup::QUALITATIVE, QUALITATIVE_WEIGHT},
                             {ColorMapGroup::MISC, MISC_WEIGHT},
                             }},
      MAPS_NAME
  };
}

auto GetAllSlimMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "AllSlimMaps";

  static constexpr auto PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT = 1.0F;
  static constexpr auto SEQUENTIAL_SLIM_WEIGHT                      = 1.0F;
  static constexpr auto SEQUENTIAL2_SLIM_WEIGHT                     = 1.0F;
  static constexpr auto CYCLIC_SLIM_WEIGHT                          = 1.0F;
  static constexpr auto DIVERGING_SLIM_WEIGHT                       = 1.0F;
  static constexpr auto DIVERGING_BLACK_SLIM_WEIGHT                 = 1.0F;
  static constexpr auto QUALITATIVE_SLIM_WEIGHT                     = 1.0F;
  static constexpr auto MISC_SLIM_WEIGHT                            = 1.0F;

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM,
                             PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT},
                             {ColorMapGroup::SEQUENTIAL_SLIM, SEQUENTIAL_SLIM_WEIGHT},
                             {ColorMapGroup::SEQUENTIAL2_SLIM, SEQUENTIAL2_SLIM_WEIGHT},
                             {ColorMapGroup::CYCLIC_SLIM, CYCLIC_SLIM_WEIGHT},
                             {ColorMapGroup::DIVERGING_SLIM, DIVERGING_SLIM_WEIGHT},
                             {ColorMapGroup::DIVERGING_BLACK_SLIM, DIVERGING_BLACK_SLIM_WEIGHT},
                             {ColorMapGroup::QUALITATIVE_SLIM, QUALITATIVE_SLIM_WEIGHT},
                             {ColorMapGroup::MISC_SLIM, MISC_SLIM_WEIGHT},
                             }},
      MAPS_NAME
  };
}

auto GetMostlySequentialStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "MostlySequentialStandardMaps";

  static constexpr auto PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT = 1.0F;
  static constexpr auto SEQUENTIAL_WEIGHT                      = 1.0F;
  static constexpr auto SEQUENTIAL2_WEIGHT                     = 1.0F;
  static constexpr auto CYCLIC_WEIGHT                          = 0.0F;
  static constexpr auto DIVERGING_WEIGHT                       = 0.0F;
  static constexpr auto DIVERGING_BLACK_WEIGHT                 = 0.0F;
  static constexpr auto QUALITATIVE_WEIGHT                     = 1.0F;
  static constexpr auto MISC_WEIGHT                            = 1.0F;

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL,
                             PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT},
                             {ColorMapGroup::SEQUENTIAL, SEQUENTIAL_WEIGHT},
                             {ColorMapGroup::SEQUENTIAL2, SEQUENTIAL2_WEIGHT},
                             {ColorMapGroup::CYCLIC, CYCLIC_WEIGHT},
                             {ColorMapGroup::DIVERGING, DIVERGING_WEIGHT},
                             {ColorMapGroup::DIVERGING_BLACK, DIVERGING_BLACK_WEIGHT},
                             {ColorMapGroup::QUALITATIVE, QUALITATIVE_WEIGHT},
                             {ColorMapGroup::MISC, MISC_WEIGHT},
                             }},
      MAPS_NAME
  };
}

auto GetMostlySequentialSlimMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "MostlySequentialSlimMaps";

  static constexpr auto PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT = 1.0F;
  static constexpr auto SEQUENTIAL_SLIM_WEIGHT                      = 1.0F;
  static constexpr auto SEQUENTIAL2_SLIM_WEIGHT                     = 1.0F;
  static constexpr auto CYCLIC_SLIM_WEIGHT                          = 0.0F;
  static constexpr auto DIVERGING_SLIM_WEIGHT                       = 0.0F;
  static constexpr auto DIVERGING_BLACK_SLIM_WEIGHT                 = 0.0F;
  static constexpr auto QUALITATIVE_SLIM_WEIGHT                     = 1.0F;
  static constexpr auto MISC_SLIM_WEIGHT                            = 1.0F;

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM,
                             PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT},
                             {ColorMapGroup::SEQUENTIAL_SLIM, SEQUENTIAL_SLIM_WEIGHT},
                             {ColorMapGroup::SEQUENTIAL2_SLIM, SEQUENTIAL2_SLIM_WEIGHT},
                             {ColorMapGroup::CYCLIC_SLIM, CYCLIC_SLIM_WEIGHT},
                             {ColorMapGroup::DIVERGING_SLIM, DIVERGING_SLIM_WEIGHT},
                             {ColorMapGroup::DIVERGING_BLACK_SLIM, DIVERGING_BLACK_SLIM_WEIGHT},
                             {ColorMapGroup::QUALITATIVE_SLIM, QUALITATIVE_SLIM_WEIGHT},
                             {ColorMapGroup::MISC_SLIM, MISC_SLIM_WEIGHT},
                             }},
      MAPS_NAME
  };
}

auto GetSlightlyDivergingStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "SlightlyDivergingStandardMaps";

  static constexpr auto PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT = 10.0F;
  static constexpr auto SEQUENTIAL_WEIGHT                      = 10.0F;
  static constexpr auto SEQUENTIAL2_WEIGHT                     = 10.0F;
  static constexpr auto CYCLIC_WEIGHT                          = 10.0F;
  static constexpr auto DIVERGING_WEIGHT                       = 20.0F;
  static constexpr auto DIVERGING_BLACK_WEIGHT                 = 01.0F;
  static constexpr auto QUALITATIVE_WEIGHT                     = 10.0F;
  static constexpr auto MISC_WEIGHT                            = 20.0F;

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL,
                             PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT},
                             {ColorMapGroup::SEQUENTIAL, SEQUENTIAL_WEIGHT},
                             {ColorMapGroup::SEQUENTIAL2, SEQUENTIAL2_WEIGHT},
                             {ColorMapGroup::CYCLIC, CYCLIC_WEIGHT},
                             {ColorMapGroup::DIVERGING, DIVERGING_WEIGHT},
                             {ColorMapGroup::DIVERGING_BLACK, DIVERGING_BLACK_WEIGHT},
                             {ColorMapGroup::QUALITATIVE, QUALITATIVE_WEIGHT},
                             {ColorMapGroup::MISC, MISC_WEIGHT},
                             }},
      MAPS_NAME
  };
}

auto GetSlightlyDivergingSlimMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "SlightlyDivergingSlimMaps";

  static constexpr auto PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT = 10.0F;
  static constexpr auto SEQUENTIAL_SLIM_WEIGHT                      = 10.0F;
  static constexpr auto SEQUENTIAL2_SLIM_WEIGHT                     = 10.0F;
  static constexpr auto CYCLIC_SLIM_WEIGHT                          = 10.0F;
  static constexpr auto DIVERGING_SLIM_WEIGHT                       = 20.0F;
  static constexpr auto DIVERGING_BLACK_SLIM_WEIGHT                 = 01.0F;
  static constexpr auto QUALITATIVE_SLIM_WEIGHT                     = 10.0F;
  static constexpr auto MISC_SLIM_WEIGHT                            = 20.0F;

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM,
                             PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT},
                             {ColorMapGroup::SEQUENTIAL_SLIM, SEQUENTIAL_SLIM_WEIGHT},
                             {ColorMapGroup::SEQUENTIAL2_SLIM, SEQUENTIAL2_SLIM_WEIGHT},
                             {ColorMapGroup::CYCLIC_SLIM, CYCLIC_SLIM_WEIGHT},
                             {ColorMapGroup::DIVERGING_SLIM, DIVERGING_SLIM_WEIGHT},
                             {ColorMapGroup::DIVERGING_BLACK_SLIM, DIVERGING_BLACK_SLIM_WEIGHT},
                             {ColorMapGroup::QUALITATIVE_SLIM, QUALITATIVE_SLIM_WEIGHT},
                             {ColorMapGroup::MISC_SLIM, MISC_SLIM_WEIGHT},
                             }},
      MAPS_NAME
  };
}

auto GetBlueStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "BlueStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::BLUES, 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetRedStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "RedStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::REDS, 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetGreenStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "GreenStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::GREENS, 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetYellowStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "YellowStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::YELLOWS, 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetOrangeStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "OrangeStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::ORANGES, 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetPurpleStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "PurpleStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::PURPLES, 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetCitiesStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "CitiesStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::CITIES, 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetSeasonsStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "SeasonsStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::SEASONS, 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetHeatStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "HeatStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::HEAT, 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetColdStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "ColdStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::COLD, 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetPastelStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "PastelStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::PASTEL, 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetDivergingBlackStandardMaps(const IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "DivergingBlackStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::DIVERGING_BLACK, 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetWesAndersonMaps(const UTILS::MATH::IGoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "WesAndersonMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                             {ColorMapGroup::WES_ANDERSON, 1.0F},
                             }},
      MAPS_NAME
  };
}

} // namespace IMPL
} // namespace

RandomColorMapsGroups::RandomColorMapsGroups(const IGoomRand& goomRand) noexcept
  : m_goomRand{&goomRand}
{
}

auto RandomColorMapsGroups::GetWeightedRandomColorMapsForGroup(
    const Groups randomColorMapsGroup) const noexcept -> WeightedRandomColorMaps
{
  switch (randomColorMapsGroup)
  {
    case Groups::ALL_MAPS_UNWEIGHTED:
      return IMPL::GetAllMapsUnweighted(*m_goomRand);
    case Groups::ALL_STANDARD_MAPS:
      return IMPL::GetAllStandardMaps(*m_goomRand);
    case Groups::ALL_SLIM_MAPS:
      return IMPL::GetAllSlimMaps(*m_goomRand);
    case Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS:
      return IMPL::GetMostlySequentialStandardMaps(*m_goomRand);
    case Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS:
      return IMPL::GetMostlySequentialSlimMaps(*m_goomRand);
    case Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS:
      return IMPL::GetSlightlyDivergingStandardMaps(*m_goomRand);
    case Groups::SLIGHTLY_DIVERGING_SLIM_MAPS:
      return IMPL::GetSlightlyDivergingSlimMaps(*m_goomRand);
    case Groups::DIVERGING_BLACK_STANDARD_MAPS:
      return IMPL::GetDivergingBlackStandardMaps(*m_goomRand);
    case Groups::RED_STANDARD_MAPS:
      return IMPL::GetRedStandardMaps(*m_goomRand);
    case Groups::GREEN_STANDARD_MAPS:
      return IMPL::GetGreenStandardMaps(*m_goomRand);
    case Groups::BLUE_STANDARD_MAPS:
      return IMPL::GetBlueStandardMaps(*m_goomRand);
    case Groups::YELLOW_STANDARD_MAPS:
      return IMPL::GetYellowStandardMaps(*m_goomRand);
    case Groups::ORANGE_STANDARD_MAPS:
      return IMPL::GetOrangeStandardMaps(*m_goomRand);
    case Groups::PURPLE_STANDARD_MAPS:
      return IMPL::GetPurpleStandardMaps(*m_goomRand);
    case Groups::CITIES_STANDARD_MAPS:
      return IMPL::GetCitiesStandardMaps(*m_goomRand);
    case Groups::SEASONS_STANDARD_MAPS:
      return IMPL::GetSeasonsStandardMaps(*m_goomRand);
    case Groups::HEAT_STANDARD_MAPS:
      return IMPL::GetHeatStandardMaps(*m_goomRand);
    case Groups::COLD_STANDARD_MAPS:
      return IMPL::GetColdStandardMaps(*m_goomRand);
    case Groups::PASTEL_STANDARD_MAPS:
      return IMPL::GetPastelStandardMaps(*m_goomRand);
    case Groups::WES_ANDERSON_MAPS:
      return IMPL::GetWesAndersonMaps(*m_goomRand);
  }
}

} // namespace GOOM::COLOR
