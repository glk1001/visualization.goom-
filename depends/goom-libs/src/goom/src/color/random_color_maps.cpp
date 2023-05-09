//#undef NO_LOGGING

#include "random_color_maps.h"

#include "color_data/color_map_enums.h"
#include "color_maps.h"
#include "goom_graphic.h"
#include "goom_logger.h"
#include "utils/enum_utils.h"
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
  return static_cast<ColorMapName>(m_goomRand->GetRandInRange(0U, GetNumColorMapNames()));
}

auto RandomColorMaps::GetRandomColorMapNameFromGroup(
    const ColorMapGroup colorMapGroup) const noexcept -> COLOR_DATA::ColorMapName
{
  const auto& colorMapNames = GetColorMapNames(colorMapGroup);
  return colorMapNames[m_goomRand->GetRandInRange(0U, static_cast<uint32_t>(colorMapNames.size()))];
}

auto RandomColorMaps::GetRandomColorMap() const noexcept -> ColorMapPtrWrapper
{
  return GetColorMap(GetRandomColorMapName());
}

auto RandomColorMaps::GetRandomColorMap(const ColorMapGroup colorMapGroup) const noexcept
    -> ColorMapPtrWrapper
{
  return GetColorMap(GetRandomColorMapNameFromGroup(colorMapGroup));
}

auto RandomColorMaps::GetRandomColorMapSharedPtr(
    const std::set<ColorMapTypes>& types) const noexcept -> ColorMapSharedPtr
{
  return GetRandomColorMapSharedPtr(GetColorMapSharedPtr(GetRandomColorMapName()), types);
}

auto RandomColorMaps::GetRandomColorMapSharedPtr(
    COLOR_DATA::ColorMapName colorMapName, const std::set<ColorMapTypes>& types) const noexcept
    -> ColorMapSharedPtr
{
  if (colorMapName == COLOR_DATA::ColorMapName::_NULL)
  {
    return GetRandomColorMapSharedPtr(types);
  }
  return GetRandomColorMapSharedPtr(GetColorMapSharedPtr(colorMapName), types);
}

auto RandomColorMaps::GetRandomColorMapSharedPtr(
    ColorMapGroup colorMapGroup, const std::set<ColorMapTypes>& types) const noexcept
    -> ColorMapSharedPtr
{
  return GetRandomColorMapSharedPtr(
      GetColorMapSharedPtr(GetRandomColorMapNameFromGroup(colorMapGroup)), types);
}

auto RandomColorMaps::GetRandomColorMapSharedPtr(
    const ColorMapSharedPtr& colorMapPtr, const std::set<ColorMapTypes>& types) const noexcept
    -> ColorMapSharedPtr
{
  if (types.empty())
  {
    return colorMapPtr;
  }

  auto newColorMap = colorMapPtr;

#if __cplusplus <= 201703L
  if (types.find(ColorMapTypes::ROTATED_T) != cend(types))
#else
  if (types.contains(ColorMapTypes::ROTATED_T))
#endif
  {
    newColorMap = GetRandomRotatedColorSharedMapPtr(colorMapPtr);
  }
#if __cplusplus <= 201703L
  if (types.find(ColorMapTypes::SHADES) != cend(types))
#else
  if (types.contains(ColorMapTypes::SHADES))
#endif
  {
    newColorMap = GetRandomTintedColorMapSharedPtr(newColorMap);
  }

  return newColorMap;
}

auto RandomColorMaps::GetRandomRotatedColorSharedMapPtr(
    const ColorMapSharedPtr& colorMapPtr) const noexcept -> ColorMapSharedPtr
{
  return GetRotatedColorMapPtr(colorMapPtr,
                               m_goomRand->GetRandInRange(m_minRotationPoint, m_maxRotationPoint));
}

auto RandomColorMaps::GetRandomTintedColorMapSharedPtr(
    const ColorMapSharedPtr& colorMapPtr) const noexcept -> ColorMapSharedPtr
{
  return GetTintedColorMapPtr(colorMapPtr,
                              {m_goomRand->GetRandInRange(m_minSaturation, m_maxSaturation),
                               m_goomRand->GetRandInRange(m_minLightness, m_maxLightness)});
}

auto RandomColorMaps::GetRandomGroup() const noexcept -> ColorMapGroup
{
  return static_cast<ColorMapGroup>(m_goomRand->GetRandInRange(0U, NUM<ColorMapGroup>));
}

auto RandomColorMaps::GetRandomColor(const IColorMap& colorMap,
                                     const float t0,
                                     const float t1) const noexcept -> Pixel
{
  return colorMap.GetColor(m_goomRand->GetRandInRange(t0, t1));
}

auto RandomColorMaps::GetMinRotationPoint() const noexcept -> float
{
  return m_minRotationPoint;
}

auto RandomColorMaps::GetMaxRotationPoint() const noexcept -> float
{
  return m_maxRotationPoint;
}

auto RandomColorMaps::SetRotationPointLimits(
    const MinMaxValues<float>& minMaxRotationPoint) noexcept -> void
{
  Expects(minMaxRotationPoint.minValue >= MIN_ROTATION_POINT);
  Expects(minMaxRotationPoint.maxValue <= MAX_ROTATION_POINT);
  Expects(minMaxRotationPoint.minValue <= minMaxRotationPoint.maxValue);

  m_minRotationPoint = minMaxRotationPoint.minValue;
  m_maxRotationPoint = minMaxRotationPoint.maxValue;
}

auto RandomColorMaps::GetMinSaturation() const noexcept -> float
{
  return m_minSaturation;
}

auto RandomColorMaps::GetMaxSaturation() const noexcept -> float
{
  return m_maxSaturation;
}

auto RandomColorMaps::SetSaturationLimits(const MinMaxValues<float>& minMaxSaturation) noexcept
    -> void
{
  Expects(minMaxSaturation.minValue >= MIN_SATURATION);
  Expects(minMaxSaturation.maxValue <= MAX_SATURATION);
  Expects(minMaxSaturation.minValue <= minMaxSaturation.maxValue);

  m_minSaturation = minMaxSaturation.minValue;
  m_maxSaturation = minMaxSaturation.maxValue;
}

auto RandomColorMaps::GetMinLightness() const noexcept -> float
{
  return m_minLightness;
}

auto RandomColorMaps::GetMaxLightness() const noexcept -> float
{
  return m_maxLightness;
}

auto RandomColorMaps::SetLightnessLimits(const MinMaxValues<float>& minMaxLightness) noexcept
    -> void
{
  Expects(minMaxLightness.minValue >= MIN_LIGHTNESS);
  Expects(minMaxLightness.maxValue <= MAX_LIGHTNESS);
  Expects(minMaxLightness.minValue <= minMaxLightness.maxValue);

  m_minLightness = minMaxLightness.minValue;
  m_maxLightness = minMaxLightness.maxValue;
}

WeightedColorMaps::WeightedColorMaps(const PixelChannelType defaultAlpha,
                                     const IGoomRand& goomRand,
                                     const Weights<ColorMapGroup>& weights,
                                     const std::string& colorMapsName) noexcept
  : RandomColorMaps{defaultAlpha, goomRand, colorMapsName}, m_weights{weights}
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
  return RandomColorMaps::GetRandomColorMapNameFromGroup(GetRandomGroup());
}

} // namespace GOOM::COLOR
