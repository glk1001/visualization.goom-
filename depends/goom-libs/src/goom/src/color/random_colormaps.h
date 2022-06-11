#pragma once

#include "color/color_data/colormap_enums.h"
#include "color/colormaps.h"
#include "goom/goom_graphic.h"
#include "utils/math/goom_rand_base.h"

#include <memory>
#include <set>

namespace GOOM::COLOR
{

class RandomColorMaps : public ColorMaps
{
public:
  RandomColorMaps(const UTILS::MATH::IGoomRand& goomRand,
                  const std::string& colorMapsName = "") noexcept;

  [[nodiscard]] auto GetColorMapsName() const noexcept -> const std::string&;

  [[nodiscard]] virtual auto GetRandomColorMapName() const -> COLOR_DATA::ColorMapName;
  [[nodiscard]] auto GetRandomColorMap() const -> const IColorMap&;
  [[nodiscard]] auto GetRandomColorMap(ColorMapGroup colorMapGroup) const -> const IColorMap&;

  enum class ColorMapTypes
  {
    ROTATED_T,
    SHADES,
  };
  static inline const std::set<ColorMapTypes> NO_COLOR_MAP_TYPES{};
  static inline const std::set<ColorMapTypes> ALL_COLOR_MAP_TYPES{
      ColorMapTypes::ROTATED_T,
      ColorMapTypes::SHADES,
  };
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

protected:
  [[nodiscard]] auto GetRandomColorMapName(ColorMapGroup colorMapGroup) const
      -> COLOR_DATA::ColorMapName;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  const std::string m_colorMapsName;
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
  WeightedColorMaps(const UTILS::MATH::IGoomRand& goomRand,
                    const UTILS::MATH::Weights<ColorMapGroup>& weights,
                    const std::string& colorMapsName = "");

  [[nodiscard]] auto GetRandomGroup() const -> ColorMapGroup override;
  [[nodiscard]] auto GetRandomColorMapName() const -> COLOR_DATA::ColorMapName override;

private:
  UTILS::MATH::Weights<ColorMapGroup> m_weights;
  bool m_weightsActive = true;
};

auto GetAllMapsUnweighted(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetAllStandardMaps(const UTILS::MATH::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetAllSlimMaps(const UTILS::MATH::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetMostlySequentialStandardMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetMostlySequentialSlimMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetSlightlyDivergingStandardMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetSlightlyDivergingSlimMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetBlueStandardMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetRedStandardMaps(const UTILS::MATH::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetGreenStandardMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetYellowStandardMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetOrangeStandardMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetPurpleStandardMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetCitiesStandardMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetSeasonsStandardMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetHeatStandardMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetColdStandardMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetPastelStandardMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetDivergingBlackStandardMaps(const UTILS::MATH::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetWesAndersonMaps(const UTILS::MATH::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;

inline RandomColorMaps::RandomColorMaps(const UTILS::MATH::IGoomRand& goomRand,
                                        const std::string& colorMapsName) noexcept
  : m_goomRand{goomRand}, m_colorMapsName{colorMapsName}
{
}

inline auto RandomColorMaps::GetColorMapsName() const noexcept -> const std::string&
{
  return m_colorMapsName;
}

} // namespace GOOM::COLOR
