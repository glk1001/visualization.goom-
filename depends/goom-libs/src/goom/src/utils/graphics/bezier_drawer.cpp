#include "bezier_drawer.h"

#include "color/colorutils.h"
#include "image_bitmaps.h"

namespace GOOM::UTILS::GRAPHICS
{

using COLOR::GetBrighterColor;

inline auto BezierDrawer::GetImageBitmap(const size_t size) const -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(m_currentBitmapName,
                                       std::clamp(size, MIN_DOT_DIAMETER, MAX_DOT_DIAMETER));
}

void BezierDrawer::Draw(const Bezier::Bezier<3>& bezier, const float colorT0, const float colorT1)
{
  const float colorTStep = (colorT1 - colorT0) / static_cast<float>(m_numBezierSteps - 1);

  const float tStep = 1.0F / static_cast<float>(m_numBezierSteps - 1);
  float colorT = colorT0 + colorTStep;
  float t = tStep;
  auto x0 = static_cast<int32_t>(bezier.valueAt(0.0F, 0));
  auto y0 = static_cast<int32_t>(bezier.valueAt(0.0F, 1));

  for (size_t i = 1; i < m_numBezierSteps; ++i)
  {
    const auto x1 = static_cast<int32_t>(bezier.valueAt(t, 0));
    const auto y1 = static_cast<int32_t>(bezier.valueAt(t, 1));

    const Pixel lineColor = GetBrighterColor(10.F, m_lineColorFunc(colorT));
    m_goomDraw.Line(x0, y0, x1, y1, lineColor, m_lineThickness);

    if (0 == (i % m_dotEveryNumBezierSteps))
    {
      const Pixel dotColor = GetBrighterColor(10.F, m_dotColorFunc(colorT));
      DrawDot({x1, y1}, m_dotDiameter, dotColor);
    }

    x0 = x1;
    y0 = y1;
    t += tStep;
    colorT += colorTStep;
  }
}

void BezierDrawer::DrawDot(const Point2dInt& centre, const uint32_t diameter, const Pixel& color)
{
  const auto getColor =
      [&color]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y, const Pixel& bgnd)
  {
    if (0 == bgnd.A())
    {
      return Pixel::BLACK;
    }
    return color;
  };

  m_goomDraw.Bitmap(centre.x, centre.y, GetImageBitmap(diameter), getColor);
}

} // namespace GOOM::UTILS::GRAPHICS
