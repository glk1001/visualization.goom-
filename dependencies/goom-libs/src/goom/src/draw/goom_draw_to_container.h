#pragma once

#include "goom_draw.h"
#include "goom_graphic.h"

#include <array>
#include <cstdint>
#include <map>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace DRAW
{
#else
namespace GOOM::DRAW
{
#endif

class GoomDrawToContainer : public IGoomDraw
{
public:
  GoomDrawToContainer() noexcept = delete;
  GoomDrawToContainer(uint32_t screenWidth, uint32_t screenHeight);

  auto GetPixel(int32_t x, int32_t y) const -> Pixel override;
  void DrawPixelsUnblended(int32_t x, int32_t y, const std::vector<Pixel>& colors) const override;

  auto GetPixels(int32_t x, int32_t y) const -> std::vector<Pixel>;

  struct Coords
  {
    int32_t x;
    int32_t y;
  };
  static constexpr size_t MAX_NUM_COLORS_LIST = 3;
  using ColorsArray = std::array<Pixel, MAX_NUM_COLORS_LIST>;
  struct ColorsList
  {
    uint8_t count = 0;
    ColorsArray colorsArray{};
  };
  [[nodiscard]] auto GetNumChangedCoords() const -> size_t;
  [[nodiscard]] auto GetChangedCoordsList() const -> const std::vector<Coords>&;
  // IMPORTANT: The above is ordered from oldest to newest.
  [[nodiscard]] auto GetColorsList(int32_t x, int32_t y) const -> const ColorsList&;

  using CoordsFunc = std::function<void(int32_t x, int32_t y, const ColorsList& colorsList)>;
  // NOTE: 'func' must be thread-safe.
  void IterateChangedCoords(const CoordsFunc& func) const;

  void ResizeChangedCoordsKeepingNewest(size_t n);
  void ClearAll();

private:
  std::vector<std::vector<ColorsList>> m_xyPixelList{};
  std::vector<Coords> m_orderedXYPixelList{};
  [[nodiscard]] auto GetWriteableColorsList(int32_t x, int32_t y) -> ColorsList&;
  [[nodiscard]] auto GetLastDrawnColor(int32_t x, int32_t y) const -> Pixel;
  [[nodiscard]] auto GetLastDrawnColors(int32_t x, int32_t y) const -> std::vector<Pixel>;
  void SavePixels(int32_t x,
                  int32_t y,
                  const std::vector<Pixel>& colors,
                  uint32_t intBuffIntensity,
                  bool allowOverexposed);
};

inline auto GoomDrawToContainer::GetPixel(const int32_t x, const int32_t y) const -> Pixel
{
  return GetLastDrawnColor(x, y);
}

inline auto GoomDrawToContainer::GetPixels(const int32_t x, const int32_t y) const
    -> std::vector<Pixel>
{
  return GetLastDrawnColors(x, y);
}

inline auto GoomDrawToContainer::GetLastDrawnColor(const int32_t x, const int32_t y) const -> Pixel
{
  const ColorsList& colorsList = GetColorsList(x, y);
  if (colorsList.count == 0)
  {
    return Pixel::BLACK;
  }
  return colorsList.colorsArray[colorsList.count - 1];
}

inline auto GoomDrawToContainer::GetLastDrawnColors(const int32_t x, const int32_t y) const
    -> std::vector<Pixel>
{
  return {GetLastDrawnColor(x, y), Pixel::BLACK};
}

inline auto GoomDrawToContainer::GetColorsList(const int32_t x, const int32_t y) const
    -> const ColorsList&
{
  return m_xyPixelList.at(static_cast<size_t>(y)).at(static_cast<size_t>(x));
}

inline auto GoomDrawToContainer::GetNumChangedCoords() const -> size_t
{
  return m_orderedXYPixelList.size();
}

inline auto GoomDrawToContainer::GetChangedCoordsList() const -> const std::vector<Coords>&
{
  return m_orderedXYPixelList;
}

#if __cplusplus <= 201402L
} // namespace DRAW
} // namespace GOOM
#else
} // namespace GOOM::DRAW
#endif

