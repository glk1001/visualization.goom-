#include "colormaps.h"

#include "color_data/color_data_maps.h"
#include "color_data/colormap_enums.h"
#include "color_data/extra_color_groups.h"
#include "colorutils.h"
#include "goom/goom_graphic.h"
#include "utils/enumutils.h"
#include "utils/logging_control.h"
//#undef NO_LOGGING
#include "utils/logging.h"
#include "utils/spimpl.h"

#include <algorithm>
#undef NDEBUG
#include <cassert>
#include <format>
#include <vector>
#include <vivid/vivid.h>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace COLOR
{
#else
namespace GOOM::COLOR
{
#endif

using COLOR_DATA::ColorMapName;
using UTILS::NUM;

class RotatedColorMap : public ColorMapWrapper
{
public:
  RotatedColorMap(const std::shared_ptr<const IColorMap>& cm, float tRotatePoint);

  [[nodiscard]] auto GetColor(float t) const -> Pixel override;

private:
#if __cplusplus <= 201402L
  static const float MIN_ROTATE_POINT;
  static const float MAX_ROTATE_POINT;
#else
  static constexpr float MIN_ROTATE_POINT = 0.0F;
  static constexpr float MAX_ROTATE_POINT = 1.0F;
#endif
  const float m_tRotatePoint;
};

#if __cplusplus <= 201402L
const float RotatedColorMap::MIN_ROTATE_POINT = 0.0F;
const float RotatedColorMap::MAX_ROTATE_POINT = 1.0F;
#else
#endif

RotatedColorMap::RotatedColorMap(const std::shared_ptr<const IColorMap>& cm,
                                 const float tRotatePoint)
  : ColorMapWrapper{cm}, m_tRotatePoint{tRotatePoint}
{
  if (tRotatePoint < MIN_ROTATE_POINT)
  {
    throw std::logic_error(
        std20::format("Invalid rotate point {} < {}.", tRotatePoint, MIN_ROTATE_POINT));
  }
  if (tRotatePoint > MAX_ROTATE_POINT)
  {
    throw std::logic_error(
        std20::format("Invalid rotate point {} > {}.", tRotatePoint, MAX_ROTATE_POINT));
  }
}

inline auto RotatedColorMap::GetColor(const float t) const -> Pixel
{
  float tNew = m_tRotatePoint + t;
  if (tNew > 1.0F)
  {
    tNew = 1.0F - (tNew - 1.0F);
  }
  return GetColorMap()->GetColor(tNew);
}

class TintedColorMap : public ColorMapWrapper
{
public:
  TintedColorMap(const std::shared_ptr<const IColorMap>& cm, float saturation, float lightness);

  [[nodiscard]] auto GetColor(float t) const -> Pixel override;

private:
#if __cplusplus <= 201402L
  static const float MIN_LIGHTNESS;
  static const float MAX_LIGHTNESS;
#else
  static constexpr float MIN_LIGHTNESS = 0.1F;
  static constexpr float MAX_LIGHTNESS = 1.0F;
#endif
  const float m_saturation;
  const float m_lightness;
};

#if __cplusplus <= 201402L
const float TintedColorMap::MIN_LIGHTNESS = 0.1F;
const float TintedColorMap::MAX_LIGHTNESS = 1.0F;
#endif

TintedColorMap::TintedColorMap(const std::shared_ptr<const IColorMap>& cm,
                               const float saturation,
                               const float lightness)
  : ColorMapWrapper{cm}, m_saturation{saturation}, m_lightness{lightness}
{
  if (lightness < MIN_LIGHTNESS)
  {
    throw std::logic_error(std20::format("Invalid lightness {} < {}.", lightness, MIN_LIGHTNESS));
  }
  if (lightness > MAX_LIGHTNESS)
  {
    throw std::logic_error(std20::format("Invalid lightness {} > {}.", lightness, MAX_LIGHTNESS));
  }
}

auto TintedColorMap::GetColor(const float t) const -> Pixel
{
  const Pixel color = GetColorMap()->GetColor(t);

  vivid::hsv_t hsv{vivid::rgb::fromRgb32(color.Rgba())};
  hsv[1] = m_saturation;
  hsv[2] = m_lightness;

  return Pixel{vivid::Color{hsv}.rgb32()};
}

class PrebuiltColorMap : public IColorMap
{
public:
  PrebuiltColorMap() noexcept = delete;
  PrebuiltColorMap(ColorMapName mapName, vivid::ColorMap cm);
  PrebuiltColorMap(const PrebuiltColorMap&) noexcept = delete;
  PrebuiltColorMap(PrebuiltColorMap&& other) noexcept = default;
  ~PrebuiltColorMap() noexcept override = default;
  auto operator=(const PrebuiltColorMap&) -> PrebuiltColorMap& = delete;
  auto operator=(PrebuiltColorMap&&) -> PrebuiltColorMap& = delete;

  [[nodiscard]] auto GetNumStops() const -> size_t override { return m_cmap.numStops(); }
  [[nodiscard]] auto GetMapName() const -> ColorMapName override { return m_mapName; }
  [[nodiscard]] auto GetColor(float t) const -> Pixel override;

  static auto GetColorMix(const Pixel& col1, const Pixel& col2, float t) -> Pixel;

private:
  const ColorMapName m_mapName;
  const vivid::ColorMap m_cmap;
};

class ColorMaps::ColorMapsImpl
{
public:
  ColorMapsImpl() noexcept;
  ColorMapsImpl(const ColorMapsImpl&) noexcept = delete;
  ColorMapsImpl(ColorMapsImpl&&) noexcept = delete;
  virtual ~ColorMapsImpl() noexcept = default;
  auto operator=(const ColorMapsImpl&) -> ColorMapsImpl& = delete;
  auto operator=(ColorMapsImpl&&) -> const ColorMapsImpl&& = delete;

  [[nodiscard]] static auto GetNumColorMapNames() -> uint32_t;
  using ColorMapNames = std::vector<ColorMapName>;
  [[nodiscard]] static auto GetColorMapNames(ColorMapGroup groupName) -> const ColorMapNames&;

  [[nodiscard]] static auto GetColorMap(ColorMapName mapName) -> const IColorMap&;

  [[nodiscard]] static auto GetColorMapPtr(ColorMapName mapName)
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] static auto GetRotatedColorMapPtr(ColorMapName mapName, float tRotatePoint)
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] static auto GetRotatedColorMapPtr(const std::shared_ptr<const IColorMap>& cm,
                                                  float tRotatePoint)
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] static auto GetTintedColorMapPtr(ColorMapName mapName,
                                                 float saturation,
                                                 float lightness)
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] static auto GetTintedColorMapPtr(const std::shared_ptr<const IColorMap>& cm,
                                                 float saturation,
                                                 float lightness)
      -> std::shared_ptr<const IColorMap>;

  [[nodiscard]] static auto GetNumGroups() -> uint32_t;

