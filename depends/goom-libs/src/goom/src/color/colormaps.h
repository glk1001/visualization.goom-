#pragma once

#include "color_data/colormap_enums.h"
#include "goom/goom_graphic.h"
#include "utils/enumutils.h"

#include <cstdint>
#include <memory>
#include <utility>

namespace GOOM::COLOR
{

class IColorMap
{
public:
  IColorMap() = default;
  IColorMap(const IColorMap&) = default;
  IColorMap(IColorMap&&) = default;
  virtual ~IColorMap() noexcept = default;
  auto operator=(const IColorMap&) -> IColorMap& = delete;
  auto operator=(IColorMap&&) -> IColorMap& = delete;

  [[nodiscard]] virtual auto GetNumStops() const -> size_t = 0;
  [[nodiscard]] virtual auto GetMapName() const -> COLOR_DATA::ColorMapName = 0;

  [[nodiscard]] virtual auto GetColor(float t) const -> Pixel = 0;

  static auto GetColorMix(const Pixel& col1, const Pixel& col2, float t) -> Pixel;

private:
  friend class ColorMaps;
};

class ColorMapWrapper : public IColorMap
{
public:
  ColorMapWrapper() noexcept = delete;
  explicit ColorMapWrapper(std::shared_ptr<const IColorMap> colorMap) noexcept;
  ColorMapWrapper(const ColorMapWrapper&) noexcept = delete;
  ColorMapWrapper(ColorMapWrapper&&) noexcept = delete;
  ~ColorMapWrapper() noexcept override = default;
  auto operator=(const ColorMapWrapper&) -> ColorMapWrapper& = delete;
  auto operator=(ColorMapWrapper&&) -> ColorMapWrapper& = delete;

  [[nodiscard]] auto GetNumStops() const -> size_t override;
  [[nodiscard]] auto GetMapName() const -> COLOR_DATA::ColorMapName override;
  [[nodiscard]] auto GetColor(float t) const -> Pixel override;

protected:
  [[nodiscard]] auto GetColorMap() const -> std::shared_ptr<const IColorMap> { return m_colorMap; }

private:
  const std::shared_ptr<const IColorMap> m_colorMap;
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
  _num // unused and must be last
};

//constexpr size_t to_int(const ColorMapGroup i) { return static_cast<size_t>(i); }
template<class T>
constexpr const T& at(const std::array<T, UTILS::NUM<ColorMapGroup>>& arr, const ColorMapGroup idx)
{
  return arr.at(static_cast<size_t>(idx));
  //  return arr[static_cast<size_t>(idx)];
}

template<class T>
constexpr T& at(std::array<T, UTILS::NUM<ColorMapGroup>>& arr, const ColorMapGroup idx)
{
  return arr.at(static_cast<size_t>(idx));
  //  return arr[static_cast<size_t>(idx)];
}

class ColorMaps
{
public:
  ColorMaps() noexcept;
  ColorMaps(const ColorMaps&) noexcept = delete;
  ColorMaps(ColorMaps&&) noexcept = delete;
  virtual ~ColorMaps() noexcept = default;
  auto operator=(const ColorMaps&) -> ColorMaps& = delete;
  auto operator=(ColorMaps&&) -> ColorMaps& = delete;

  [[nodiscard]] auto GetNumColorMapNames() const -> uint32_t;
  using ColorMapNames = std::vector<COLOR_DATA::ColorMapName>;
  [[nodiscard]] auto GetColorMapNames(ColorMapGroup cmg) const -> const ColorMapNames&;

  [[nodiscard]] auto GetColorMap(COLOR_DATA::ColorMapName mapName) const -> const IColorMap&;

  [[nodiscard]] auto GetColorMapPtr(COLOR_DATA::ColorMapName mapName) const
      -> std::shared_ptr<const IColorMap>;

  [[nodiscard]] auto GetRotatedColorMapPtr(COLOR_DATA::ColorMapName mapName,
                                           float tRotatePoint) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetRotatedColorMapPtr(const std::shared_ptr<const IColorMap>& cm,
                                           float tRotatePoint) const
      -> std::shared_ptr<const IColorMap>;

  [[nodiscard]] auto GetTintedColorMapPtr(COLOR_DATA::ColorMapName mapName,
                                          float saturation,
                                          float lightness) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetTintedColorMapPtr(const std::shared_ptr<const IColorMap>& cm,
                                          float saturation,
                                          float lightness) const
      -> std::shared_ptr<const IColorMap>;

  [[nodiscard]] auto GetNumGroups() const -> uint32_t;

private:
  class ColorMapsImpl;
};

inline ColorMapWrapper::ColorMapWrapper(std::shared_ptr<const IColorMap> colorMap) noexcept
  : m_colorMap{std::move(colorMap)}
{
}

inline auto ColorMapWrapper::GetNumStops() const -> size_t
{
  return m_colorMap->GetNumStops();
}

inline auto ColorMapWrapper::GetMapName() const -> COLOR_DATA::ColorMapName
{
  return m_colorMap->GetMapName();
}

inline auto ColorMapWrapper::GetColor(const float t) const -> Pixel
{
  return m_colorMap->GetColor(t);
}

} // namespace GOOM::COLOR
