#include "random_colormaps.h"

//#undef NO_LOGGING

#include "color_data/colormap_enums.h"
#include "colormaps.h"
#include "goom/goom_graphic.h"
#include "goom/logging.h"
#include "utils/enumutils.h"
#include "utils/math/goom_rand_base.h"

#include <memory>
#include <set>

namespace GOOM::COLOR
{

using COLOR_DATA::ColorMapName;
using UTILS::NUM;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Weights;

auto RandomColorMaps::GetRandomColorMapName() const noexcept -> COLOR_DATA::ColorMapName
{
  return static_cast<ColorMapName>(m_goomRand.GetRandInRange(0U, GetNumColorMapNames()));
}

auto RandomColorMaps::GetRandomColorMapName(const ColorMapGroup colorMapGroup) const noexcept
    -> COLOR_DATA::ColorMapName
{
  const ColorMapNames& colorMapNames = GetColorMapNames(colorMapGroup);
  return colorMapNames[m_goomRand.GetRandInRange(0U, static_cast<uint32_t>(colorMapNames.size()))];
}

auto RandomColorMaps::GetRandomColorMap() const noexcept -> const IColorMap&
{
  return GetColorMap(GetRandomColorMapName());
}

auto RandomColorMaps::GetRandomColorMap(const ColorMapGroup colorMapGroup) const noexcept
    -> const IColorMap&
{
  return GetColorMap(GetRandomColorMapName(colorMapGroup));
}

auto RandomColorMaps::GetRandomColorMapPtr(const std::set<ColorMapTypes>& types) const noexcept
    -> std::shared_ptr<const IColorMap>
{
  return GetRandomColorMapPtr(GetColorMapPtr(GetRandomColorMapName()), types);
}

auto RandomColorMaps::GetRandomColorMapPtr(const COLOR_DATA::ColorMapName colorMapName,
                                           const std::set<ColorMapTypes>& types) const noexcept
    -> std::shared_ptr<const IColorMap>
{
  if (colorMapName == COLOR_DATA::ColorMapName::_NULL)
  {
    return GetRandomColorMapPtr(types);
  }
  return GetRandomColorMapPtr(GetColorMapPtr(colorMapName), types);
}

auto RandomColorMaps::GetRandomColorMapPtr(const ColorMapGroup colorMapGroup,
                                           const std::set<ColorMapTypes>& types) const noexcept
    -> std::shared_ptr<const IColorMap>
{
  return GetRandomColorMapPtr(GetColorMapPtr(GetRandomColorMapName(colorMapGroup)), types);
}

auto RandomColorMaps::GetRandomColorMapPtr(const std::shared_ptr<const IColorMap>& colorMap,
                                           const std::set<ColorMapTypes>& types) const noexcept
    -> std::shared_ptr<const IColorMap>
{
  if (types.empty())
  {
    return colorMap;
  }

  std::shared_ptr<const IColorMap> newColorMap = colorMap;

#if __cplusplus <= 201703L
  if (types.find(ColorMapTypes::ROTATED_T) != types.end())
#else
  if (types.contains(ColorMapTypes::ROTATED_T))
#endif
  {
    newColorMap = GetRandomRotatedColorMapPtr(newColorMap);
  }
#if __cplusplus <= 201703L
  if (types.find(ColorMapTypes::SHADES) != types.end())
#else
  if (types.contains(ColorMapTypes::SHADES))
#endif
  {
    newColorMap = GetRandomTintedColorMapPtr(newColorMap);
  }

  return newColorMap;
}

auto RandomColorMaps::GetRandomRotatedColorMapPtr() const noexcept
    -> std::shared_ptr<const IColorMap>
{
  return GetRotatedColorMapPtr(GetRandomColorMapName(),
                               m_goomRand.GetRandInRange(m_minRotationPoint, m_maxRotationPoint));
}

auto RandomColorMaps::GetRandomRotatedColorMapPtr(
    const COLOR_DATA::ColorMapName colorMapName) const noexcept -> std::shared_ptr<const IColorMap>
{
  return GetRotatedColorMapPtr(colorMapName,
                               m_goomRand.GetRandInRange(m_minRotationPoint, m_maxRotationPoint));
}

auto RandomColorMaps::GetRandomRotatedColorMapPtr(const ColorMapGroup colorMapGroup) const noexcept
    -> std::shared_ptr<const IColorMap>
{
  return GetRotatedColorMapPtr(GetRandomColorMapName(colorMapGroup),
                               m_goomRand.GetRandInRange(m_minRotationPoint, m_maxRotationPoint));
}

auto RandomColorMaps::GetRandomRotatedColorMapPtr(const std::shared_ptr<const IColorMap>& colorMap)
    const noexcept -> std::shared_ptr<const IColorMap>
{
  return GetRotatedColorMapPtr(colorMap,
                               m_goomRand.GetRandInRange(m_minRotationPoint, m_maxRotationPoint));
}

auto RandomColorMaps::GetRandomTintedColorMapPtr() const noexcept
    -> std::shared_ptr<const IColorMap>
{
  return GetTintedColorMapPtr(GetRandomColorMapName(),
                              m_goomRand.GetRandInRange(m_minSaturation, m_maxSaturation),
                              m_goomRand.GetRandInRange(m_minLightness, m_maxLightness));
}

auto RandomColorMaps::GetRandomTintedColorMapPtr(const ColorMapName colorMapName) const noexcept
    -> std::shared_ptr<const IColorMap>
{
  return GetTintedColorMapPtr(colorMapName,
                              m_goomRand.GetRandInRange(m_minSaturation, m_maxSaturation),
                              m_goomRand.GetRandInRange(m_minLightness, m_maxLightness));
}

auto RandomColorMaps::GetRandomTintedColorMapPtr(const ColorMapGroup colorMapGroup) const noexcept
    -> std::shared_ptr<const IColorMap>
{
  return GetTintedColorMapPtr(GetRandomColorMapName(colorMapGroup),
                              m_goomRand.GetRandInRange(m_minSaturation, m_maxSaturation),
                              m_goomRand.GetRandInRange(m_minLightness, m_maxLightness));
}

auto RandomColorMaps::GetRandomTintedColorMapPtr(const std::shared_ptr<const IColorMap>& colorMap)
    const noexcept -> std::shared_ptr<const IColorMap>
{
  return GetTintedColorMapPtr(colorMap, m_goomRand.GetRandInRange(m_minSaturation, m_maxSaturation),
                              m_goomRand.GetRandInRange(m_minLightness, m_maxLightness));
}

auto RandomColorMaps::GetRandomGroup() const noexcept -> ColorMapGroup
{
  return static_cast<ColorMapGroup>(m_goomRand.GetRandInRange(0U, NUM<ColorMapGroup>));
}

auto RandomColorMaps::GetRandomColor(const IColorMap& colorMap,
                                     const float t0,
                                     const float t1) const noexcept -> Pixel
{
  return colorMap.GetColor(m_goomRand.GetRandInRange(t0, t1));
}

auto RandomColorMaps::GetMinRotationPoint() const noexcept -> float
{
  return m_minRotationPoint;
}

auto RandomColorMaps::GetMaxRotationPoint() const noexcept -> float
{
  return m_maxRotationPoint;
}

auto RandomColorMaps::SetRotationPointLimits(const float minRotationPoint,
                                             const float maxRotationPoint) noexcept -> void
{
  Expects(minRotationPoint >= MIN_ROTATION_POINT);
  Expects(maxRotationPoint <= MAX_ROTATION_POINT);
  Expects(minRotationPoint <= maxRotationPoint);

  m_minRotationPoint = minRotationPoint;
  m_maxRotationPoint = maxRotationPoint;
}

auto RandomColorMaps::GetMinSaturation() const noexcept -> float
{
  return m_minSaturation;
}

auto RandomColorMaps::GetMaxSaturation() const noexcept -> float
{
  return m_maxSaturation;
}

auto RandomColorMaps::SetSaturationLimits(const float minSaturation,
                                          const float maxSaturation) noexcept -> void
{
  Expects(minSaturation >= MIN_SATURATION);
  Expects(maxSaturation <= MAX_SATURATION);
  Expects(minSaturation <= maxSaturation);

  m_minSaturation = minSaturation;
  m_maxSaturation = maxSaturation;
}

auto RandomColorMaps::GetMinLightness() const noexcept -> float
{
  return m_minLightness;
}

auto RandomColorMaps::GetMaxLightness() const noexcept -> float
{
  return m_maxLightness;
}

auto RandomColorMaps::SetLightnessLimits(const float minLightness,
                                         const float maxLightness) noexcept -> void
{
  Expects(minLightness >= MIN_LIGHTNESS);
  Expects(maxLightness <= MAX_LIGHTNESS);
  Expects(minLightness <= maxLightness);

  m_minLightness = minLightness;
  m_maxLightness = maxLightness;
}

WeightedColorMaps::WeightedColorMaps(const IGoomRand& goomRand,
                                     const Weights<ColorMapGroup>& weights,
                                     const std::string& colorMapsName) noexcept
  : RandomColorMaps{goomRand, colorMapsName}, m_weights{weights}
{
}

auto WeightedColorMaps::GetRandomGroup() const noexcept -> ColorMapGroup
{
  if (not m_weightsActive)
  {
    return RandomColorMaps::GetRandomGroup();
  }

  return m_weights.GetRandomWeighted();
}

auto WeightedColorMaps::GetRandomColorMapName() const noexcept -> COLOR_DATA::ColorMapName
{
  return RandomColorMaps::GetRandomColorMapName(GetRandomGroup());
}

auto MakeSharedAllMapsUnweighted(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "AllMapsUnweighted";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::ALL, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedAllStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "AllStandardMaps";

  // clang-format off
  static constexpr float PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT = 1.0F;
  static constexpr float SEQUENTIAL_WEIGHT                      = 1.0F;
  static constexpr float SEQUENTIAL2_WEIGHT                     = 1.0F;
  static constexpr float CYCLIC_WEIGHT                          = 1.0F;
  static constexpr float DIVERGING_WEIGHT                       = 1.0F;
  static constexpr float DIVERGING_BLACK_WEIGHT                 = 1.0F;
  static constexpr float QUALITATIVE_WEIGHT                     = 1.0F;
  static constexpr float MISC_WEIGHT                            = 1.0F;

  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL, PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT},
              {ColorMapGroup::SEQUENTIAL,                      SEQUENTIAL_WEIGHT},
              {ColorMapGroup::SEQUENTIAL2,                     SEQUENTIAL2_WEIGHT},
              {ColorMapGroup::CYCLIC,                          CYCLIC_WEIGHT},
              {ColorMapGroup::DIVERGING,                       DIVERGING_WEIGHT},
              {ColorMapGroup::DIVERGING_BLACK,                 DIVERGING_BLACK_WEIGHT},
              {ColorMapGroup::QUALITATIVE,                     QUALITATIVE_WEIGHT},
              {ColorMapGroup::MISC,                            MISC_WEIGHT},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedAllSlimMaps(const IGoomRand& goomRand) noexcept -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "AllSlimMaps";

