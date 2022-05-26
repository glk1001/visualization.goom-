#include "goom_draw_to_many.h"

#include "goom_config.h"
#include "goom_graphic.h"

#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

GoomDrawToMany::GoomDrawToMany(const uint32_t screenWidth,
                               const uint32_t screenHeight,
                               const std::vector<IGoomDraw*>& manyDraws)
  : IGoomDraw{screenWidth, screenHeight}, m_manyDraws{manyDraws}
{
  Expects(not manyDraws.empty());
}

auto GoomDrawToMany::GetPixel(const Point2dInt point) const -> Pixel
{
  const IGoomDraw* const draw = m_manyDraws[0];
  return draw->GetPixel(point);
}

void GoomDrawToMany::DrawPixelsUnblended(const Point2dInt point, const MultiplePixels& colors)
{
  for (auto* const draw : m_manyDraws)
  {
    draw->DrawPixelsUnblended(point, colors);
  }
}

void GoomDrawToMany::DrawPixelsToDevice(const Point2dInt point,
                                        const MultiplePixels& colors,
                                        [[maybe_unused]] const uint32_t intBuffIntensity)
{
  for (auto* const draw : m_manyDraws)
  {
    draw->DrawPixels(point, colors);
  }
}

} // namespace GOOM::DRAW
