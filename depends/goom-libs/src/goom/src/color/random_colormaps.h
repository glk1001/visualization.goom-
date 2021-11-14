#pragma once

#include "color/color_data/colormap_enums.h"
#include "color/colormaps.h"
#include "goom/goom_graphic.h"
#include "utils/goom_rand_base.h"

#include <array>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace COLOR
{
#else
namespace GOOM::COLOR
{
#endif

class RandomColorMaps : public ColorMaps
{
public:
  explicit RandomColorMaps(UTILS::IGoomRand& goomRand) noexcept;
  RandomColorMaps(const RandomColorMaps&) noexcept = delete;
  RandomColorMaps(RandomColorMaps&&) noexcept = delete;
  ~RandomColorMaps() noexcept override = default;
  auto operator=(const RandomColorMaps&) -> RandomColorMaps& = delete;
  auto operator=(RandomColorMaps&&) -> RandomColorMaps& = delete;

  [[nodiscard]] auto GetRandomColorMapName() const -> COLOR_DATA::ColorMapName;
  [[nodiscard]] auto GetRandomColorMapName(ColorMapGroup cmg) const -> COLOR_DATA::ColorMapName;

  [[nodiscard]] auto GetRandomColorMap() const -> const IColorMap&;
  [[nodiscard]] auto GetRandomColorMap(ColorMapGroup cmg) const -> const IColorMap&;

  enum class ColorMapTypes
  {
    ROTATED_T,
    SHADES,
  };
  static const std::set<ColorMapTypes> EMPTY;
  static const std::set<ColorMapTypes> ALL;
  [[nodiscard]] auto GetRandomColorMapPtr(const std::set<ColorMapTypes>& types = EMPTY) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetRandomColorMapPtr(COLOR_DATA::ColorMapName colorMapName,
                                          const std::set<ColorMapTypes>& types = EMPTY) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetRandomColorMapPtr(ColorMapGroup colorMapGroup,
                                          const std::set<ColorMapTypes>& types = EMPTY) const
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

  [[nodiscard]] auto GetRandomColor(const IColorMap& colorMap, float t0, float t1) -> Pixel;

private:
  UTILS::IGoomRand& m_goomRand;
#if __cplusplus <= 201402L
  static const float MIN_ROTATION_POINT;
  static const float MAX_ROTATION_POINT;
#else
  static constexpr float MIN_ROTATION_POINT = 0.1F;
  static constexpr float MAX_ROTATION_POINT = 0.9F;
#endif
  float m_minRotationPoint = MIN_ROTATION_POINT;
  float m_maxRotationPoint = MAX_ROTATION_POINT;

#if __cplusplus <= 201402L
  static const float MIN_SATURATION;
  static const float MAX_SATURATION;
  static const float MIN_LIGHTNESS;
  static const float MAX_LIGHTNESS;
#else
  static constexpr float MIN_SATURATION = 0.1F;
  static constexpr float MAX_SATURATION = 1.0F;
  static constexpr float MIN_LIGHTNESS = 0.1F;
  static constexpr float MAX_LIGHTNESS = 1.0F;
#endif
  float m_minSaturation = MIN_SATURATION;
  float m_maxSaturation = MAX_SATURATION;
  float m_minLightness = MIN_LIGHTNESS;
  float m_maxLightness = MAX_LIGHTNESS;
};

class WeightedColorMaps : public RandomColorMaps
{
public:
  WeightedColorMaps(UTILS::IGoomRand& goomRand, const UTILS::Weights<ColorMapGroup>& weights);

  [[nodiscard]] auto GetRandomGroup() const -> ColorMapGroup override;

private:
  UTILS::Weights<ColorMapGroup> m_weights;
  bool m_weightsActive = true;
};

auto GetAllMapsUnweighted(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetAllStandardMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetAllSlimMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetMostlySequentialStandardMaps(UTILS::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetMostlySequentialSlimMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetSlightlyDivergingStandardMaps(UTILS::IGoomRand& goomRand)
    -> std::shared_ptr<RandomColorMaps>;
auto GetSlightlyDivergingSlimMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetBlueStandardMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetRedStandardMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetGreenStandardMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetYellowStandardMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetOrangeStandardMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetPurpleStandardMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetCitiesStandardMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetSeasonsStandardMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetHeatStandardMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetColdStandardMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetPastelStandardMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;
auto GetDivergingBlackStandardMaps(UTILS::IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>;

inline RandomColorMaps::RandomColorMaps(UTILS::IGoomRand& goomRand) noexcept : m_goomRand{goomRand}
{
}

#if __cplusplus <= 201402L
} // namespace COLOR
} // namespace GOOM
#else
} // namespace GOOM::COLOR
#endif
