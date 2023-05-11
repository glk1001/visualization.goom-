#pragma once

#include "color_data/color_map_enums.h"
#include "color_maps_base.h"
#include "goom_graphic.h"
#include "utils/enum_utils.h"

#include <cstdint>
#include <memory>
#include <utility>

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

  [[nodiscard]] auto GetNumStops() const -> size_t override;
  [[nodiscard]] auto GetMapName() const -> COLOR_DATA::ColorMapName override;
  [[nodiscard]] auto GetColor(float t) const -> Pixel override;

  [[nodiscard]] auto IsNotNull() const -> bool;

protected:
  [[nodiscard]] auto GetColorMap() const -> const IColorMap& { return *m_colorMap; }
  [[nodiscard]] auto GetDefaultAlpha() const -> PixelChannelType { return m_defaultAlpha; }

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
  explicit ColorMaps(PixelChannelType defaultAlpha) noexcept;

  auto SetDefaultAlpha(PixelChannelType defaultAlpha) noexcept -> void;

  static auto GetColorMix(const Pixel& color1, const Pixel& color2, float t) -> Pixel;

  [[nodiscard]] static auto GetNumColorMapNames() -> uint32_t;
  using ColorMapNames = std::vector<COLOR_DATA::ColorMapName>;
  [[nodiscard]] static auto GetColorMapNames(ColorMapGroup colorMapGroup) -> const ColorMapNames&;

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

  [[nodiscard]] static auto GetNumGroups() -> uint32_t;

private:
  PixelChannelType m_defaultAlpha;

  friend class RandomColorMaps;
  [[nodiscard]] static auto GetColorMapSharedPtr(COLOR_DATA::ColorMapName colorMapName) noexcept
      -> ColorMapSharedPtr;
};

//constexpr size_t to_int(const ColorMapGroup i) { return static_cast<size_t>(i); }
template<class T>
constexpr const T& at(const std::array<T, UTILS::NUM<ColorMapGroup>>& arr, const ColorMapGroup idx)
{
  return arr.at(static_cast<size_t>(idx));
}

template<class T>
constexpr T& at(std::array<T, UTILS::NUM<ColorMapGroup>>& arr, const ColorMapGroup idx)
{
  return arr.at(static_cast<size_t>(idx));
}

inline ColorMapPtrWrapper::ColorMapPtrWrapper(const IColorMap* const colorMap,
                                              const PixelChannelType defaultAlpha) noexcept
  : m_colorMap{colorMap}, m_defaultAlpha{defaultAlpha}
{
}

inline auto ColorMapPtrWrapper::GetNumStops() const -> size_t
{
  return m_colorMap->GetNumStops();
}

inline auto ColorMapPtrWrapper::GetMapName() const -> COLOR_DATA::ColorMapName
{
  return m_colorMap->GetMapName();
}

inline auto ColorMapPtrWrapper::GetColor(const float t) const -> Pixel
{
  const auto color = m_colorMap->GetColor(t);
  return Pixel{
      {color.R(), color.G(), color.B(), m_defaultAlpha}
  };
}

inline auto ColorMapPtrWrapper::IsNotNull() const -> bool
{
  return m_colorMap != nullptr;
}

} // namespace GOOM::COLOR
