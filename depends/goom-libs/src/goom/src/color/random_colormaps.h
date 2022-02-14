#pragma once

#include "color/color_data/colormap_enums.h"
#include "color/colormaps.h"
#include "goom/goom_graphic.h"
#include "utils/goom_rand_base.h"

#include <memory>
#include <set>

namespace GOOM::COLOR
{

class RandomColorMaps : public ColorMaps
{
public:
  explicit RandomColorMaps(const UTILS::IGoomRand& goomRand) noexcept;

  [[nodiscard]] auto GetRandomColorMapName() const -> COLOR_DATA::ColorMapName;
  [[nodiscard]] auto GetRandomColorMapName(ColorMapGroup cmg) const -> COLOR_DATA::ColorMapName;

  [[nodiscard]] auto GetRandomColorMap() const -> const IColorMap&;
  [[nodiscard]] auto GetRandomColorMap(ColorMapGroup cmg) const -> const IColorMap&;

  enum class ColorMapTypes
  {
    ROTATED_T,
    SHADES,
  };
  static const std::set<ColorMapTypes> NO_COLOR_MAP_TYPES;
  static const std::set<ColorMapTypes> ALL_COLOR_MAP_TYPES;
  [[nodiscard]] auto GetRandomColorMapPtr(const std::set<ColorMapTypes>& types) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetRandomColorMapPtr(COLOR_DATA::ColorMapName colorMapName,
                                          const std::set<ColorMapTypes>& types) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetRandomColorMapPtr(ColorMapGroup colorMapGroup,
                                          const std::set<ColorMapTypes>& types) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetRandomColorMapPtr(const std::shared_ptr<const IColorMap>& colorMap,
                                          const std::set<ColorMapTypes>& types) const
      -> std::shared_ptr<const IColorMap>;

  [[nodiscard]] auto GetMinRotationPoint() const -> float;
  [[nodiscard]] auto GetMaxRotationPoint() const -> float;
  void SetRotationPointLimits(float minRotationPoint, float maxRotationPoint);

  [[nodiscard]] auto GetRandomRotatedColorMapPtr() const -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetRandomRotatedColorMapPtr(COLOR_DATA::ColorMapName colorMapName) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetRandomRotatedColorMapPtr(ColorMapGroup colorMapGroup) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetRandomRotatedColorMapPtr(
      const std::shared_ptr<const IColorMap>& colorMap) const -> std::shared_ptr<const IColorMap>;

  [[nodiscard]] auto GetMinSaturation() const -> float;
  [[nodiscard]] auto GetMaxSaturation() const -> float;
  void SetSaturationLimits(float minSaturation, float maxSaturation);
  [[nodiscard]] auto GetMaxLightness() const -> float;
  [[nodiscard]] auto GetMinLightness() const -> float;
  void SetLightnessLimits(float minLightness, float maxLightness);

  [[nodiscard]] auto GetRandomTintedColorMapPtr() const -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetRandomTintedColorMapPtr(COLOR_DATA::ColorMapName colorMapName) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetRandomTintedColorMapPtr(ColorMapGroup colorMapGroup) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetRandomTintedColorMapPtr(
      const std::shared_ptr<const IColorMap>& colorMap) const -> std::shared_ptr<const IColorMap>;

  [[nodiscard]] virtual auto GetRandomGroup() const -> ColorMapGroup;

  [[nodiscard]] auto GetRandomColor(const IColorMap& colorMap, float t0, float t1) const -> Pixel;

private:
  const UTILS::IGoomRand& m_goomRand;
  static constexpr float MIN_ROTATION_POINT = 0.1F;
  static constexpr float MAX_ROTATION_POINT = 0.9F;
  float m_minRotationPoint = MIN_ROTATION_POINT;
  float m_maxRotationPoint = MAX_ROTATION_POINT;

  static constexpr float MIN_SATURATION = 0.1F;
  static constexpr float MAX_SATURATION = 1.0F;
  static constexpr float MIN_LIGHTNESS = 0.1F;
  static constexpr float MAX_LIGHTNESS = 1.0F;
  float m_minSaturation = MIN_SATURATION;
  float m_maxSaturation = MAX_SATURATION;
  float m_minLightness = MIN_LIGHTNESS;
  float m_maxLightness = MAX_LIGHTNESS;
};

class WeightedColorMaps : public RandomColorMaps
{
public:
  WeightedColorMaps(const UTILS::IGoomRand& goomRand, const UTILS::Weights<ColorMapGroup>& weights);

  [[nodiscard]] auto GetRandomGroup() const -> ColorMapGroup override;

private:
  UTILS::Weights<ColorMapGroup> m_weights;
  bool m_weightsActive = true;
};

auto GetAllMapsUnweighted(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetAllStandardMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetAllSlimMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetMostlySequentialStandardMaps(const UTILS::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetMostlySequentialSlimMaps(const UTILS::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetSlightlyDivergingStandardMaps(const UTILS::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetSlightlyDivergingSlimMaps(const UTILS::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetBlueStandardMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetRedStandardMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetGreenStandardMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetYellowStandardMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetOrangeStandardMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetPurpleStandardMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetCitiesStandardMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetSeasonsStandardMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetHeatStandardMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetColdStandardMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetPastelStandardMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetDivergingBlackStandardMaps(const UTILS::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetWesAndersonMaps(const UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;

inline RandomColorMaps::RandomColorMaps(const UTILS::IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand}
{
}

} // namespace GOOM::COLOR
