#include "goom_image_buffers.h"

#include "goom_graphic.h"

#include <cstdint>
#include <memory>

namespace GOOM::CONTROL
{

auto GoomImageBuffers::GetBuffs(const uint32_t width, const uint32_t height)
    -> std::array<std::unique_ptr<PixelBuffer>, MAX_NUM_BUFFS>
{
  std::array<std::unique_ptr<PixelBuffer>, MAX_NUM_BUFFS> buffs{};
  for (auto& buff : buffs)
  {
    buff = std::make_unique<PixelBuffer>(width, height);
  }
  return buffs;
}

GoomImageBuffers::GoomImageBuffers(const uint32_t width, const uint32_t height) noexcept
  : m_buffs{GetBuffs(width, height)}, m_p1{*m_buffs[0]}, m_p2{*m_buffs[1]}
{
}

void GoomImageBuffers::RotateBuffers()
{
  std::swap(m_p1, m_p2);
}

} // namespace GOOM::CONTROL
