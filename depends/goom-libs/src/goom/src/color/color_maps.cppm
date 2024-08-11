module;

#include "color_data/color_map_enums.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

export module Goom.Color.ColorMaps;

import Goom.Color.ColorMapBase;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;

export namespace GOOM::COLOR
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

enum class ColorMapGroup : UnderlyingEnumType
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
};

using ConstColorMapSharedPtr = std::shared_ptr<const IColorMap>;

class ColorMaps
{
public:
  ColorMaps() noexcept = default;
  explicit ColorMaps(PixelChannelType defaultAlpha) noexcept;

  [[nodiscard]] static auto GetColorMix(const Pixel& color1,
                                        const Pixel& color2,
                                        float t) noexcept -> Pixel;

  [[nodiscard]] static auto GetNumGroups() noexcept -> uint32_t;
  [[nodiscard]] static auto GetNumColorMapNames() noexcept -> uint32_t;
  [[nodiscard]] static auto GetColorMapNames(ColorMapGroup colorMapGroup) noexcept
      -> const std::vector<COLOR_DATA::ColorMapName>&;

  [[nodiscard]] auto GetColorMap(COLOR_DATA::ColorMapName colorMapName) const noexcept
      -> ColorMapPtrWrapper;

  [[nodiscard]] auto GetRotatedColorMapPtr(COLOR_DATA::ColorMapName colorMapName,
                                           float tRotatePoint) const noexcept
      -> ConstColorMapSharedPtr;
  [[nodiscard]] auto GetRotatedColorMapPtr(const ConstColorMapSharedPtr& colorMapPtr,
                                           float tRotatePoint) const noexcept
      -> ConstColorMapSharedPtr;

  struct TintProperties
  {
    float saturation;
    float lightness;
  };
  [[nodiscard]] auto GetTintedColorMapPtr(COLOR_DATA::ColorMapName colorMapName,
                                          const TintProperties& tintProperties) const noexcept
      -> ConstColorMapSharedPtr;
  [[nodiscard]] auto GetTintedColorMapPtr(const ConstColorMapSharedPtr& colorMapPtr,
                                          const TintProperties& tintProperties) const noexcept
      -> ConstColorMapSharedPtr;

  [[nodiscard]] static auto GetColorMapSharedPtr(COLOR_DATA::ColorMapName colorMapName) noexcept
      -> ConstColorMapSharedPtr;

private:
  PixelChannelType m_defaultAlpha = MAX_ALPHA;
};

} // namespace GOOM::COLOR

namespace GOOM::COLOR
{

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
      {.red = color.R(), .green = color.G(), .blue = color.B(), .alpha = m_defaultAlpha}
  };
}

inline auto ColorMapPtrWrapper::IsNotNull() const noexcept -> bool
{
  return m_colorMap != nullptr;
}

} // namespace GOOM::COLOR
