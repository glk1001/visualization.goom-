module;

#include "goom/goom_graphic.h"
#include "goom/point2d.h"

#include <vector>

export module Goom.Draw.GoomDrawToMany;

import Goom.Draw.GoomDrawBase;
import Goom.Lib.GoomTypes;

export namespace GOOM::DRAW
{

class GoomDrawToMany : public IGoomDraw
{
public:
  GoomDrawToMany() noexcept = delete;
  GoomDrawToMany(const Dimensions& dimensions, const std::vector<IGoomDraw*>& manyDraws) noexcept;

  [[nodiscard]] auto GetPixel(const Point2dInt& point) const noexcept -> Pixel override;
  auto DrawPixelsUnblended(const Point2dInt& point, const MultiplePixels& colors) noexcept
      -> void override;

protected:
  auto DrawPixelsToDevice(const Point2dInt& point, const MultiplePixels& colors) noexcept
      -> void override;

private:
  std::vector<IGoomDraw*> m_manyDraws;
};

} // namespace GOOM::DRAW
