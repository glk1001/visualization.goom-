#include "goom_image_buffers.h"

#include "goom_graphic.h"
#include "goom_types.h"

#include <memory>

namespace GOOM::CONTROL
{

GoomImageBuffers::GoomImageBuffers(const Dimensions& dimensions) noexcept
  : m_p1{std::make_unique<PixelBufferVector>(dimensions)},
    m_p2{std::make_unique<PixelBufferVector>(dimensions)}
{
}

void GoomImageBuffers::RotateBuffers() noexcept
{
  std::swap(m_p1, m_p2);
}

} // namespace GOOM::CONTROL
