#pragma once

#include "color_data/color_map_enums.h"
#include "color_maps.h"
#include "color_maps_base.h"
#include "goom_graphic.h"
#include "goom_types.h"
#include "utils/math/goom_rand_base.h"

#include <memory>
#include <set>
#include <string>

namespace GOOM::COLOR
{

class RandomColorMaps : public ColorMaps
{
public:
  RandomColorMaps() noexcept = default;
  RandomColorMaps(PixelChannelType defaultAlpha,
                  const UTILS::MATH::IGoomRand& goomRand,
                  const std::string& colorMapsName = "") noexcept;
  RandomColorMaps(const RandomColorMaps&)                    = default;
  RandomColorMaps(RandomColorMaps&&)                         = default;
  virtual ~RandomColorMaps()                                 = default;
  auto operator=(const RandomColorMaps&) -> RandomColorMaps& = default;
  auto operator=(RandomColorMaps&&) -> RandomColorMaps&      = default;

  [[nodiscard]] auto GetColorMapsName() const noexcept -> const std::string&;
  [[nodiscard]] virtual auto GetRandomColorMapName() const noexcept -> COLOR_DATA::ColorMapName;

  [[nodiscard]] auto GetRandomColorMap() const noexcept -> ColorMapPtrWrapper;
  [[nodiscard]] auto GetRandomColorMap(ColorMapGroup colorMapGroup) const noexcept
      -> ColorMapPtrWrapper;

  enum class ColorMapTypes
  {
    ROTATED_T,
    SHADES,
  };
  [[nodiscard]] static auto GetNoColorMapsTypes() noexcept -> const std::set<ColorMapTypes>&;
  [[nodiscard]] static auto GetAllColorMapsTypes() noexcept -> const std::set<ColorMapTypes>&;

  [[nodiscard]] auto GetRandomColorMapSharedPtr(const std::set<ColorMapTypes>& types) const noexcept
      -> ColorMapSharedPtr;
  [[nodiscard]] auto GetRandomColorMapSharedPtr(COLOR_DATA::ColorMapName colorMapName,
                                                const std::set<ColorMapTypes>& types) const noexcept
      -> ColorMapSharedPtr;
  [[nodiscard]] auto GetRandomColorMapSharedPtr(ColorMapGroup colorMapGroup,
                                                const std::set<ColorMapTypes>& types) const noexcept
      -> ColorMapSharedPtr;
  [[nodiscard]] auto GetRandomColorMapSharedPtr(const ColorMapSharedPtr& colorMapPtr,
                                                const std::set<ColorMapTypes>& types) const noexcept
      -> ColorMapSharedPtr;

  [[nodiscard]] auto GetMinRotationPoint() const noexcept -> float;
  [[nodiscard]] auto GetMaxRotationPoint() const noexcept -> float;
  auto SetRotationPointLimits(const MinMaxValues<float>& minMaxRotationPoint) noexcept -> void;

  [[nodiscard]] auto GetMinSaturation() const noexcept -> float;
  [[nodiscard]] auto GetMaxSaturation() const noexcept -> float;
  auto SetSaturationLimits(const MinMaxValues<float>& minMaxSaturation) noexcept -> void;
  [[nodiscard]] auto GetMaxLightness() const noexcept -> float;
  [[nodiscard]] auto GetMinLightness() const noexcept -> float;
  auto SetLightnessLimits(const MinMaxValues<float>& minMaxLightness) noexcept -> void;

  [[nodiscard]] virtual auto GetRandomGroup() const noexcept -> ColorMapGroup;

  [[nodiscard]] auto GetRandomColor(const IColorMap& colorMap, float t0, float t1) const noexcept
      -> Pixel;

protected:
  [[nodiscard]] auto GetGoomRand() const noexcept -> const UTILS::MATH::IGoomRand&;
  [[nodiscard]] auto GetRandomColorMapNameFromGroup(ColorMapGroup colorMapGroup) const noexcept
      -> COLOR_DATA::ColorMapName;

private:
  const UTILS::MATH::IGoomRand* m_goomRand = nullptr;
  std::string m_colorMapsName{};

  static constexpr float MIN_ROTATION_POINT = 0.5F;
  static constexpr float MAX_ROTATION_POINT = 0.9F;
  float m_minRotationPoint                  = MIN_ROTATION_POINT;
  float m_maxRotationPoint                  = MAX_ROTATION_POINT;

  static constexpr float MIN_SATURATION = 0.5F;
  static constexpr float MAX_SATURATION = 1.0F;
  static constexpr float MIN_LIGHTNESS  = 0.5F;
  static constexpr float MAX_LIGHTNESS  = 1.0F;
  float m_minSaturation                 = MIN_SATURATION;
  float m_maxSaturation                 = MAX_SATURATION;
  float m_minLightness                  = MIN_LIGHTNESS;
  float m_maxLightness                  = MAX_LIGHTNESS;

  [[nodiscard]] auto GetRandomRotatedColorSharedMapPtr(
      const ColorMapSharedPtr& colorMapPtr) const noexcept -> ColorMapSharedPtr;
  [[nodiscard]] auto GetRandomTintedColorMapSharedPtr(
      const ColorMapSharedPtr& colorMapPtr) const noexcept -> ColorMapSharedPtr;
};

class WeightedRandomColorMaps : public RandomColorMaps
{
public:
  WeightedRandomColorMaps() noexcept = default;
  WeightedRandomColorMaps(PixelChannelType defaultAlpha,
                          const UTILS::MATH::IGoomRand& goomRand,
                          const UTILS::MATH::Weights<ColorMapGroup>& weights,
                          const std::string& colorMapsName = "") noexcept;
  WeightedRandomColorMaps(const WeightedRandomColorMaps& weightedRandomColorMaps,
                          PixelChannelType newDefaultAlpha) noexcept;

  [[nodiscard]] auto GetRandomGroup() const noexcept -> ColorMapGroup override;
  [[nodiscard]] auto GetRandomColorMapName() const noexcept -> COLOR_DATA::ColorMapName override;

private:
  UTILS::MATH::Weights<ColorMapGroup> m_weights{};
  bool m_weightsActive = true;
};

inline RandomColorMaps::RandomColorMaps(const PixelChannelType defaultAlpha,
                                        const UTILS::MATH::IGoomRand& goomRand,
                                        const std::string& colorMapsName) noexcept
  : ColorMaps{defaultAlpha}, m_goomRand{&goomRand}, m_colorMapsName{colorMapsName}
{
}

inline auto RandomColorMaps::GetNoColorMapsTypes() noexcept -> const std::set<ColorMapTypes>&
{
  static const auto s_NO_COLOR_MAP_TYPES = std::set<ColorMapTypes>{};

  return s_NO_COLOR_MAP_TYPES;
}

inline auto RandomColorMaps::GetAllColorMapsTypes() noexcept -> const std::set<ColorMapTypes>&
{
  static const auto s_ALL_COLOR_MAP_TYPES = std::set<ColorMapTypes>{
      ColorMapTypes::ROTATED_T,
      ColorMapTypes::SHADES,
  };

  return s_ALL_COLOR_MAP_TYPES;
}

inline auto RandomColorMaps::GetColorMapsName() const noexcept -> const std::string&
{
  return m_colorMapsName;
}

inline auto RandomColorMaps::GetGoomRand() const noexcept -> const UTILS::MATH::IGoomRand&
{
  return *m_goomRand;
}

} // namespace GOOM::COLOR
