#pragma once

#include "goom_draw.h"
#include "goom_graphic.h"
#include "goom_types.h"
#include "point2d.h"

#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

class GoomDrawToMany : public IGoomDraw
{
public:
  GoomDrawToMany() noexcept = delete;
  GoomDrawToMany(const Dimensions& dimensions,
                 const std::vector<IGoomDraw*>& manyDraws);

  auto GetPixel(Point2dInt point) const -> Pixel override;
  void DrawPixelsUnblended(Point2dInt point, const MultiplePixels& colors) override;

protected:
  void DrawPixelsToDevice(Point2dInt point,
                          const MultiplePixels& colors,
                          uint32_t intBuffIntensity) override;

private:
  const std::vector<IGoomDraw*> m_manyDraws;
};

} // namespace GOOM::DRAW
