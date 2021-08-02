#include "goom_draw_to_container.h"

#include "goom_graphic.h"
#include "goomutils/colorutils.h"

#undef NDEBUG
#include <cassert>
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

using UTILS::GetBrighterColorInt;

GoomDrawToContainer::GoomDrawToContainer(const uint32_t screenWidth, const uint32_t screenHeight)
  : IGoomDraw{screenWidth, screenHeight,
              [&](int32_t x,
                  int32_t y,
                  const std::vector<Pixel>& newColors,
                  const bool allowOverexposed) {
                DrawPixels(x, y, newColors, GetIntBuffIntensity(), allowOverexposed);
              }},
    m_coordList(screenHeight)
{
  const std::vector<Pixel> blackColors{Pixel::BLACK, Pixel::BLACK};
  for (auto& xCoordInfoList : m_coordList)
  {
    xCoordInfoList.resize(screenWidth);
    std::fill(xCoordInfoList.begin(), xCoordInfoList.end(), blackColors);
  }
}

GoomDrawToContainer::~GoomDrawToContainer() noexcept = default;

auto GoomDrawToContainer::GetPixel(const int32_t x, const int32_t y) const -> Pixel
{
  return m_coordList.at(static_cast<size_t>(y)).at(static_cast<size_t>(x))[0];
}

auto GoomDrawToContainer::GetPixels(int32_t x, int32_t y) const -> const std::vector<Pixel>&
{
  return m_coordList.at(static_cast<size_t>(y)).at(static_cast<size_t>(x));
}

void GoomDrawToContainer::DrawPixelsUnblended(
    [[maybe_unused]] const int32_t x,
    [[maybe_unused]] const int32_t y,
    [[maybe_unused]] const std::vector<Pixel>& colors) const
{
  throw std::logic_error("GoomDrawToContainer::DrawPixelsUnblended not implemented.");
}

void GoomDrawToContainer::DrawPixels(const int32_t x,
                                     const int32_t y,
                                     const std::vector<Pixel>& colors,
                                     const uint32_t intBuffIntensity,
                                     const bool allowOverexposed)
{
  Colors& coordColors = m_coordList.at(static_cast<size_t>(y)).at(static_cast<size_t>(x));
  assert(colors.size() <= coordColors.size());
  for (size_t i = 0; i < colors.size(); i++)
  {
    coordColors.at(i) = GetBrighterColorInt(intBuffIntensity, colors[i], allowOverexposed);
  }
  m_changedCoordsList.emplace_front(Coords{x, y}); // TODO could be duplicates
}

#if __cplusplus <= 201402L
} // namespace DRAW
} // namespace GOOM
#else
} // namespace GOOM::DRAW
#endif
