#ifndef VISUALIZATION_GOOM_GOOMDRAWBUFFER_H
#define VISUALIZATION_GOOM_GOOMDRAWBUFFER_H

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

class GoomDrawToBuffer : public IGoomDraw
{
public:
  GoomDrawToBuffer() noexcept = delete;
  GoomDrawToBuffer(uint32_t screenWidth, uint32_t screenHeight);
  GoomDrawToBuffer(const GoomDrawToBuffer&) noexcept = delete;
  GoomDrawToBuffer(GoomDrawToBuffer&&) noexcept = delete;
  ~GoomDrawToBuffer() noexcept override;
  auto operator=(const GoomDrawToBuffer&) -> GoomDrawToBuffer& = delete;
  auto operator=(GoomDrawToBuffer&&) -> GoomDrawToBuffer& = delete;

  void SetBuffers(const std::vector<PixelBuffer*>& buffs);

  auto GetPixel(int32_t x, int32_t y) const -> Pixel override;
  void DrawPixelsUnblended(int32_t x, int32_t y, const std::vector<Pixel>& colors) const override;

private:
  std::vector<PixelBuffer*> m_multipleBuffers{};
  static void DrawPixels(const std::vector<PixelBuffer*>& buffs,
                         int32_t x,
                         int32_t y,
                         const std::vector<Pixel>& colors,
                         uint32_t intBuffIntensity,
                         bool allowOverexposed);
};

inline void GoomDrawToBuffer::SetBuffers(const std::vector<PixelBuffer*>& buffs)
{
  m_multipleBuffers = buffs;
}

#if __cplusplus <= 201402L
} // namespace DRAW
} // namespace GOOM
#else
} // namespace GOOM::DRAW
#endif

#endif //VISUALIZATION_GOOM_GOOMDRAWBUFFER_H