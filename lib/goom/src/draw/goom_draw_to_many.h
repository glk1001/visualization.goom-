#ifndef VISUALIZATION_GOOM_GOOMDRAWTOMANY_H
#define VISUALIZATION_GOOM_GOOMDRAWTOMANY_H

#include "goom_draw.h"
#include "goom_graphic.h"

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

class GoomDrawToMany : public IGoomDraw
{
public:
  GoomDrawToMany() noexcept = delete;
  GoomDrawToMany(uint32_t screenWidth,
                 uint32_t screenHeight,
                 const std::vector<const IGoomDraw*>& manyDraws);
  GoomDrawToMany(const GoomDrawToMany&) noexcept = delete;
  GoomDrawToMany(GoomDrawToMany&&) noexcept = delete;
  ~GoomDrawToMany() noexcept override;
  auto operator=(const GoomDrawToMany&) -> GoomDrawToMany& = delete;
  auto operator=(GoomDrawToMany&&) -> GoomDrawToMany& = delete;

  auto GetPixel(int32_t x, int32_t y) const -> Pixel override;
  void DrawPixelsUnblended(int32_t x, int32_t y, const std::vector<Pixel>& colors) const override;

private:
  const std::vector<const IGoomDraw*> m_manyDraws;
  static void DrawPixels(const std::vector<const IGoomDraw*>& manyDraws,
                         int32_t x,
                         int32_t y,
                         const std::vector<Pixel>& colors,
                         bool allowOverexposed);
};

#if __cplusplus <= 201402L
} // namespace DRAW
} // namespace GOOM
#else
} // namespace GOOM::DRAW
#endif

#endif //VISUALIZATION_GOOM_GOOMDRAWBUFFER_H
