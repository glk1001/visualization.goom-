#include "goom_draw_to_container.h"

#include "goom_graphic.h"
#include "goomutils/colorutils.h"
#include "goomutils/logging_control.h"
//#undef NO_LOGGING
#include "goomutils/logging.h"

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
using UTILS::Logging;

GoomDrawToContainer::GoomDrawToContainer(const uint32_t screenWidth, const uint32_t screenHeight)
  : IGoomDraw{screenWidth, screenHeight,
              [&](const int32_t x,
                  const int32_t y,
                  const std::vector<Pixel>& newColors,
                  const bool allowOverexposed) {
                SavePixels(x, y, newColors, GetIntBuffIntensity(), allowOverexposed);
              }},
    m_xyPixelList(screenHeight)
{
  for (auto& xPixelList : m_xyPixelList)
  {
    xPixelList.resize(screenWidth);
  }
}

GoomDrawToContainer::~GoomDrawToContainer() noexcept = default;

void GoomDrawToContainer::ClearAll()
{
  m_orderedXYPixelList.clear();

  const ColorsList emptyColorsList{};
  for (auto& xPixelList : m_xyPixelList)
  {
    std::fill(xPixelList.begin(), xPixelList.end(), emptyColorsList);
  }
}

inline auto GoomDrawToContainer::GetWriteableColorsList(const int32_t x, const int32_t y)
    -> ColorsList&
{
  return m_xyPixelList.at(static_cast<size_t>(y)).at(static_cast<size_t>(x));
}

void GoomDrawToContainer::DrawPixelsUnblended(
    [[maybe_unused]] const int32_t x,
    [[maybe_unused]] const int32_t y,
    [[maybe_unused]] const std::vector<Pixel>& colors) const
{
  throw std::logic_error("GoomDrawToContainer::DrawPixelsUnblended not implemented.");
}

void GoomDrawToContainer::SavePixels(const int32_t x,
                                     const int32_t y,
                                     const std::vector<Pixel>& colors,
                                     const uint32_t intBuffIntensity,
                                     const bool allowOverexposed)
{
  Colors newColors(colors.size());
  for (size_t i = 0; i < newColors.size(); i++)
  {
    newColors[i] = GetBrighterColorInt(intBuffIntensity, colors[i], allowOverexposed);
  }

  ColorsList& colorsList = GetWriteableColorsList(x, y);
  colorsList.emplace_back(newColors);
  if (colorsList.size() == 1)
  {
    m_orderedXYPixelList.emplace_back(Coords{x, y});
  }
}

void GoomDrawToContainer::ResizeChangedCoordsKeepingNewest(const size_t n)
{
  assert(n <= m_orderedXYPixelList.size());

  const auto eraseFrom = m_orderedXYPixelList.begin();
  const auto eraseTo =
      m_orderedXYPixelList.begin() + static_cast<std::ptrdiff_t>(m_orderedXYPixelList.size() - n);

  for (auto coords = eraseFrom; coords != eraseTo; ++coords)
  {
    GetWriteableColorsList(coords->x, coords->y).clear();
  }

  m_orderedXYPixelList.erase(eraseFrom, eraseTo);
  m_orderedXYPixelList.resize(n);
}

void GoomDrawToContainer::IterateChangedCoordsNewToOld(const CoordsFunc& f) const
{
  // Start with the newest coords added.
  std::for_each(m_orderedXYPixelList.rbegin(), m_orderedXYPixelList.rend(),
                [&](const auto& coords) {
                  const int32_t x = coords.x;
                  const int32_t y = coords.y;
                  const ColorsList& colorsList = GetColorsList(x, y);
                  f(x, y, colorsList);
                });
}

#if __cplusplus <= 201402L
} // namespace DRAW
} // namespace GOOM
#else
} // namespace GOOM::DRAW
#endif