private:
  [[nodiscard]] static auto GetAllColorMapNames() -> const std::vector<ColorMapName>&;
  [[nodiscard]] static auto MakeAllColorMapNames() -> std::vector<ColorMapName>;

  [[nodiscard]] static auto GetPreBuiltColorMaps() -> const std::vector<PrebuiltColorMap>&;
  [[nodiscard]] static auto MakePrebuiltColorMaps() -> std::vector<PrebuiltColorMap>;

  using ColorGroupNamesArray = std::array<const ColorMapNames*, NUM<ColorMapGroup>>;
  [[nodiscard]] static auto GetColorGroupNames() -> const ColorGroupNamesArray&;
  [[nodiscard]] static auto MakeColorGroupNames() -> ColorGroupNamesArray;
};

auto IColorMap::GetColorMix(const Pixel& col1, const Pixel& col2, const float t) -> Pixel
{
  return PrebuiltColorMap::GetColorMix(col1, col2, t);
}

ColorMaps::ColorMaps() noexcept : m_colorMapsImpl{spimpl::make_unique_impl<ColorMapsImpl>()}
{
}

auto ColorMaps::GetNumColorMapNames() const -> uint32_t
{
  return ColorMapsImpl::GetNumColorMapNames();
}

auto ColorMaps::GetColorMapNames(const ColorMapGroup cmg) const -> const ColorMaps::ColorMapNames&
{
  return ColorMapsImpl::GetColorMapNames(cmg);
}

