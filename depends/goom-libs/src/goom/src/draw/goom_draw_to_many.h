#pragma once

#include "goom_draw.h"
#include "goom_graphic.h"
#include "point2d.h"

#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

class GoomDrawToMany : public IGoomDraw
{
public:
  GoomDrawToMany() noexcept = delete;
  GoomDrawToMany(uint32_t screenWidth,
                 uint32_t screenHeight,
                 const std::vector<IGoomDraw*>& manyDraws);

  auto GetPixel(Point2dInt point) const -> Pixel override;
  void DrawPixelsUnblended(Point2dInt point, const std::vector<Pixel>& colors) override;

protected:
  void DrawPixelsToDevice(Point2dInt point,
                          const std::vector<Pixel>& colors,
                          uint32_t intBuffIntensity) override;

private:
  const std::vector<IGoomDraw*> m_manyDraws;
};

} // namespace GOOM::DRAW
