#include "goom_draw_to_many.h"

#include "goom_graphic.h"

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

GoomDrawToMany::GoomDrawToMany(const uint32_t screenWidth,
                               const uint32_t screenHeight,
                               const std::vector<const IGoomDraw*>& manyDraws)
  : IGoomDraw{screenWidth, screenHeight,
              [&](int32_t x,
                  int32_t y,
                  const std::vector<Pixel>& newColors,
                  const bool allowOverexposed) {
                DrawPixels(m_manyDraws, x, y, newColors, allowOverexposed);
              }},
    m_manyDraws{manyDraws}
{
  assert(!m_manyDraws.empty());
}

GoomDrawToMany::~GoomDrawToMany() noexcept = default;

auto GoomDrawToMany::GetPixel(const int32_t x, const int32_t y) const -> Pixel
{
  const IGoomDraw* draw = m_manyDraws[0];
  return draw->GetPixel(x, y);
}

void GoomDrawToMany::DrawPixelsUnblended(const int32_t x,
                                         const int32_t y,
                                         const std::vector<Pixel>& colors) const
{
  for (const auto& draw : m_manyDraws)
  {
    draw->DrawPixelsUnblended(x, y, colors);
  }
}

void GoomDrawToMany::DrawPixels(const std::vector<const IGoomDraw*>& manyDraws,
                                const int32_t x,
                                const int32_t y,
                                const std::vector<Pixel>& colors,
                                const bool allowOverexposed)
{
  for (const auto& draw : manyDraws)
  {
    draw->DrawPixels(x, y, colors, allowOverexposed);
  }
}

#if __cplusplus <= 201402L
} // namespace DRAW
} // namespace GOOM
#else
} // namespace GOOM::DRAW
#endif