auto ColorMaps::GetColorMap(const ColorMapName mapName) const -> const IColorMap&
{
  return ColorMapsImpl::GetColorMap(mapName);
}

auto ColorMaps::GetColorMapPtr(const ColorMapName mapName) const -> std::shared_ptr<const IColorMap>
{
  return ColorMapsImpl::GetColorMapPtr(mapName);
}

auto ColorMaps::GetRotatedColorMapPtr(const ColorMapName mapName, const float tRotatePoint) const
    -> std::shared_ptr<const IColorMap>
{
  return ColorMapsImpl::GetRotatedColorMapPtr(mapName, tRotatePoint);
}

auto ColorMaps::GetRotatedColorMapPtr(const std::shared_ptr<const IColorMap>& cm,
                                      const float tRotatePoint) const
    -> std::shared_ptr<const IColorMap>
{
  return ColorMapsImpl::GetRotatedColorMapPtr(cm, tRotatePoint);
}

auto ColorMaps::GetTintedColorMapPtr(const ColorMapName mapName,
                                     const float saturation,
                                     const float lightness) const
    -> std::shared_ptr<const IColorMap>
{
  return ColorMapsImpl::GetTintedColorMapPtr(mapName, saturation, lightness);
}

auto ColorMaps::GetTintedColorMapPtr(const std::shared_ptr<const IColorMap>& cm,
                                     const float saturation,
                                     const float lightness) const
    -> std::shared_ptr<const IColorMap>
{
  return std::make_shared<TintedColorMap>(cm, saturation, lightness);
}

auto ColorMaps::GetNumGroups() const -> uint32_t
{
  return ColorMapsImpl::GetNumGroups();
}


ColorMaps::ColorMapsImpl::ColorMapsImpl() noexcept = default;

inline auto ColorMaps::ColorMapsImpl::GetColorMap(const ColorMapName mapName) -> const IColorMap&
{
  return GetPreBuiltColorMaps().at(static_cast<size_t>(mapName));
}

// Wrap a raw pointer in a shared_ptr and make sure the raw pointer is never deleted.
static const auto MAKE_SHARED_ADDR = [](const IColorMap* const colorMap) {
  return std::shared_ptr<const IColorMap>{colorMap,
                                          []([[maybe_unused]] const IColorMap* const cm) {}};
};

inline auto ColorMaps::ColorMapsImpl::GetColorMapPtr(const ColorMapName mapName)
    -> std::shared_ptr<const IColorMap>
{
  return MAKE_SHARED_ADDR(&GetColorMap(mapName));
}

inline auto ColorMaps::ColorMapsImpl::GetRotatedColorMapPtr(const ColorMapName mapName,
                                                            const float tRotatePoint)
    -> std::shared_ptr<const IColorMap>
{
  return std::make_shared<RotatedColorMap>(MAKE_SHARED_ADDR(&GetColorMap(mapName)), tRotatePoint);
}

inline auto ColorMaps::ColorMapsImpl::GetRotatedColorMapPtr(
    const std::shared_ptr<const IColorMap>& cm, const float tRotatePoint)
    -> std::shared_ptr<const IColorMap>
{
  return std::make_shared<RotatedColorMap>(cm, tRotatePoint);
}

inline auto ColorMaps::ColorMapsImpl::GetTintedColorMapPtr(const ColorMapName mapName,
                                                           const float saturation,
                                                           const float lightness)
    -> std::shared_ptr<const IColorMap>
{
  return std::make_shared<TintedColorMap>(MAKE_SHARED_ADDR(&GetColorMap(mapName)), saturation,
                                          lightness);
}

inline auto ColorMaps::ColorMapsImpl::GetTintedColorMapPtr(
    const std::shared_ptr<const IColorMap>& cm, const float saturation, const float lightness)
    -> std::shared_ptr<const IColorMap>
{
  return std::make_shared<TintedColorMap>(cm, saturation, lightness);
}

inline auto ColorMaps::ColorMapsImpl::GetNumColorMapNames() -> uint32_t
{
  return static_cast<uint32_t>(GetPreBuiltColorMaps().size());
}

