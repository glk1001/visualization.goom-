#pragma once

#include <array>
#include <cstdint>
#include <memory>

namespace GOOM
{

class PixelBuffer;

namespace CONTROL
{

class GoomImageBuffers
{
public:
  GoomImageBuffers() noexcept = delete;
  GoomImageBuffers(uint32_t width, uint32_t height) noexcept;
  GoomImageBuffers(const GoomImageBuffers&) noexcept = delete;
  GoomImageBuffers(GoomImageBuffers&&) noexcept = delete;
  ~GoomImageBuffers() noexcept;
  auto operator=(const GoomImageBuffers&) noexcept -> GoomImageBuffers& = delete;
  auto operator=(GoomImageBuffers&&) noexcept -> GoomImageBuffers& = delete;

  [[nodiscard]] auto GetP1() const -> PixelBuffer& { return *m_p1; }
  [[nodiscard]] auto GetP2() const -> PixelBuffer& { return *m_p2; }

  [[nodiscard]] auto GetOutputBuff() const -> PixelBuffer& { return *m_outputBuff; }
  void SetOutputBuff(const std::shared_ptr<PixelBuffer>& buffer) { m_outputBuff = buffer; }

  void RotateBuffers();

private:
  static constexpr size_t MAX_NUM_BUFFS = 2;
  std::array<std::unique_ptr<PixelBuffer>, MAX_NUM_BUFFS> m_buffs;
  PixelBuffer* m_p1{m_buffs[0].get()};
  PixelBuffer* m_p2{m_buffs[1].get()};
  std::shared_ptr<PixelBuffer> m_outputBuff{};
  [[nodiscard]] static auto GetBuffs(uint32_t width, uint32_t height)
      -> std::array<std::unique_ptr<PixelBuffer>, MAX_NUM_BUFFS>;
};

} // namespace CONTROL
} // namespace GOOM
