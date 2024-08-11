module;

#include <cmath>
#include <cstdint>
#include <utility>

module Goom.Draw.ShaperDrawers.LineDrawWu;

namespace GOOM::DRAW::SHAPE_DRAWERS::WU_LINES
{

auto WuLine(float x0, float y0, float x1, float y1, const PlotPointFunc& plot) noexcept -> void
{
  const auto iPart  = [](const float x) { return static_cast<int>(std::floor(x)); };
  const auto fRound = [](const float x) { return std::round(x); };
  const auto fPart  = [](const float x) { return x - std::floor(x); };
  const auto rFPart = [=](const float x) { return 1 - fPart(x); };

  const auto steep = std::abs(y1 - y0) > std::abs(x1 - x0);
  if (steep)
  {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  if (x0 > x1)
  {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  const auto dx       = x1 - x0; // because of above swap, must be >= 0
  const auto dy       = y1 - y0;
  const auto gradient = (dx < 0.001F) ? 1 : (dy / dx);

  int32_t xpx11; // NOLINT(cppcoreguidelines-init-variables)
  float interY; // NOLINT(cppcoreguidelines-init-variables)
  {
    const auto xEnd  = fRound(x0);
    const auto yEnd  = y0 + (gradient * (xEnd - x0));
    const auto xGap  = rFPart(x0 + 0.5F);
    xpx11            = static_cast<int>(xEnd);
    const auto ypx11 = iPart(yEnd);
    if (steep)
    {
      plot({.x = ypx11, .y = xpx11}, rFPart(yEnd) * xGap);
      plot({.x = ypx11 + 1, .y = xpx11}, fPart(yEnd) * xGap);
    }
    else
    {
      plot({.x = xpx11, .y = ypx11}, rFPart(yEnd) * xGap);
      plot({.x = xpx11, .y = ypx11 + 1}, fPart(yEnd) * xGap);
    }
    interY = yEnd + gradient;
  }

  int32_t xpx12; // NOLINT(cppcoreguidelines-init-variables)
  {
    const auto xEnd  = fRound(x1);
    const auto yEnd  = y1 + (gradient * (xEnd - x1));
    const auto xGap  = rFPart(x1 + 0.5F);
    xpx12            = static_cast<int>(xEnd);
    const auto ypx12 = iPart(yEnd);
    if (steep)
    {
      plot({.x = ypx12, .y = xpx12}, rFPart(yEnd) * xGap);
      plot({.x = ypx12 + 1, .y = xpx12}, fPart(yEnd) * xGap);
    }
    else
    {
      plot({.x = xpx12, .y = ypx12}, rFPart(yEnd) * xGap);
      plot({.x = xpx12, .y = ypx12 + 1}, fPart(yEnd) * xGap);
    }
  }

  if (steep)
  {
    for (auto x = xpx11 + 1; x < xpx12; ++x)
    {
      plot({.x = iPart(interY), .y = x}, rFPart(interY));
      plot({.x = iPart(interY) + 1, .y = x}, fPart(interY));
      interY += gradient;
    }
  }
  else
  {
    for (auto x = xpx11 + 1; x < xpx12; ++x)
    {
      plot({.x = x, .y = iPart(interY)}, rFPart(interY));
      plot({.x = x, .y = iPart(interY) + 1}, fPart(interY));
      interY += gradient;
    }
  }
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS::WU_LINES
