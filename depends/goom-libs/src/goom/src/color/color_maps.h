#pragma once

#include "color_data/color_map_enums.h"
#include "goom_graphic.h"
#include "spimpl.h"
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

class IColorMap
{
public:
  IColorMap() noexcept                           = default;
  IColorMap(const IColorMap&) noexcept           = default;
  IColorMap(IColorMap&&) noexcept                = default;
  virtual ~IColorMap() noexcept                  = default;
  auto operator=(const IColorMap&) -> IColorMap& = default;
  auto operator=(IColorMap&&) -> IColorMap&      = default;

  [[nodiscard]] virtual auto GetNumStops() const -> size_t                  = 0;
  [[nodiscard]] virtual auto GetMapName() const -> COLOR_DATA::ColorMapName = 0;

  [[nodiscard]] virtual auto GetColor(float t) const -> Pixel = 0;

  static auto GetColorMix(const Pixel& col1, const Pixel& col2, float t) -> Pixel;

private:
  friend class ColorMaps;
};

class ColorMapPtrWrapper : public IColorMap
{
public:
  ColorMapPtrWrapper(const IColorMap* colorMap, PixelChannelType defaultAlpha) noexcept;
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

class ColorMapSharedPtrWrapper : public IColorMap
{
public:
  ColorMapSharedPtrWrapper(const std::shared_ptr<const IColorMap>& colorMap,
                           PixelChannelType defaultAlpha) noexcept;
  ColorMapSharedPtrWrapper(const ColorMapSharedPtrWrapper&) noexcept           = default;
  ColorMapSharedPtrWrapper(ColorMapSharedPtrWrapper&&) noexcept                = default;
  ~ColorMapSharedPtrWrapper() noexcept override                                = default;
  auto operator=(const ColorMapSharedPtrWrapper&) -> ColorMapSharedPtrWrapper& = default;
  auto operator=(ColorMapSharedPtrWrapper&&) -> ColorMapSharedPtrWrapper&      = default;

  [[nodiscard]] auto GetNumStops() const -> size_t override;
  [[nodiscard]] auto GetMapName() const -> COLOR_DATA::ColorMapName override;
  [[nodiscard]] auto GetColor(float t) const -> Pixel override;

  [[nodiscard]] auto IsNotNull() const -> bool;

protected:
  [[nodiscard]] auto GetColorMap() const -> const IColorMap& { return *m_colorMap; }
  [[nodiscard]] auto GetColorMapPtr() const -> const std::shared_ptr<const IColorMap>&
  {
    return m_colorMap;
  }
  [[nodiscard]] auto GetDefaultAlpha() const -> PixelChannelType { return m_defaultAlpha; }

private:
  std::shared_ptr<const IColorMap> m_colorMap;
  PixelChannelType m_defaultAlpha;
  friend class RandomColorMaps;
};

enum class ColorMapGroup : int
{
  ALL = 0, // all possible maps
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

class ColorMaps
{
public:
  explicit ColorMaps(PixelChannelType defaultAlpha) noexcept;

  auto SetDefaultAlpha(PixelChannelType defaultAlpha) noexcept -> void;

  [[nodiscard]] static auto GetNumColorMapNames() -> uint32_t;
  using ColorMapNames = std::vector<COLOR_DATA::ColorMapName>;
  [[nodiscard]] static auto GetColorMapNames(ColorMapGroup cmg) -> const ColorMapNames&;

  [[nodiscard]] auto GetColorMap(COLOR_DATA::ColorMapName mapName) const noexcept
      -> ColorMapPtrWrapper;

  [[nodiscard]] auto GetColorMapSharedPtr(COLOR_DATA::ColorMapName mapName) const noexcept
      -> ColorMapSharedPtrWrapper;

  [[nodiscard]] auto GetRotatedColorMapPtr(COLOR_DATA::ColorMapName mapName,
                                           float tRotatePoint) const noexcept
      -> ColorMapSharedPtrWrapper;
  [[nodiscard]] auto GetRotatedColorMapPtr(const std::shared_ptr<const IColorMap>& cm,
                                           float tRotatePoint) const noexcept
      -> ColorMapSharedPtrWrapper;

  struct TintProperties
  {
    float saturation;
    float lightness;
  };
  [[nodiscard]] auto GetTintedColorMapPtr(COLOR_DATA::ColorMapName mapName,
                                          const TintProperties& tintProperties) const noexcept
      -> ColorMapSharedPtrWrapper;
  [[nodiscard]] auto GetTintedColorMapPtr(const std::shared_ptr<const IColorMap>& colorMap,
                                          const TintProperties& tintProperties) const noexcept
      -> ColorMapSharedPtrWrapper;

  [[nodiscard]] static auto GetNumGroups() -> uint32_t;

private:
  class ColorMapsImpl;
  spimpl::unique_impl_ptr<ColorMapsImpl> m_pimpl;

  friend class RandomColorMaps;
  [[nodiscard]] auto GetColorMapSharedPtr1(COLOR_DATA::ColorMapName mapName) const noexcept
      -> std::shared_ptr<const IColorMap>;
};

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
  return m_colorMap->GetColor(t);
}

inline auto ColorMapPtrWrapper::IsNotNull() const -> bool
{
  return m_colorMap != nullptr;
}

inline ColorMapSharedPtrWrapper::ColorMapSharedPtrWrapper(
    const std::shared_ptr<const IColorMap>& colorMap, const PixelChannelType defaultAlpha) noexcept
  : m_colorMap{colorMap}, m_defaultAlpha{defaultAlpha}
{
}

inline auto ColorMapSharedPtrWrapper::GetNumStops() const -> size_t
{
  return m_colorMap->GetNumStops();
}

inline auto ColorMapSharedPtrWrapper::GetMapName() const -> COLOR_DATA::ColorMapName
{
  return m_colorMap->GetMapName();
}

inline auto ColorMapSharedPtrWrapper::GetColor(const float t) const -> Pixel
{
  return m_colorMap->GetColor(t);
}

inline auto ColorMapSharedPtrWrapper::IsNotNull() const -> bool
{
  return m_colorMap != nullptr;
}

} // namespace GOOM::COLOR
