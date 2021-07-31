#ifndef VISUALIZATION_GOOM_GOOMDRAWBUFFER_H
#define VISUALIZATION_GOOM_GOOMDRAWBUFFER_H

#include "goom_draw.h"
#include "goom_graphic.h"

#include <cstdint>
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
  GoomDrawToContainer(const GoomDrawToContainer&) noexcept = delete;
  GoomDrawToContainer(GoomDrawToContainer&&) noexcept = delete;
  ~GoomDrawToContainer() noexcept override;
  auto operator=(const GoomDrawToContainer&) -> GoomDrawToContainer& = delete;
  auto operator=(GoomDrawToContainer&&) -> GoomDrawToContainer& = delete;

  auto GetPixel(int32_t x, int32_t y) const -> Pixel override;
  void DrawPixelsUnblended(int32_t x, int32_t y, const std::vector<Pixel>& colors) const override;

  auto GetPixels(int32_t x, int32_t y) const -> const std::vector<Pixel>&;

  struct Coords
  {
    int32_t x;
    int32_t y;
  };
  [[nodiscard]] auto GetChangedCoords() const -> const std::vector<Coords>&;
  void ClearChangedCoords();

private:
  std::vector<Coords> m_changedCoordsList{};
  using Colors = std::vector<Pixel>;
  std::vector<std::vector<Colors>> m_coordList{};
  void DrawPixels(int32_t x,
                  int32_t y,
                  const std::vector<Pixel>& colors,
                  uint32_t intBuffIntensity,
                  bool allowOverexposed);
};

inline auto GoomDrawToContainer::GetChangedCoords() const -> const std::vector<Coords>&
{
  return m_changedCoordsList;
}

inline void GoomDrawToContainer::ClearChangedCoords()
{
  m_changedCoordsList.clear();
}

#if __cplusplus <= 201402L
} // namespace DRAW
} // namespace GOOM
#else
} // namespace GOOM::DRAW
#endif

#endif //VISUALIZATION_GOOM_GOOMDRAWBUFFER_H
