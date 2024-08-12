module;

//#undef NO_LOGGING

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <utility>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // glm: nonstandard extension used: nameless struct/union
#pragma warning(disable : 4242) // possible loss of data
#pragma warning(disable : 4244) // possible loss of data
#endif
#include <vivid/vivid.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

module Goom.Color.ColorMaps;

import Goom.Color.ColorData.ColorDataMaps;
import Goom.Color.ColorData.ColorMapEnums;
import Goom.Color.ColorData.ExtraColorGroups;
import Goom.Color.ColorMapBase;
import Goom.Color.ColorUtils;
import Goom.Utils.EnumUtils;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;

namespace GOOM::COLOR
{

using COLOR_DATA::ColorMapName;
using COLOR_DATA::GetAllMapArrays;
using COLOR_DATA::GetAllMaps;
using COLOR_DATA::GetBlueMaps;
using COLOR_DATA::GetCityMaps;
using COLOR_DATA::GetColdMaps;
using COLOR_DATA::GetCyclicMaps;
using COLOR_DATA::GetCyclicSlimMaps;
using COLOR_DATA::GetDivergingBlackMaps;
using COLOR_DATA::GetDivergingBlackSlimMaps;
using COLOR_DATA::GetDivergingMaps;
using COLOR_DATA::GetDivergingSlimMaps;
using COLOR_DATA::GetGreenMaps;
using COLOR_DATA::GetHeatMaps;
using COLOR_DATA::GetMiscMaps;
using COLOR_DATA::GetMiscSlimMaps;
using COLOR_DATA::GetOrangeMaps;
using COLOR_DATA::GetPastelMaps;
using COLOR_DATA::GetPercUnifSequentialMaps;
using COLOR_DATA::GetPercUnifSequentialSlimMaps;
using COLOR_DATA::GetPurpleMaps;
using COLOR_DATA::GetQualitativeMaps;
using COLOR_DATA::GetQualitativeSlimMaps;
using COLOR_DATA::GetRedMaps;
using COLOR_DATA::GetSeasonMaps;
using COLOR_DATA::GetSequential2Maps;
using COLOR_DATA::GetSequential2SlimMaps;
using COLOR_DATA::GetSequentialMaps;
using COLOR_DATA::GetSequentialSlimMaps;
using COLOR_DATA::GetWesAndersonMaps;
using COLOR_DATA::GetYellowMaps;
using COLOR_DATA::NUM_COLOR_MAP_ENUMS;
using UTILS::NUM;

class ColorMapSharedPtrWrapper : public IColorMap
{
public:
  ColorMapSharedPtrWrapper(const ConstColorMapSharedPtr& colorMapPtr,
                           PixelChannelType defaultAlpha) noexcept;
  ColorMapSharedPtrWrapper(const ColorMapSharedPtrWrapper&) noexcept                    = default;
  ColorMapSharedPtrWrapper(ColorMapSharedPtrWrapper&&) noexcept                         = default;
  ~ColorMapSharedPtrWrapper() noexcept override                                         = default;
  auto operator=(const ColorMapSharedPtrWrapper&) noexcept -> ColorMapSharedPtrWrapper& = default;
  auto operator=(ColorMapSharedPtrWrapper&&) noexcept -> ColorMapSharedPtrWrapper&      = default;

  [[nodiscard]] auto GetNumStops() const noexcept -> size_t override;
  [[nodiscard]] auto GetMapName() const noexcept -> ColorMapName override;
  [[nodiscard]] auto GetColor(float t) const noexcept -> Pixel override;

protected:
  [[nodiscard]] auto GetColorMap() const noexcept -> const IColorMap& { return *m_colorMapPtr; }
  [[nodiscard]] auto GetDefaultAlpha() const noexcept -> PixelChannelType { return m_defaultAlpha; }

private:
  ConstColorMapSharedPtr m_colorMapPtr;
  PixelChannelType m_defaultAlpha;
};

class RotatedColorMap : public ColorMapSharedPtrWrapper
{
public:
  RotatedColorMap(const ConstColorMapSharedPtr& colorMapPtr,
                  PixelChannelType defaultAlpha,
                  float tRotatePoint) noexcept;

  [[nodiscard]] auto GetColor(float t) const noexcept -> Pixel override;

private:
  static constexpr float MIN_ROTATE_POINT = 0.0F;
  static constexpr float MAX_ROTATE_POINT = 1.0F;
  float m_tRotatePoint;
};

class TintedColorMap : public ColorMapSharedPtrWrapper
{
public:
  TintedColorMap(const ConstColorMapSharedPtr& colorMapPtr,
                 PixelChannelType defaultAlpha,
                 const ColorMaps::TintProperties& tintProperties) noexcept;