  // clang-format off
  static constexpr float PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT = 1.0F;
  static constexpr float SEQUENTIAL_SLIM_WEIGHT                      = 1.0F;
  static constexpr float SEQUENTIAL2_SLIM_WEIGHT                     = 1.0F;
  static constexpr float CYCLIC_SLIM_WEIGHT                          = 1.0F;
  static constexpr float DIVERGING_SLIM_WEIGHT                       = 1.0F;
  static constexpr float DIVERGING_BLACK_SLIM_WEIGHT                 = 1.0F;
  static constexpr float QUALITATIVE_SLIM_WEIGHT                     = 1.0F;
  static constexpr float MISC_SLIM_WEIGHT                            = 1.0F;

  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM, PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT},
              {ColorMapGroup::SEQUENTIAL_SLIM,                      SEQUENTIAL_SLIM_WEIGHT},
              {ColorMapGroup::SEQUENTIAL2_SLIM,                     SEQUENTIAL2_SLIM_WEIGHT},
              {ColorMapGroup::CYCLIC_SLIM,                          CYCLIC_SLIM_WEIGHT},
              {ColorMapGroup::DIVERGING_SLIM,                       DIVERGING_SLIM_WEIGHT},
              {ColorMapGroup::DIVERGING_BLACK_SLIM,                 DIVERGING_BLACK_SLIM_WEIGHT},
              {ColorMapGroup::QUALITATIVE_SLIM,                     QUALITATIVE_SLIM_WEIGHT},
              {ColorMapGroup::MISC_SLIM,                            MISC_SLIM_WEIGHT},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedMostlySequentialStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "MostlySequentialStandardMaps";

  // clang-format off
  static constexpr float PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT = 1.0F;
  static constexpr float SEQUENTIAL_WEIGHT                      = 1.0F;
  static constexpr float SEQUENTIAL2_WEIGHT                     = 1.0F;
  static constexpr float CYCLIC_WEIGHT                          = 0.0F;
  static constexpr float DIVERGING_WEIGHT                       = 0.0F;
  static constexpr float DIVERGING_BLACK_WEIGHT                 = 0.0F;
  static constexpr float QUALITATIVE_WEIGHT                     = 1.0F;
  static constexpr float MISC_WEIGHT                            = 1.0F;

  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL, PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT},
              {ColorMapGroup::SEQUENTIAL,                      SEQUENTIAL_WEIGHT},
              {ColorMapGroup::SEQUENTIAL2,                     SEQUENTIAL2_WEIGHT},
              {ColorMapGroup::CYCLIC,                          CYCLIC_WEIGHT},
              {ColorMapGroup::DIVERGING,                       DIVERGING_WEIGHT},
              {ColorMapGroup::DIVERGING_BLACK,                 DIVERGING_BLACK_WEIGHT},
              {ColorMapGroup::QUALITATIVE,                     QUALITATIVE_WEIGHT},
              {ColorMapGroup::MISC,                            MISC_WEIGHT},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedMostlySequentialSlimMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "MostlySequentialSlimMaps";

  // clang-format off
  static constexpr float PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT = 1.0F;
  static constexpr float SEQUENTIAL_SLIM_WEIGHT                      = 1.0F;
  static constexpr float SEQUENTIAL2_SLIM_WEIGHT                     = 1.0F;
  static constexpr float CYCLIC_SLIM_WEIGHT                          = 0.0F;
  static constexpr float DIVERGING_SLIM_WEIGHT                       = 0.0F;
  static constexpr float DIVERGING_BLACK_SLIM_WEIGHT                 = 0.0F;
  static constexpr float QUALITATIVE_SLIM_WEIGHT                     = 1.0F;
  static constexpr float MISC_SLIM_WEIGHT                            = 1.0F;

  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM, PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT},
              {ColorMapGroup::SEQUENTIAL_SLIM,                      SEQUENTIAL_SLIM_WEIGHT},
              {ColorMapGroup::SEQUENTIAL2_SLIM,                     SEQUENTIAL2_SLIM_WEIGHT},
              {ColorMapGroup::CYCLIC_SLIM,                          CYCLIC_SLIM_WEIGHT},
              {ColorMapGroup::DIVERGING_SLIM,                       DIVERGING_SLIM_WEIGHT},
              {ColorMapGroup::DIVERGING_BLACK_SLIM,                 DIVERGING_BLACK_SLIM_WEIGHT},
              {ColorMapGroup::QUALITATIVE_SLIM,                     QUALITATIVE_SLIM_WEIGHT},
              {ColorMapGroup::MISC_SLIM,                            MISC_SLIM_WEIGHT},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedSlightlyDivergingStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "SlightlyDivergingStandardMaps";

  // clang-format off
  static constexpr float PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT = 10.0F;
  static constexpr float SEQUENTIAL_WEIGHT                      = 10.0F;
  static constexpr float SEQUENTIAL2_WEIGHT                     = 10.0F;
  static constexpr float CYCLIC_WEIGHT                          = 10.0F;
  static constexpr float DIVERGING_WEIGHT                       = 20.0F;
  static constexpr float DIVERGING_BLACK_WEIGHT                 =  1.0F;
  static constexpr float QUALITATIVE_WEIGHT                     = 10.0F;
  static constexpr float MISC_WEIGHT                            = 20.0F;

  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL, PERCEPTUALLY_UNIFORM_SEQUENTIAL_WEIGHT},
              {ColorMapGroup::SEQUENTIAL,                      SEQUENTIAL_WEIGHT},
              {ColorMapGroup::SEQUENTIAL2,                     SEQUENTIAL2_WEIGHT},
              {ColorMapGroup::CYCLIC,                          CYCLIC_WEIGHT},
              {ColorMapGroup::DIVERGING,                       DIVERGING_WEIGHT},
              {ColorMapGroup::DIVERGING_BLACK,                 DIVERGING_BLACK_WEIGHT},
              {ColorMapGroup::QUALITATIVE,                     QUALITATIVE_WEIGHT},
              {ColorMapGroup::MISC,                            MISC_WEIGHT},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedSlightlyDivergingSlimMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "SlightlyDivergingSlimMaps";

  // clang-format off
  static constexpr float PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT = 10.0F;
  static constexpr float SEQUENTIAL_SLIM_WEIGHT                      = 10.0F;
  static constexpr float SEQUENTIAL2_SLIM_WEIGHT                     = 10.0F;
  static constexpr float CYCLIC_SLIM_WEIGHT                          = 10.0F;
  static constexpr float DIVERGING_SLIM_WEIGHT                       = 20.0F;
  static constexpr float DIVERGING_BLACK_SLIM_WEIGHT                 =  1.0F;
  static constexpr float QUALITATIVE_SLIM_WEIGHT                     = 10.0F;
  static constexpr float MISC_SLIM_WEIGHT                            = 20.0F;

  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM, PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM_WEIGHT},
              {ColorMapGroup::SEQUENTIAL_SLIM,                      SEQUENTIAL_SLIM_WEIGHT},
              {ColorMapGroup::SEQUENTIAL2_SLIM,                     SEQUENTIAL2_SLIM_WEIGHT},
              {ColorMapGroup::CYCLIC_SLIM,                          CYCLIC_SLIM_WEIGHT},
              {ColorMapGroup::DIVERGING_SLIM,                       DIVERGING_SLIM_WEIGHT},
              {ColorMapGroup::DIVERGING_BLACK_SLIM,                 DIVERGING_BLACK_SLIM_WEIGHT},
              {ColorMapGroup::QUALITATIVE_SLIM,                     QUALITATIVE_SLIM_WEIGHT},
              {ColorMapGroup::MISC_SLIM,                            MISC_SLIM_WEIGHT},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedBlueStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "BlueStandardMaps";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::BLUES, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedRedStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "RedStandardMaps";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::REDS, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedGreenStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "GreenStandardMaps";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::GREENS, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedYellowStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "YellowStandardMaps";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::YELLOWS, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedOrangeStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "OrangeStandardMaps";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::ORANGES, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedPurpleStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "PurpleStandardMaps";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PURPLES, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedCitiesStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "CitiesStandardMaps";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::CITIES, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedSeasonsStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "SeasonsStandardMaps";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::SEASONS, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedHeatStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "HeatStandardMaps";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::HEAT, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedColdStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "ColdStandardMaps";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::COLD, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedPastelStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "PastelStandardMaps";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PASTEL, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedDivergingBlackStandardMaps(const IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "DivergingBlackStandardMaps";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::DIVERGING_BLACK, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

auto MakeSharedWesAndersonMaps(const UTILS::MATH::IGoomRand& goomRand) noexcept
    -> std::shared_ptr<RandomColorMaps>
{
  static constexpr const char* MAPS_NAME = "WesAndersonMaps";

  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::WES_ANDERSON, 1.0F},
          }
      },
      MAPS_NAME
  );
  // clang-format on
}

} // namespace GOOM::COLOR
