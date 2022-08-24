#pragma once

#include "goom_draw.h"
#include "goom_graphic.h"
#include "goom_types.h"
#include "point2d.h"

#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

class GoomDrawToBuffer : public IGoomDraw
{
public:
  GoomDrawToBuffer() noexcept = delete;
  explicit GoomDrawToBuffer(const Dimensions& dimensions);
  GoomDrawToBuffer(const GoomDrawToBuffer&) noexcept = delete;
  GoomDrawToBuffer(GoomDrawToBuffer&&) noexcept      = delete;
  ~GoomDrawToBuffer() noexcept override;
  auto operator=(const GoomDrawToBuffer&) noexcept -> GoomDrawToBuffer& = delete;
  auto operator=(GoomDrawToBuffer&&) noexcept -> GoomDrawToBuffer&      = delete;

  void SetBuffers(const std::vector<PixelBuffer*>& buffs);

  auto GetPixel(Point2dInt point) const -> Pixel override;
  void DrawPixelsUnblended(Point2dInt point, const MultiplePixels& colors) override;

protected:
  void DrawPixelsToDevice(Point2dInt point,
                          const MultiplePixels& colors,
                          uint32_t intBuffIntensity) override;

private:
  size_t m_numBuffers = 0;
  std::vector<PixelBuffer*> m_multipleBuffers{};
  PixelBuffer* m_multipleBuffer0{};
};

} // namespace GOOM::DRAW
