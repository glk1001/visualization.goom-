#pragma once

#include "goom_draw.h"
#include "goom_graphic.h"
#include "point2d.h"

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

  auto GetPixel(Point2dInt point) const -> Pixel override;
  void DrawPixelsUnblended(Point2dInt point, const std::vector<Pixel>& colors) override;

protected:
  void DrawPixelsToDevice(Point2dInt point,
                          const std::vector<Pixel>& colors,
                          uint32_t intBuffIntensity) override;

private:
  size_t m_numBuffers = 0;
  std::vector<PixelBuffer*> m_multipleBuffers{};
};

inline void GoomDrawToBuffer::SetBuffers(const std::vector<PixelBuffer*>& buffs)
{
  m_multipleBuffers = buffs;
  m_numBuffers = m_multipleBuffers.size();
}

} // namespace GOOM::DRAW
