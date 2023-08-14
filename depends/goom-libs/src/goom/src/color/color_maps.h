#pragma once

#include "color_data/color_map_enums.h"
#include "color_maps_base.h"
#include "goom_graphic.h"
#include "utils/enum_utils.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#ifndef NO_MAGIC_ENUM_AVAILABLE
#include <magic_enum.hpp>
#endif

#ifndef NO_MAGIC_ENUM_AVAILABLE
template<>
struct magic_enum::customize::enum_range<GOOM::COLOR::COLOR_DATA::ColorMapName>
{
  static constexpr int32_t min = -1; // NOLINT(readability-identifier-naming)
  // NOLINTNEXTLINE(readability-identifier-naming)
  static constexpr int32_t max = GOOM::UTILS::NUM<GOOM::COLOR::COLOR_DATA::ColorMapName>;
};
#endif

namespace GOOM::COLOR
{

class ColorMapPtrWrapper : public IColorMap
{
public:
  explicit ColorMapPtrWrapper(const IColorMap* colorMap,
                              PixelChannelType defaultAlpha = MAX_ALPHA) noexcept;
  ColorMapPtrWrapper(const ColorMapPtrWrapper&) noexcept           = default;
  ColorMapPtrWrapper(ColorMapPtrWrapper&&) noexcept                = default;
  ~ColorMapPtrWrapper() noexcept override                          = default;
  auto operator=(const ColorMapPtrWrapper&) -> ColorMapPtrWrapper& = default;
  auto operator=(ColorMapPtrWrapper&&) -> ColorMapPtrWrapper&      = default;

  [[nodiscard]] auto GetNumStops() const noexcept -> size_t override;
  [[nodiscard]] auto GetMapName() const noexcept -> COLOR_DATA::ColorMapName override;
  [[nodiscard]] auto GetColor(float t) const noexcept -> Pixel override;

  [[nodiscard]] auto IsNotNull() const noexcept -> bool;

protected:
  [[nodiscard]] auto GetColorMap() const noexcept -> const IColorMap& { return *m_colorMap; }
  [[nodiscard]] auto GetDefaultAlpha() const noexcept -> PixelChannelType { return m_defaultAlpha; }

private:
  const IColorMap* m_colorMap;
  PixelChannelType m_defaultAlpha;
};

enum class ColorMapGroup
{
  ALL, // all possible maps
  PERCEPTUALLY_UNIFORM_SEQUENTIAL,
  SEQUENTIAL,
  SEQUENTIAL2,
  CYCLIC,
  DIVERGING,
  DIVERGING_BLACK,
  QUALITATIVE,
  MISC,
  PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM,
  SEQUENTIAL_SLIM,
  SEQUENTIAL2_SLIM,
  CYCLIC_SLIM,
  DIVERGING_SLIM,
  DIVERGING_BLACK_SLIM,
  QUALITATIVE_SLIM,
  MISC_SLIM,
  WES_ANDERSON,
  BLUES,
  REDS,
  GREENS,
  YELLOWS,
  ORANGES,
  PURPLES,
  CITIES,
  SEASONS,
  HEAT,
  COLD,
  PASTEL,
  _num // unused, and marks the enum end
};

using ColorMapSharedPtr = std::shared_ptr<const IColorMap>;

class ColorMaps
{
public:
  ColorMaps() noexcept = default;
  explicit ColorMaps(PixelChannelType defaultAlpha) noexcept;

  [[nodiscard]] static auto GetColorMix(const Pixel& color1, const Pixel& color2, float t) noexcept
      -> Pixel;

  [[nodiscard]] static auto GetNumGroups() noexcept -> uint32_t;
  [[nodiscard]] static auto GetNumColorMapNames() noexcept -> uint32_t;
  [[nodiscard]] static auto GetColorMapNames(ColorMapGroup colorMapGroup) noexcept
      -> const std::vector<COLOR_DATA::ColorMapName>&;

  [[nodiscard]] auto GetColorMap(COLOR_DATA::ColorMapName colorMapName) const noexcept
      -> ColorMapPtrWrapper;

  [[nodiscard]] auto GetRotatedColorMapPtr(COLOR_DATA::ColorMapName colorMapName,
                                           float tRotatePoint) const noexcept -> ColorMapSharedPtr;
  [[nodiscard]] auto GetRotatedColorMapPtr(const ColorMapSharedPtr& colorMapPtr,
                                           float tRotatePoint) const noexcept -> ColorMapSharedPtr;

  struct TintProperties
  {
    float saturation;
    float lightness;
  };
  [[nodiscard]] auto GetTintedColorMapPtr(COLOR_DATA::ColorMapName colorMapName,
                                          const TintProperties& tintProperties) const noexcept
      -> ColorMapSharedPtr;
  [[nodiscard]] auto GetTintedColorMapPtr(const ColorMapSharedPtr& colorMapPtr,
                                          const TintProperties& tintProperties) const noexcept
      -> ColorMapSharedPtr;

private:
  PixelChannelType m_defaultAlpha = MAX_ALPHA;

  friend class RandomColorMaps;
  [[nodiscard]] static auto GetColorMapSharedPtr(COLOR_DATA::ColorMapName colorMapName) noexcept
      -> ColorMapSharedPtr;
};

inline ColorMapPtrWrapper::ColorMapPtrWrapper(const IColorMap* const colorMap,
                                              const PixelChannelType defaultAlpha) noexcept
  : m_colorMap{colorMap}, m_defaultAlpha{defaultAlpha}
{
}

inline auto ColorMapPtrWrapper::GetNumStops() const noexcept -> size_t
{
  return m_colorMap->GetNumStops();
}

inline auto ColorMapPtrWrapper::GetMapName() const noexcept -> COLOR_DATA::ColorMapName
{
  return m_colorMap->GetMapName();
}

inline auto ColorMapPtrWrapper::GetColor(const float t) const noexcept -> Pixel
{
  const auto color = m_colorMap->GetColor(t);
  return Pixel{
      {color.R(), color.G(), color.B(), m_defaultAlpha}
  };
}

inline auto ColorMapPtrWrapper::IsNotNull() const noexcept -> bool
{
  return m_colorMap != nullptr;
}

} // namespace GOOM::COLOR
