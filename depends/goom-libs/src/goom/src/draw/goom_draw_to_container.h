#pragma once

#include "goom_draw.h"
#include "goom_graphic.h"
#include "goom_types.h"
#include "point2d.h"

#include <array>
#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

class GoomDrawToContainer : public IGoomDraw
{
public:
  GoomDrawToContainer() noexcept = delete;
  explicit GoomDrawToContainer(const Dimensions& dimensions);
  GoomDrawToContainer(const GoomDrawToContainer&) noexcept = delete;
  GoomDrawToContainer(GoomDrawToContainer&&) noexcept      = delete;
  ~GoomDrawToContainer() noexcept override;
  auto operator=(const GoomDrawToContainer&) noexcept -> GoomDrawToContainer& = delete;
  auto operator=(GoomDrawToContainer&&) noexcept -> GoomDrawToContainer&      = delete;

  auto GetPixel(Point2dInt point) const -> Pixel override;
  void DrawPixelsUnblended(Point2dInt point, const MultiplePixels& colors) override;

  auto GetPixels(Point2dInt point) const -> MultiplePixels;

  static constexpr size_t MAX_NUM_COLORS_LIST = 3;
  using ColorsArray                           = std::array<Pixel, MAX_NUM_COLORS_LIST>;
  struct ColorsList
  {
    uint8_t count = 0;
    ColorsArray colorsArray{};
  };
  [[nodiscard]] auto GetNumChangedCoords() const -> size_t;
  [[nodiscard]] auto GetChangedCoordsList() const -> const std::vector<Point2dInt>&;
  // IMPORTANT: The above is ordered from oldest to newest.
  [[nodiscard]] auto GetColorsList(Point2dInt point) const -> const ColorsList&;

  using CoordsFunc = std::function<void(Point2dInt point, const ColorsList& colorsList)>;
  // NOTE: 'func' must be thread-safe.
  void IterateChangedCoordsNewToOld(const CoordsFunc& func) const;

  void ResizeChangedCoordsKeepingNewest(size_t numToKeep);
  void ClearAll();

protected:
  void DrawPixelsToDevice(Point2dInt point,
                          const MultiplePixels& colors,
                          uint32_t intBuffIntensity) override;

private:
  std::vector<std::vector<ColorsList>> m_xyPixelList{};
  std::vector<Point2dInt> m_orderedXYPixelList{};
  [[nodiscard]] auto GetWriteableColorsList(Point2dInt point) -> ColorsList&;
  [[nodiscard]] auto GetLastDrawnColor(Point2dInt point) const -> Pixel;
  [[nodiscard]] auto GetLastDrawnColors(Point2dInt point) const -> MultiplePixels;
};

inline auto GoomDrawToContainer::GetPixel(const Point2dInt point) const -> Pixel
{
  return GetLastDrawnColor(point);
}

inline auto GoomDrawToContainer::GetPixels(const Point2dInt point) const -> MultiplePixels
{
  return GetLastDrawnColors(point);
}

inline auto GoomDrawToContainer::GetLastDrawnColor(const Point2dInt point) const -> Pixel
{
  const auto& colorsList = GetColorsList(point);
  if (0 == colorsList.count)
  {
    return BLACK_PIXEL;
  }
  return colorsList.colorsArray[static_cast<size_t>(colorsList.count - 1)];
}

inline auto GoomDrawToContainer::GetLastDrawnColors(const Point2dInt point) const -> MultiplePixels
{
  return {GetLastDrawnColor(point), BLACK_PIXEL};
}

inline auto GoomDrawToContainer::GetColorsList(const Point2dInt point) const -> const ColorsList&
{
  return m_xyPixelList.at(static_cast<size_t>(point.y)).at(static_cast<size_t>(point.x));
}

inline auto GoomDrawToContainer::GetNumChangedCoords() const -> size_t
{
  return m_orderedXYPixelList.size();
}

inline auto GoomDrawToContainer::GetChangedCoordsList() const -> const std::vector<Point2dInt>&
{
  return m_orderedXYPixelList;
}

} // namespace GOOM::DRAW
