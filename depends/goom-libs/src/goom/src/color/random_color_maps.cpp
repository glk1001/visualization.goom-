module;

//#undef NO_LOGGING

#include "color_data/color_map_enums.h"
#include "goom/goom_graphic.h"
#include "goom/goom_types.h"

#include <cstdint>
#include <memory>
#include <set>
#include <string>

module Goom.Color.RandomColorMaps;

import Goom.Color.ColorMapBase;
import Goom.Color.ColorMaps;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;

namespace GOOM::COLOR
{

using COLOR_DATA::ColorMapName;
using UTILS::NUM;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Weights;

auto RandomColorMaps::GetRandomColorMapName() const noexcept -> COLOR_DATA::ColorMapName
{
  Expects(IsActive());

  return static_cast<ColorMapName>(m_goomRand->GetRandInRange(0U, GetNumColorMapNames()));
}

auto RandomColorMaps::GetRandomColorMapNameFromGroup(
    const ColorMapGroup colorMapGroup) const noexcept -> COLOR_DATA::ColorMapName
{
  Expects(IsActive());

  const auto& colorMapNames = GetColorMapNames(colorMapGroup);
  return colorMapNames[m_goomRand->GetRandInRange(0U, static_cast<uint32_t>(colorMapNames.size()))];
}

auto RandomColorMaps::GetRandomColorMap() const noexcept -> ColorMapPtrWrapper
{
  Expects(IsActive());

  return GetColorMap(GetRandomColorMapName());
}

auto RandomColorMaps::GetRandomColorMap(const ColorMapGroup colorMapGroup) const noexcept
    -> ColorMapPtrWrapper
{
  Expects(IsActive());

  return GetColorMap(GetRandomColorMapNameFromGroup(colorMapGroup));
}

auto RandomColorMaps::GetRandomColorMapSharedPtr(
    const std::set<ColorMapTypes>& types) const noexcept -> ConstColorMapSharedPtr
{
  Expects(IsActive());

  return GetRandomColorMapSharedPtr(GetColorMapSharedPtr(GetRandomColorMapName()), types);
}

auto RandomColorMaps::GetRandomColorMapSharedPtr(
    const COLOR_DATA::ColorMapName colorMapName,
    const std::set<ColorMapTypes>& types) const noexcept -> ConstColorMapSharedPtr
{
  Expects(IsActive());

  if (colorMapName == COLOR_DATA::ColorMapName::_NULL)
  {
    return GetRandomColorMapSharedPtr(types);
  }
  return GetRandomColorMapSharedPtr(GetColorMapSharedPtr(colorMapName), types);
}

auto RandomColorMaps::GetRandomColorMapSharedPtr(
    const ColorMapGroup colorMapGroup, const std::set<ColorMapTypes>& types) const noexcept
    -> ConstColorMapSharedPtr
{
  Expects(IsActive());

  return GetRandomColorMapSharedPtr(
      GetColorMapSharedPtr(GetRandomColorMapNameFromGroup(colorMapGroup)), types);
}

auto RandomColorMaps::GetRandomColorMapSharedPtr(
    const ConstColorMapSharedPtr& colorMapPtr, const std::set<ColorMapTypes>& types) const noexcept
    -> ConstColorMapSharedPtr
{
  Expects(IsActive());

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
    const ConstColorMapSharedPtr& colorMapPtr) const noexcept -> ConstColorMapSharedPtr
{
  Expects(IsActive());

  return GetRotatedColorMapPtr(colorMapPtr,
                               m_goomRand->GetRandInRange(m_minRotationPoint, m_maxRotationPoint));
}

auto RandomColorMaps::GetRandomTintedColorMapSharedPtr(
    const ConstColorMapSharedPtr& colorMapPtr) const noexcept -> ConstColorMapSharedPtr
{
  Expects(IsActive());

  return GetTintedColorMapPtr(colorMapPtr,
                              {m_goomRand->GetRandInRange(m_minSaturation, m_maxSaturation),
                               m_goomRand->GetRandInRange(m_minLightness, m_maxLightness)});
}

auto RandomColorMaps::GetRandomGroup() const noexcept -> ColorMapGroup
{
  Expects(IsActive());

  return static_cast<ColorMapGroup>(m_goomRand->GetRandInRange(0U, NUM<ColorMapGroup>));
}

auto RandomColorMaps::GetRandomColor(const IColorMap& colorMap,
                                     const float t0,
                                     const float t1) const noexcept -> Pixel
{
  Expects(IsActive());

  return colorMap.GetColor(m_goomRand->GetRandInRange(t0, t1));
}

auto RandomColorMaps::GetMinRotationPoint() const noexcept -> float
{
  Expects(IsActive());

  return m_minRotationPoint;
}

auto RandomColorMaps::GetMaxRotationPoint() const noexcept -> float
{
  Expects(IsActive());

  return m_maxRotationPoint;
}

auto RandomColorMaps::SetRotationPointLimits(
    const MinMaxValues<float>& minMaxRotationPoint) noexcept -> void
{
  Expects(IsActive());
  Expects(minMaxRotationPoint.minValue >= MIN_ROTATION_POINT);
  Expects(minMaxRotationPoint.maxValue <= MAX_ROTATION_POINT);
  Expects(minMaxRotationPoint.minValue <= minMaxRotationPoint.maxValue);

  m_minRotationPoint = minMaxRotationPoint.minValue;
  m_maxRotationPoint = minMaxRotationPoint.maxValue;
}

auto RandomColorMaps::GetMinSaturation() const noexcept -> float
{
  Expects(IsActive());

  return m_minSaturation;
}

auto RandomColorMaps::GetMaxSaturation() const noexcept -> float
{
  Expects(IsActive());

  return m_maxSaturation;
}

auto RandomColorMaps::SetSaturationLimits(const MinMaxValues<float>& minMaxSaturation) noexcept
    -> void
{
  Expects(IsActive());
  Expects(minMaxSaturation.minValue >= MIN_SATURATION);
  Expects(minMaxSaturation.maxValue <= MAX_SATURATION);
  Expects(minMaxSaturation.minValue <= minMaxSaturation.maxValue);

  m_minSaturation = minMaxSaturation.minValue;
  m_maxSaturation = minMaxSaturation.maxValue;
}

auto RandomColorMaps::GetMinLightness() const noexcept -> float
{
  Expects(IsActive());

  return m_minLightness;
}

auto RandomColorMaps::GetMaxLightness() const noexcept -> float
{
  Expects(IsActive());

  return m_maxLightness;
}

auto RandomColorMaps::SetLightnessLimits(const MinMaxValues<float>& minMaxLightness) noexcept
    -> void
{
  Expects(IsActive());
  Expects(minMaxLightness.minValue >= MIN_LIGHTNESS);
  Expects(minMaxLightness.maxValue <= MAX_LIGHTNESS);
  Expects(minMaxLightness.minValue <= minMaxLightness.maxValue);

  m_minLightness = minMaxLightness.minValue;
  m_maxLightness = minMaxLightness.maxValue;
}

WeightedRandomColorMaps::WeightedRandomColorMaps(const PixelChannelType defaultAlpha,
                                                 const IGoomRand& goomRand,
                                                 const Weights<ColorMapGroup>& weights,
                                                 const std::string& colorMapsName) noexcept
  : RandomColorMaps{defaultAlpha, goomRand, colorMapsName}, m_weights{weights}
{
}

WeightedRandomColorMaps::WeightedRandomColorMaps(
    const WeightedRandomColorMaps& weightedRandomColorMaps,
    const PixelChannelType newDefaultAlpha) noexcept
  : RandomColorMaps{newDefaultAlpha,
                    weightedRandomColorMaps.GetGoomRand(),
                    weightedRandomColorMaps.GetColorMapsName()},
    m_weights{weightedRandomColorMaps.m_weights}
{
}

auto WeightedRandomColorMaps::GetRandomGroup() const noexcept -> ColorMapGroup
{
  Expects(IsActive());

  if (not m_weightsActive)
  {
    return RandomColorMaps::GetRandomGroup();
  }

  return m_weights.GetRandomWeighted();
}

auto WeightedRandomColorMaps::GetRandomColorMapName() const noexcept -> COLOR_DATA::ColorMapName
{
  Expects(IsActive());

  return RandomColorMaps::GetRandomColorMapNameFromGroup(GetRandomGroup());
}

} // namespace GOOM::COLOR