inline auto ColorMaps::ColorMapsImpl::GetColorMapNames(const ColorMapGroup groupName)
    -> const ColorMapNames&
{
  if (groupName == ColorMapGroup::ALL)
  {
    return GetAllColorMapNames();
  }

  return *at(GetColorGroupNames(), groupName);
}

auto ColorMaps::ColorMapsImpl::GetPreBuiltColorMaps() -> const std::vector<PrebuiltColorMap>&
{
  static const std::vector<PrebuiltColorMap> s_preBuiltColorMaps{MakePrebuiltColorMaps()};

  return s_preBuiltColorMaps;
}

auto ColorMaps::ColorMapsImpl::MakePrebuiltColorMaps() -> std::vector<PrebuiltColorMap>
{
  static_assert(NUM<ColorMapName> == COLOR_DATA::ALL_MAPS.size(), "Invalid allMaps size.");

  std::vector<PrebuiltColorMap> preBuiltColorMaps{};
  preBuiltColorMaps.reserve(COLOR_DATA::ALL_MAPS.size());

  for (const auto& map : COLOR_DATA::ALL_MAPS)
  {
    preBuiltColorMaps.emplace_back(map.colorMapName, *map.vividArray);
  }

  return preBuiltColorMaps;
}

inline auto ColorMaps::ColorMapsImpl::GetNumGroups() -> uint32_t
{
  return static_cast<uint32_t>(GetColorGroupNames().size());
}

auto ColorMaps::ColorMapsImpl::GetAllColorMapNames() -> const std::vector<ColorMapName>&
{
  static const std::vector<ColorMapName> s_allColorMapNames{MakeAllColorMapNames()};

  return s_allColorMapNames;
}

auto ColorMaps::ColorMapsImpl::MakeAllColorMapNames() -> std::vector<ColorMapName>
{
  std::vector<ColorMapName> allColorMapNames{};
  allColorMapNames.reserve(COLOR_DATA::ALL_MAPS.size());

  for (const auto& map : COLOR_DATA::ALL_MAPS)
  {
    allColorMapNames.emplace_back(map.colorMapName);
  }

  return allColorMapNames;
}

auto ColorMaps::ColorMapsImpl::GetColorGroupNames() -> const ColorGroupNamesArray&
{
  static const ColorGroupNamesArray s_colorGroupNames{MakeColorGroupNames()};

  return s_colorGroupNames;
}

