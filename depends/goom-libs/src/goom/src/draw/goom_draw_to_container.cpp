#include "goom_draw_to_container.h"

//#undef NO_LOGGING

#include "color/color_utils.h"
#include "goom/logging.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_types.h"

#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

using COLOR::GetBrighterColorInt;
using UTILS::Logging; // NOLINT(misc-unused-using-decls)

GoomDrawToContainer::GoomDrawToContainer(const Dimensions& dimensions)
  : IGoomDraw{dimensions}, m_xyPixelList(dimensions.GetHeight())
{
  for (auto& xPixelList : m_xyPixelList)
  {
    xPixelList.resize(dimensions.GetWidth());
  }
}

GoomDrawToContainer::~GoomDrawToContainer() noexcept = default;

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

inline auto GoomDrawToContainer::GetWriteableColorsList(const Point2dInt point) -> ColorsList&
{
  return m_xyPixelList.at(static_cast<size_t>(point.y)).at(static_cast<size_t>(point.x));
}

void GoomDrawToContainer::DrawPixelsUnblended([[maybe_unused]] const Point2dInt point,
                                              [[maybe_unused]] const MultiplePixels& colors)
{
  throw std::logic_error("GoomDrawToContainer::DrawPixelsUnblended not implemented.");
}

void GoomDrawToContainer::DrawPixelsToDevice(const Point2dInt point,
                                             const MultiplePixels& colors,
                                             const uint32_t intBuffIntensity)
{
  auto& colorsList = GetWriteableColorsList(point);

  if (colorsList.count == colorsList.colorsArray.size())
  {
    return;
  }

  // NOTE: Just save the first pixel in 'colors'. May need to improve this.
  const auto newColor = GetBrighterColorInt(intBuffIntensity, colors[0]);

  colorsList.colorsArray[colorsList.count] = newColor;
  ++colorsList.count;
  if (1 == colorsList.count)
  {
    m_orderedXYPixelList.emplace_back(point);
  }
}

void GoomDrawToContainer::ResizeChangedCoordsKeepingNewest(const size_t numToKeep)
{
  Expects(numToKeep <= m_orderedXYPixelList.size());

  const auto eraseFrom = m_orderedXYPixelList.begin();
  const auto eraseTo   = m_orderedXYPixelList.begin() +
                       static_cast<std::ptrdiff_t>(m_orderedXYPixelList.size() - numToKeep);

  for (auto coords = eraseFrom; coords != eraseTo; ++coords)
  {
    GetWriteableColorsList(*coords).count = 0;
  }

  m_orderedXYPixelList.erase(eraseFrom, eraseTo);
  m_orderedXYPixelList.resize(numToKeep);
}

void GoomDrawToContainer::IterateChangedCoordsNewToOld(const CoordsFunc& func) const
{
  const auto runFunc = [&](const size_t i)
  {
    const auto& coords     = m_orderedXYPixelList[i];
    const auto& colorsList = GetColorsList(coords);
    func(coords, colorsList);
  };

  // Start with the newest coords added.
  const auto maxIndex = static_cast<int32_t>(m_orderedXYPixelList.size() - 1);
  for (auto i = maxIndex; i >= 0; --i)
  {
    runFunc(static_cast<size_t>(i));
  }
}

} // namespace GOOM::DRAW
