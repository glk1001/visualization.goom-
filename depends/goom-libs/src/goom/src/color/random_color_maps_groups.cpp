module Goom.Color.RandomColorMapsGroups;

import Goom.Color.ColorMaps;
import Goom.Color.RandomColorMaps;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomGraphic;

namespace GOOM::COLOR
{

using COLOR::ColorMapGroup;
using COLOR::WeightedRandomColorMaps;
using UTILS::MATH::GoomRand;
using UTILS::MATH::Weights;

namespace
{

constexpr auto DEFAULT_ALPHA = MAX_ALPHA;

namespace IMPL
{

auto GetAllMapsUnweighted(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "AllMapsUnweighted";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::ALL, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetAllStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
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
      Weights<ColorMapGroup>{
                             goomRand, {
              {.key    = ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL,
               .weight = PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT},
              {.key = ColorMapGroup::SEQUENTIAL, .weight = SEQUENTIAL_WEIGHT},
              {.key = ColorMapGroup::SEQUENTIAL2, .weight = SEQUENTIAL2_WEIGHT},
              {.key = ColorMapGroup::CYCLIC, .weight = CYCLIC_WEIGHT},
              {.key = ColorMapGroup::DIVERGING, .weight = DIVERGING_WEIGHT},
              {.key = ColorMapGroup::DIVERGING_BLACK, .weight = DIVERGING_BLACK_WEIGHT},
              {.key = ColorMapGroup::QUALITATIVE, .weight = QUALITATIVE_WEIGHT},
              {.key = ColorMapGroup::MISC, .weight = MISC_WEIGHT},
          }},
      MAPS_NAME
  };
}

auto GetAllSlimMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
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
      Weights<ColorMapGroup>{
                             goomRand, {
              {.key    = ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM,
               .weight = PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT},
              {.key = ColorMapGroup::SEQUENTIAL_SLIM, .weight = SEQUENTIAL_SLIM_WEIGHT},
              {.key = ColorMapGroup::SEQUENTIAL2_SLIM, .weight = SEQUENTIAL2_SLIM_WEIGHT},
              {.key = ColorMapGroup::CYCLIC_SLIM, .weight = CYCLIC_SLIM_WEIGHT},
              {.key = ColorMapGroup::DIVERGING_SLIM, .weight = DIVERGING_SLIM_WEIGHT},
              {.key = ColorMapGroup::DIVERGING_BLACK_SLIM, .weight = DIVERGING_BLACK_SLIM_WEIGHT},
              {.key = ColorMapGroup::QUALITATIVE_SLIM, .weight = QUALITATIVE_SLIM_WEIGHT},
              {.key = ColorMapGroup::MISC_SLIM, .weight = MISC_SLIM_WEIGHT},
          }},
      MAPS_NAME
  };
}

auto GetMostlySequentialStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
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
      Weights<ColorMapGroup>{
                             goomRand, {
              {.key    = ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL,
               .weight = PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT},
              {.key = ColorMapGroup::SEQUENTIAL, .weight = SEQUENTIAL_WEIGHT},
              {.key = ColorMapGroup::SEQUENTIAL2, .weight = SEQUENTIAL2_WEIGHT},
              {.key = ColorMapGroup::CYCLIC, .weight = CYCLIC_WEIGHT},
              {.key = ColorMapGroup::DIVERGING, .weight = DIVERGING_WEIGHT},
              {.key = ColorMapGroup::DIVERGING_BLACK, .weight = DIVERGING_BLACK_WEIGHT},
              {.key = ColorMapGroup::QUALITATIVE, .weight = QUALITATIVE_WEIGHT},
              {.key = ColorMapGroup::MISC, .weight = MISC_WEIGHT},
          }},
      MAPS_NAME
  };
}

auto GetMostlySequentialSlimMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
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
      Weights<ColorMapGroup>{
                             goomRand, {
              {.key    = ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM,
               .weight = PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT},
              {.key = ColorMapGroup::SEQUENTIAL_SLIM, .weight = SEQUENTIAL_SLIM_WEIGHT},
              {.key = ColorMapGroup::SEQUENTIAL2_SLIM, .weight = SEQUENTIAL2_SLIM_WEIGHT},
              {.key = ColorMapGroup::CYCLIC_SLIM, .weight = CYCLIC_SLIM_WEIGHT},
              {.key = ColorMapGroup::DIVERGING_SLIM, .weight = DIVERGING_SLIM_WEIGHT},
              {.key = ColorMapGroup::DIVERGING_BLACK_SLIM, .weight = DIVERGING_BLACK_SLIM_WEIGHT},
              {.key = ColorMapGroup::QUALITATIVE_SLIM, .weight = QUALITATIVE_SLIM_WEIGHT},
              {.key = ColorMapGroup::MISC_SLIM, .weight = MISC_SLIM_WEIGHT},
          }},
      MAPS_NAME
  };
}

auto GetSlightlyDivergingStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
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
      Weights<ColorMapGroup>{
                             goomRand, {
              {.key    = ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL,
               .weight = PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT},
              {.key = ColorMapGroup::SEQUENTIAL, .weight = SEQUENTIAL_WEIGHT},
              {.key = ColorMapGroup::SEQUENTIAL2, .weight = SEQUENTIAL2_WEIGHT},
              {.key = ColorMapGroup::CYCLIC, .weight = CYCLIC_WEIGHT},
              {.key = ColorMapGroup::DIVERGING, .weight = DIVERGING_WEIGHT},
              {.key = ColorMapGroup::DIVERGING_BLACK, .weight = DIVERGING_BLACK_WEIGHT},
              {.key = ColorMapGroup::QUALITATIVE, .weight = QUALITATIVE_WEIGHT},
              {.key = ColorMapGroup::MISC, .weight = MISC_WEIGHT},
          }},
      MAPS_NAME
  };
}

auto GetSlightlyDivergingSlimMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
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
      Weights<ColorMapGroup>{
                             goomRand, {
              {.key    = ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM,
               .weight = PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT},
              {.key = ColorMapGroup::SEQUENTIAL_SLIM, .weight = SEQUENTIAL_SLIM_WEIGHT},
              {.key = ColorMapGroup::SEQUENTIAL2_SLIM, .weight = SEQUENTIAL2_SLIM_WEIGHT},
              {.key = ColorMapGroup::CYCLIC_SLIM, .weight = CYCLIC_SLIM_WEIGHT},
              {.key = ColorMapGroup::DIVERGING_SLIM, .weight = DIVERGING_SLIM_WEIGHT},
              {.key = ColorMapGroup::DIVERGING_BLACK_SLIM, .weight = DIVERGING_BLACK_SLIM_WEIGHT},
              {.key = ColorMapGroup::QUALITATIVE_SLIM, .weight = QUALITATIVE_SLIM_WEIGHT},
              {.key = ColorMapGroup::MISC_SLIM, .weight = MISC_SLIM_WEIGHT},
          }},
      MAPS_NAME
  };
}

auto GetBlueStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "BlueStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::BLUES, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetRedStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "RedStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::REDS, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetGreenStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "GreenStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::GREENS, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetYellowStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "YellowStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::YELLOWS, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetOrangeStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "OrangeStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::ORANGES, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetPurpleStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "PurpleStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::PURPLES, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetCitiesStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "CitiesStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::CITIES, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetSeasonsStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "SeasonsStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::SEASONS, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetHeatStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "HeatStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::HEAT, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetColdStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "ColdStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::COLD, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetPastelStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "PastelStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::PASTEL, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetDivergingBlackStandardMaps(const GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "DivergingBlackStandardMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::DIVERGING_BLACK, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

auto GetWesAndersonMaps(const UTILS::MATH::GoomRand& goomRand) noexcept -> WeightedRandomColorMaps
{
  static constexpr auto* MAPS_NAME = "WesAndersonMaps";

  return WeightedRandomColorMaps{
      DEFAULT_ALPHA,
      goomRand,
      Weights<ColorMapGroup>{goomRand,
                             {
                                 {.key = ColorMapGroup::WES_ANDERSON, .weight = 1.0F},
                             }},
      MAPS_NAME
  };
}

} // namespace IMPL
} // namespace

RandomColorMapsGroups::RandomColorMapsGroups(const GoomRand& goomRand) noexcept
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
