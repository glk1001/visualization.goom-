#pragma once

#include "goom_draw.h"
#include "goom_graphic.h"

#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

class GoomDrawToBuffer : public IGoomDraw
{
public:
  GoomDrawToBuffer() noexcept = delete;
  GoomDrawToBuffer(uint32_t screenWidth, uint32_t screenHeight);

  void SetBuffers(const std::vector<PixelBuffer*>& buffs);

  auto GetPixel(int32_t x, int32_t y) const -> Pixel override;
  void DrawPixelsUnblended(int32_t x, int32_t y, const std::vector<Pixel>& colors) override;

protected:
  void DrawPixelsToDevice(int32_t x,
                          int32_t y,
                          const std::vector<Pixel>& colors,
                          uint32_t intBuffIntensity) override;

private:
  std::vector<PixelBuffer*> m_multipleBuffers{};
};

inline void GoomDrawToBuffer::SetBuffers(const std::vector<PixelBuffer*>& buffs)
{
  m_multipleBuffers = buffs;
}

} // namespace GOOM::DRAW
