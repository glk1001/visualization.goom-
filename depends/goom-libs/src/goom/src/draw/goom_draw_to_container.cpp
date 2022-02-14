#include "goom_draw_to_container.h"

//#undef NO_LOGGING

#include "color/colorutils.h"
#include "goom_graphic.h"
#include "goom/logging.h"

#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

using COLOR::GetBrighterColorInt;
using UTILS::Logging;

GoomDrawToContainer::GoomDrawToContainer(const uint32_t screenWidth, const uint32_t screenHeight)
  : IGoomDraw{screenWidth, screenHeight}, m_xyPixelList(screenHeight)
{
  for (auto& xPixelList : m_xyPixelList)
  {
    xPixelList.resize(screenWidth);
  }
}

void GoomDrawToContainer::ClearAll()
{
  m_orderedXYPixelList.clear();

  for (auto& xPixelList : m_xyPixelList)
  {
    for (auto& colorsList : xPixelList)
    {
      colorsList.count = 0;
    }
  }
}

inline auto GoomDrawToContainer::GetWriteableColorsList(const int32_t x, const int32_t y)
    -> ColorsList&
{
  return m_xyPixelList.at(static_cast<size_t>(y)).at(static_cast<size_t>(x));
}

void GoomDrawToContainer::DrawPixelsUnblended([[maybe_unused]] const int32_t x,
                                              [[maybe_unused]] const int32_t y,
                                              [[maybe_unused]] const std::vector<Pixel>& colors)
{
  throw std::logic_error("GoomDrawToContainer::DrawPixelsUnblended not implemented.");
}

void GoomDrawToContainer::DrawPixelsToDevice(const int32_t x,
                                             const int32_t y,
                                             const std::vector<Pixel>& colors,
                                             const uint32_t intBuffIntensity)
{
  ColorsList& colorsList = GetWriteableColorsList(x, y);

  if (colorsList.count == colorsList.colorsArray.size())
  {
    return;
  }

  // NOTE: Just save the first pixel in 'colors'. May need to improve this.
  const Pixel newColor = GetBrighterColorInt(intBuffIntensity, colors[0]);

  colorsList.colorsArray[colorsList.count] = newColor;
  ++colorsList.count;
  if (1 == colorsList.count)
  {
    m_orderedXYPixelList.emplace_back(Coords{x, y});
  }
}

void GoomDrawToContainer::ResizeChangedCoordsKeepingNewest(const size_t numToKeep)
{
  assert(numToKeep <= m_orderedXYPixelList.size());

  const auto eraseFrom = m_orderedXYPixelList.begin();
  const auto eraseTo = m_orderedXYPixelList.begin() +
                       static_cast<std::ptrdiff_t>(m_orderedXYPixelList.size() - numToKeep);

  for (auto coords = eraseFrom; coords != eraseTo; ++coords)
  {
    GetWriteableColorsList(coords->x, coords->y).count = 0;
  }

  m_orderedXYPixelList.erase(eraseFrom, eraseTo);
  m_orderedXYPixelList.resize(numToKeep);
}

void GoomDrawToContainer::IterateChangedCoordsNewToOld(const CoordsFunc& func) const
{
  const auto runFunc = [&](const size_t i) {
    const auto& coords = m_orderedXYPixelList[i];
    const int32_t x = coords.x;
    const int32_t y = coords.y;
    const ColorsList& colorsList = GetColorsList(x, y);
    func(x, y, colorsList);
  };

  // Start with the newest coords added.
  const auto maxIndex = static_cast<int32_t>(m_orderedXYPixelList.size() - 1);
  for (int32_t i = maxIndex; i >= 0; --i)
  {
    runFunc(static_cast<size_t>(i));
  }
}

} // namespace GOOM::DRAW