  [[nodiscard]] auto GetColor(float t) const noexcept -> Pixel override;

private:
  static constexpr float MIN_LIGHTNESS = 0.1F;
  static constexpr float MAX_LIGHTNESS = 1.0F;
  float m_saturation;
  float m_lightness;
};

class PrebuiltColorMap : public IColorMap
{
public:
  PrebuiltColorMap(ColorMapName mapName, const std::span<const vivid::srgb_t>& vividArray) noexcept;
  PrebuiltColorMap(const PrebuiltColorMap&) noexcept           = delete;
  PrebuiltColorMap(PrebuiltColorMap&&) noexcept                = default;
  ~PrebuiltColorMap() noexcept override                        = default;
  auto operator=(const PrebuiltColorMap&) -> PrebuiltColorMap& = delete;
  auto operator=(PrebuiltColorMap&&) -> PrebuiltColorMap&      = delete;

  [[nodiscard]] auto GetNumStops() const noexcept -> size_t override
  {
    return m_vividColorMap.numStops();
  }
  [[nodiscard]] auto GetMapName() const noexcept -> ColorMapName override { return m_mapName; }
  [[nodiscard]] auto GetColor(float t) const noexcept -> Pixel override;

  static auto GetColorMix(const Pixel& col1, const Pixel& col2, float t) noexcept -> Pixel;

private:
  ColorMapName m_mapName;
  vivid::ColorMap m_vividColorMap;
};

namespace
{

template<class T>
auto at(std::array<T, NUM<ColorMapGroup>>& arr, ColorMapGroup idx) noexcept -> T&;

[[nodiscard]] auto GetPreBuiltColorMaps() noexcept -> const std::vector<PrebuiltColorMap>&;
[[nodiscard]] auto MakePrebuiltColorMaps() noexcept -> std::vector<PrebuiltColorMap>;

using ColorGroupNamesArray = std::array<std::span<const ColorMapName>, NUM<ColorMapGroup>>;
[[nodiscard]] auto GetColorGroupNames() noexcept -> const ColorGroupNamesArray&;
[[nodiscard]] auto MakeColorGroupNames() noexcept -> ColorGroupNamesArray;

} // namespace

auto ColorMaps::GetColorMix(const Pixel& color1,
                            const Pixel& color2,
                            const float t) noexcept -> Pixel
{
  return PrebuiltColorMap::GetColorMix(color1, color2, t);
}

ColorMaps::ColorMaps(const PixelChannelType defaultAlpha) noexcept : m_defaultAlpha{defaultAlpha}
{
}

auto ColorMaps::GetColorMap(const ColorMapName colorMapName) const noexcept -> ColorMapPtrWrapper
{
  return ColorMapPtrWrapper{&GetPreBuiltColorMaps().at(static_cast<size_t>(colorMapName)),
                            m_defaultAlpha};
}

auto ColorMaps::GetRotatedColorMapPtr(const ColorMapName colorMapName, const float tRotatePoint)
    const noexcept -> ConstColorMapSharedPtr
{
  return std::make_shared<RotatedColorMap>(
      GetColorMapSharedPtr(colorMapName), m_defaultAlpha, tRotatePoint);
}

auto ColorMaps::GetRotatedColorMapPtr(const ConstColorMapSharedPtr& colorMapPtr,
                                      const float tRotatePoint) const noexcept
    -> ConstColorMapSharedPtr
{
  return std::make_shared<RotatedColorMap>(colorMapPtr, m_defaultAlpha, tRotatePoint);
}

auto ColorMaps::GetTintedColorMapPtr(const ColorMapName colorMapName,
                                     const TintProperties& tintProperties) const noexcept
    -> ConstColorMapSharedPtr
{
  return std::make_shared<TintedColorMap>(
      GetColorMapSharedPtr(colorMapName), m_defaultAlpha, tintProperties);
}

auto ColorMaps::GetTintedColorMapPtr(const ConstColorMapSharedPtr& colorMapPtr,
                                     const TintProperties& tintProperties) const noexcept
    -> ConstColorMapSharedPtr
{
  return std::make_shared<TintedColorMap>(colorMapPtr, m_defaultAlpha, tintProperties);
}

auto ColorMaps::GetNumColorMapNames() noexcept -> uint32_t
{
  return static_cast<uint32_t>(GetPreBuiltColorMaps().size());
}

auto ColorMaps::GetColorMapNames(const ColorMapGroup colorMapGroup) noexcept
    -> std::span<const ColorMapName>
{
  if (colorMapGroup == ColorMapGroup::ALL)
  {
    return GetAllMaps();
  }

  return GetColorGroupNames().at(static_cast<size_t>(colorMapGroup));
}

auto ColorMaps::GetNumGroups() noexcept -> uint32_t
{
  return static_cast<uint32_t>(GetColorGroupNames().size());
}

// Wrap a raw pointer in a shared_ptr and make sure the raw pointer is never deleted.
static const auto MAKE_SHARED_ADDR = [](const IColorMap* const colorMap)
{
  return ConstColorMapSharedPtr{
      colorMap, []([[maybe_unused]] const IColorMap* const cm) { /* never delete */ }};
};

auto ColorMaps::GetColorMapSharedPtr(const ColorMapName colorMapName) noexcept
    -> ConstColorMapSharedPtr
{
  return MAKE_SHARED_ADDR(&GetPreBuiltColorMaps().at(static_cast<size_t>(colorMapName)));
}

namespace
{

auto GetPreBuiltColorMaps() noexcept -> const std::vector<PrebuiltColorMap>&
{
  static const auto s_PRE_BUILT_COLOR_MAPS = MakePrebuiltColorMaps();

  return s_PRE_BUILT_COLOR_MAPS;
}

auto MakePrebuiltColorMaps() noexcept -> std::vector<PrebuiltColorMap>
{
  Expects(NUM_COLOR_MAP_ENUMS == GetAllMapArrays().size());

  auto preBuiltColorMaps = std::vector<PrebuiltColorMap>{};
  preBuiltColorMaps.reserve(GetAllMapArrays().size());

  for (const auto& [colorMap, vivArray] : GetAllMapArrays())
  {
    preBuiltColorMaps.emplace_back(colorMap, vivArray);
  }

  return preBuiltColorMaps;
}

template<class T>
auto at(std::array<T, NUM<ColorMapGroup>>& arr, const ColorMapGroup idx) noexcept -> T&
{
  return arr.at(static_cast<size_t>(idx));
}

auto GetColorGroupNames() noexcept -> const ColorGroupNamesArray&
{
  static const auto s_COLOR_GROUP_NAMES = MakeColorGroupNames();

  return s_COLOR_GROUP_NAMES;
}

auto MakeColorGroupNames() noexcept -> ColorGroupNamesArray
{
  auto groups = ColorGroupNamesArray{};

  at(groups, ColorMapGroup::ALL) = GetAllMaps();

  at(groups, ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL) = GetPercUnifSequentialMaps();
  at(groups, ColorMapGroup::SEQUENTIAL)                      = GetSequentialMaps();
  at(groups, ColorMapGroup::SEQUENTIAL2)                     = GetSequential2Maps();
  at(groups, ColorMapGroup::CYCLIC)                          = GetCyclicMaps();
  at(groups, ColorMapGroup::DIVERGING)                       = GetDivergingMaps();
  at(groups, ColorMapGroup::DIVERGING_BLACK)                 = GetDivergingBlackMaps();
  at(groups, ColorMapGroup::QUALITATIVE)                     = GetQualitativeMaps();
  at(groups, ColorMapGroup::MISC)                            = GetMiscMaps();

  at(groups, ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM) = GetPercUnifSequentialSlimMaps();
  at(groups, ColorMapGroup::SEQUENTIAL_SLIM)                      = GetSequentialSlimMaps();
  at(groups, ColorMapGroup::SEQUENTIAL2_SLIM)                     = GetSequential2SlimMaps();
  at(groups, ColorMapGroup::CYCLIC_SLIM)                          = GetCyclicSlimMaps();
  at(groups, ColorMapGroup::DIVERGING_SLIM)                       = GetDivergingSlimMaps();
  at(groups, ColorMapGroup::DIVERGING_BLACK_SLIM)                 = GetDivergingBlackSlimMaps();
  at(groups, ColorMapGroup::QUALITATIVE_SLIM)                     = GetQualitativeSlimMaps();
  at(groups, ColorMapGroup::MISC_SLIM)                            = GetMiscSlimMaps();

  at(groups, ColorMapGroup::WES_ANDERSON) = GetWesAndersonMaps();
  at(groups, ColorMapGroup::BLUES)        = GetBlueMaps();
  at(groups, ColorMapGroup::REDS)         = GetRedMaps();
  at(groups, ColorMapGroup::GREENS)       = GetGreenMaps();
  at(groups, ColorMapGroup::YELLOWS)      = GetYellowMaps();
  at(groups, ColorMapGroup::ORANGES)      = GetOrangeMaps();
  at(groups, ColorMapGroup::PURPLES)      = GetPurpleMaps();
  at(groups, ColorMapGroup::CITIES)       = GetCityMaps();
  at(groups, ColorMapGroup::SEASONS)      = GetSeasonMaps();
  at(groups, ColorMapGroup::HEAT)         = GetHeatMaps();
  at(groups, ColorMapGroup::COLD)         = GetColdMaps();
  at(groups, ColorMapGroup::PASTEL)       = GetPastelMaps();

  return groups;
}

} // namespace

inline ColorMapSharedPtrWrapper::ColorMapSharedPtrWrapper(
    const ConstColorMapSharedPtr& colorMapPtr, const PixelChannelType defaultAlpha) noexcept
  : m_colorMapPtr{colorMapPtr}, m_defaultAlpha{defaultAlpha}
{
}

inline auto ColorMapSharedPtrWrapper::GetNumStops() const noexcept -> size_t
{
  return m_colorMapPtr->GetNumStops();
}

inline auto ColorMapSharedPtrWrapper::GetMapName() const noexcept -> COLOR_DATA::ColorMapName
{
  return m_colorMapPtr->GetMapName();
}

inline auto ColorMapSharedPtrWrapper::GetColor(const float t) const noexcept -> Pixel
{
  const auto color = m_colorMapPtr->GetColor(t);
  return Pixel{
      {.red = color.R(), .green = color.G(), .blue = color.B(), .alpha = m_defaultAlpha}
  };
}

RotatedColorMap::RotatedColorMap(const ConstColorMapSharedPtr& colorMapPtr,
                                 // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                                 const PixelChannelType defaultAlpha,
                                 const float tRotatePoint) noexcept
  : ColorMapSharedPtrWrapper{colorMapPtr, defaultAlpha}, m_tRotatePoint{tRotatePoint}
{
  Expects(tRotatePoint >= MIN_ROTATE_POINT);
  Expects(tRotatePoint <= MAX_ROTATE_POINT);
}

inline auto RotatedColorMap::GetColor(const float t) const noexcept -> Pixel
{
  auto tNew = m_tRotatePoint + t;
  if (tNew > 1.0F)
  {
    tNew = tNew - 1.0F;
  }
  return ColorMapSharedPtrWrapper::GetColor(tNew);
}

TintedColorMap::TintedColorMap(const ConstColorMapSharedPtr& colorMapPtr,
                               const PixelChannelType defaultAlpha,
                               const ColorMaps::TintProperties& tintProperties) noexcept
  : ColorMapSharedPtrWrapper{colorMapPtr, defaultAlpha},
    m_saturation{tintProperties.saturation},
    m_lightness{tintProperties.lightness}
{
  Expects(tintProperties.lightness >= MIN_LIGHTNESS);
  Expects(tintProperties.lightness <= MAX_LIGHTNESS);
}

auto TintedColorMap::GetColor(const float t) const noexcept -> Pixel
{
  const auto color = GetColorMap().GetColor(t);
  const auto rgb8  = vivid::col8_t{color.R(), color.G(), color.B()};

  static constexpr auto SATURATION_INDEX = 1;
  static constexpr auto LIGHTNESS_INDEX  = 2;

  auto hsv              = vivid::hsv_t{vivid::rgb::fromRgb8(rgb8)};
  hsv[SATURATION_INDEX] = m_saturation;
  hsv[LIGHTNESS_INDEX]  = m_lightness;

  const auto newRgb8 = vivid::rgb8::fromRgb(vivid::rgb::fromHsv(hsv));
  return Pixel{
      {newRgb8.r, newRgb8.g, newRgb8.b, GetDefaultAlpha()}  // NOLINT: union hard to fix here
  };
}

inline PrebuiltColorMap::PrebuiltColorMap(const ColorMapName mapName,
                                          const std::span<const vivid::srgb_t>& vividArray) noexcept
  : m_mapName{mapName}, m_vividColorMap{vividArray}
{
}

inline auto PrebuiltColorMap::GetColor(const float t) const noexcept -> Pixel
{
  const auto rgb8 = vivid::col8_t{vivid::rgb8::fromRgb(m_vividColorMap.at(t))};
  return Pixel{
      {rgb8.r, rgb8.g, rgb8.b, MAX_ALPHA}  // NOLINT: union hard to fix here
  };
}

inline auto PrebuiltColorMap::GetColorMix(const Pixel& col1,
                                          const Pixel& col2,
                                          const float t) noexcept -> Pixel
{
  // Optimisation: faster to use this lesser quality RGB Lerp than vivid's Lerp.
  return GetRgbColorLerp(col1, col2, t);
  /**
  const vivid::rgb_t c1 = vivid::rgb::fromRgb32(col1.Rgba());
  const vivid::rgb_t c2 = vivid::rgb::fromRgb32(col2.Rgba());
  return Pixel{vivid::lerpHsl(c1, c2, t).rgb32()};
   **/
}

} // namespace GOOM::COLOR