auto ColorMaps::ColorMapsImpl::MakeColorGroupNames() -> ColorGroupNamesArray
{
  ColorGroupNamesArray groups{};

  at(groups, ColorMapGroup::ALL) = &GetAllColorMapNames();

  at(groups, ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL) =
      &COLOR_DATA::PERC_UNIF_SEQUENTIAL_MAPS;
  at(groups, ColorMapGroup::SEQUENTIAL) = &COLOR_DATA::SEQUENTIAL_MAPS;
  at(groups, ColorMapGroup::SEQUENTIAL2) = &COLOR_DATA::SEQUENTIAL2_MAPS;
  at(groups, ColorMapGroup::CYCLIC) = &COLOR_DATA::CYCLIC_MAPS;
  at(groups, ColorMapGroup::DIVERGING) = &COLOR_DATA::DIVERGING_MAPS;
  at(groups, ColorMapGroup::DIVERGING_BLACK) = &COLOR_DATA::DIVERGING_BLACK_MAPS;
  at(groups, ColorMapGroup::QUALITATIVE) = &COLOR_DATA::QUALITATIVE_MAPS;
  at(groups, ColorMapGroup::MISC) = &COLOR_DATA::MISC_MAPS;

  at(groups, ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM) =
      &COLOR_DATA::PERC_UNIF_SEQUENTIAL_SLIM_MAPS;
  at(groups, ColorMapGroup::SEQUENTIAL_SLIM) = &COLOR_DATA::SEQUENTIAL_SLIM_MAPS;
  at(groups, ColorMapGroup::SEQUENTIAL2_SLIM) = &COLOR_DATA::SEQUENTIAL2_SLIM_MAPS;
  at(groups, ColorMapGroup::CYCLIC_SLIM) = &COLOR_DATA::CYCLIC_SLIM_MAPS;
  at(groups, ColorMapGroup::DIVERGING_SLIM) = &COLOR_DATA::DIVERGING_SLIM_MAPS;
  at(groups, ColorMapGroup::DIVERGING_BLACK_SLIM) = &COLOR_DATA::DIVERGING_BLACK_SLIM_MAPS;
  at(groups, ColorMapGroup::QUALITATIVE_SLIM) = &COLOR_DATA::QUALITATIVE_SLIM_MAPS;
  at(groups, ColorMapGroup::MISC_SLIM) = &COLOR_DATA::MISC_SLIM_MAPS;

  at(groups, ColorMapGroup::WES_ANDERSON) = &COLOR_DATA::WES_ANDERSON_MAPS;
  at(groups, ColorMapGroup::BLUES) = &COLOR_DATA::BLUE_MAPS;
  at(groups, ColorMapGroup::REDS) = &COLOR_DATA::RED_MAPS;
  at(groups, ColorMapGroup::GREENS) = &COLOR_DATA::GREEN_MAPS;
  at(groups, ColorMapGroup::YELLOWS) = &COLOR_DATA::YELLOW_MAPS;
  at(groups, ColorMapGroup::ORANGES) = &COLOR_DATA::ORANGE_MAPS;
  at(groups, ColorMapGroup::PURPLES) = &COLOR_DATA::PURPLE_MAPS;
  at(groups, ColorMapGroup::CITIES) = &COLOR_DATA::CITY_MAPS;
  at(groups, ColorMapGroup::SEASONS) = &COLOR_DATA::SEASON_MAPS;
  at(groups, ColorMapGroup::HEAT) = &COLOR_DATA::HEAT_MAPS;
  at(groups, ColorMapGroup::COLD) = &COLOR_DATA::COLD_MAPS;
  at(groups, ColorMapGroup::PASTEL) = &COLOR_DATA::PASTEL_MAPS;

  assert(std::all_of(groups.cbegin(), groups.cend(), [](const auto& g) { return g != nullptr; }));

  return groups;
}

inline PrebuiltColorMap::PrebuiltColorMap(const ColorMapName mapName, vivid::ColorMap cm)
  : m_mapName{mapName}, m_cmap{std::move(cm)}
{
}

inline auto PrebuiltColorMap::GetColor(const float t) const -> Pixel
{
  return Pixel{vivid::Color{m_cmap.at(t)}.rgb32()};

  /**
  // Optimise for RGB
  if (m_cmap.empty()) {
    return Pixel::BLACK;
  }

  const size_t numStops = m_cmap.numStops();
  t = stdnew::clamp(t, 0.0F, 1.0F);
  const float stopIndexFlt = t * static_cast<float>(numStops);
  const auto stopIndex = static_cast<size_t>(stopIndexFlt);

  if (stopIndex + 1 == numStops) {
    return Pixel{vivid::Color{m_cmap.stops().back()}.rgb32()};
  }

  const float u = stopIndexFlt - static_cast<float>(stopIndex);
  const Pixel color1 = Pixel{vivid::Color{m_cmap.stops()[stopIndex]}.rgb32()};
  const Pixel color2 = Pixel{vivid::Color{m_cmap.stops()[stopIndex + 1]}.rgb32()};
  return GetRgbColorLerp(color1, color2, u);
**/
}

inline auto PrebuiltColorMap::GetColorMix(const Pixel& col1, const Pixel& col2, const float t)
    -> Pixel
{
  // Optimisation: faster to use this lesser quality RGB Lerp than vivid's Lerp.
  return GetRgbColorLerp(col1, col2, t);
  /**
  const vivid::rgb_t c1 = vivid::rgb::fromRgb32(col1.Rgba());
  const vivid::rgb_t c2 = vivid::rgb::fromRgb32(col2.Rgba());
  return Pixel{vivid::lerpHsl(c1, c2, t).rgb32()};
   **/
}

#if __cplusplus <= 201402L
} // namespace COLOR
} // namespace GOOM
#else
} // namespace GOOM::COLOR
#endif
