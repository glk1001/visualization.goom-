#include "colormaps.h"

#include "color_data/color_data_maps.h"
#include "color_data/colormap_enums.h"
#include "color_data/extra_color_groups.h"
#include "colorutils.h"
#include "enumutils.h"
#include "goom/goom_graphic.h"
#include "goomutils/logging_control.h"
//#undef NO_LOGGING
#include "goomutils/logging.h"
#include "goomutils/spimpl.h"

#include <algorithm>
#undef NDEBUG
#include <cassert>
#include <format>
#include <utility>
#include <vector>
#include <vivid/vivid.h>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace UTILS
{
#else
namespace GOOM::UTILS
{
#endif

using COLOR_DATA::ColorMapName;

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

protected:
  using GroupColorNames = std::array<const ColorMapNames*, NUM<ColorMapGroup>>;
  [[nodiscard]] static auto GetGroups() -> const GroupColorNames& { return s_groups; }
  static void InitGroups();

private:
  static std::vector<PrebuiltColorMap> s_preBuiltColorMaps;
  static void InitPrebuiltColorMaps();
  static GroupColorNames s_groups;
  static std::vector<ColorMapName> s_allColorMapNames;
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

std::vector<PrebuiltColorMap> ColorMaps::ColorMapsImpl::s_preBuiltColorMaps{};
ColorMaps::ColorMapsImpl::GroupColorNames ColorMaps::ColorMapsImpl::s_groups{nullptr};
std::vector<ColorMapName> ColorMaps::ColorMapsImpl::s_allColorMapNames{};

ColorMaps::ColorMapsImpl::ColorMapsImpl() noexcept = default;

inline auto ColorMaps::ColorMapsImpl::GetColorMap(const ColorMapName mapName) -> const IColorMap&
{
  InitPrebuiltColorMaps();
  return s_preBuiltColorMaps.at(static_cast<size_t>(mapName));
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
  InitGroups();
  InitPrebuiltColorMaps();
  return s_preBuiltColorMaps.size();
}

inline auto ColorMaps::ColorMapsImpl::GetColorMapNames(const ColorMapGroup groupName)
    -> const ColorMapNames&
{
  InitGroups();

  if (groupName == ColorMapGroup::ALL)
  {
    return s_allColorMapNames;
  }

  return *at(s_groups, groupName);
}

void ColorMaps::ColorMapsImpl::InitPrebuiltColorMaps()
{
  if (!s_preBuiltColorMaps.empty())
  {
    return;
  }
  static_assert(NUM<ColorMapName> == COLOR_DATA::allMaps.size(), "Invalid allMaps size.");

  s_preBuiltColorMaps.reserve(COLOR_DATA::allMaps.size());

  for (const auto& maps : COLOR_DATA::allMaps)
  {
    const ColorMapName name = maps.first;
    const std::vector<vivid::srgb_t>& vividArray = maps.second;
    (void)s_preBuiltColorMaps.emplace_back(name, vividArray);
  }
}

inline auto ColorMaps::ColorMapsImpl::GetNumGroups() -> uint32_t
{
  InitGroups();
  return s_groups.size();
}

void ColorMaps::ColorMapsImpl::InitGroups()
{
  if (s_groups[0] != nullptr && !s_allColorMapNames.empty())
  {
    return;
  }

  s_allColorMapNames.reserve(COLOR_DATA::allMaps.size());
  for (const auto& maps : COLOR_DATA::allMaps)
  {
    s_allColorMapNames.emplace_back(maps.first);
  }

  at(s_groups, ColorMapGroup::ALL) = &s_allColorMapNames;

  at(s_groups, ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL) =
      &COLOR_DATA::perc_unif_sequentialMaps;
  at(s_groups, ColorMapGroup::SEQUENTIAL) = &COLOR_DATA::sequentialMaps;
  at(s_groups, ColorMapGroup::SEQUENTIAL2) = &COLOR_DATA::sequential2Maps;
  at(s_groups, ColorMapGroup::CYCLIC) = &COLOR_DATA::cyclicMaps;
  at(s_groups, ColorMapGroup::DIVERGING) = &COLOR_DATA::divergingMaps;
  at(s_groups, ColorMapGroup::DIVERGING_BLACK) = &COLOR_DATA::diverging_blackMaps;
  at(s_groups, ColorMapGroup::QUALITATIVE) = &COLOR_DATA::qualitativeMaps;
  at(s_groups, ColorMapGroup::MISC) = &COLOR_DATA::miscMaps;

  at(s_groups, ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM) =
      &COLOR_DATA::perc_unif_sequential_slimMaps;
  at(s_groups, ColorMapGroup::SEQUENTIAL_SLIM) = &COLOR_DATA::sequential_slimMaps;
  at(s_groups, ColorMapGroup::SEQUENTIAL2_SLIM) = &COLOR_DATA::sequential2_slimMaps;
  at(s_groups, ColorMapGroup::CYCLIC_SLIM) = &COLOR_DATA::cyclic_slimMaps;
  at(s_groups, ColorMapGroup::DIVERGING_SLIM) = &COLOR_DATA::diverging_slimMaps;
  at(s_groups, ColorMapGroup::DIVERGING_BLACK_SLIM) = &COLOR_DATA::diverging_black_slimMaps;
  at(s_groups, ColorMapGroup::QUALITATIVE_SLIM) = &COLOR_DATA::qualitative_slimMaps;
  at(s_groups, ColorMapGroup::MISC_SLIM) = &COLOR_DATA::misc_slimMaps;

  at(s_groups, ColorMapGroup::WES_ANDERSON) = &COLOR_DATA::wesAndersonMaps;
  at(s_groups, ColorMapGroup::BLUES) = &COLOR_DATA::blueMaps;
  at(s_groups, ColorMapGroup::REDS) = &COLOR_DATA::redMaps;
  at(s_groups, ColorMapGroup::GREENS) = &COLOR_DATA::greenMaps;
  at(s_groups, ColorMapGroup::YELLOWS) = &COLOR_DATA::yellowMaps;
  at(s_groups, ColorMapGroup::ORANGES) = &COLOR_DATA::orangeMaps;
  at(s_groups, ColorMapGroup::PURPLES) = &COLOR_DATA::purpleMaps;
  at(s_groups, ColorMapGroup::CITIES) = &COLOR_DATA::cityMaps;
  at(s_groups, ColorMapGroup::SEASONS) = &COLOR_DATA::seasonMaps;
  at(s_groups, ColorMapGroup::HEAT) = &COLOR_DATA::heatMaps;
  at(s_groups, ColorMapGroup::COLD) = &COLOR_DATA::coldMaps;
  at(s_groups, ColorMapGroup::PASTEL) = &COLOR_DATA::pastelMaps;

  assert(
      std::all_of(s_groups.cbegin(), s_groups.cend(), [](const auto& g) { return g != nullptr; }));
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
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS
#endif
