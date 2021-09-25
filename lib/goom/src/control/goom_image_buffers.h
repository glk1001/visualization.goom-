#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>

#if __cplusplus <= 201402L
namespace GOOM
{

class PixelBuffer;

namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

class GoomImageBuffers
{
public:
  GoomImageBuffers() noexcept = delete;
  GoomImageBuffers(uint32_t width, uint32_t height) noexcept;

  [[nodiscard]] auto GetP1() const -> PixelBuffer& { return m_p1; }
  [[nodiscard]] auto GetP2() const -> PixelBuffer& { return m_p2; }

  [[nodiscard]] auto GetOutputBuff() const -> PixelBuffer& { return *m_outputBuff; }
  void SetOutputBuff(const std::shared_ptr<PixelBuffer>& buffer) { m_outputBuff = buffer; }

  void RotateBuffers();

private:
  static constexpr size_t MAX_NUM_BUFFS = 2;
  std::array<std::unique_ptr<PixelBuffer>, MAX_NUM_BUFFS> m_buffs{};
  std::reference_wrapper<PixelBuffer> m_p1;
  std::reference_wrapper<PixelBuffer> m_p2;
  std::shared_ptr<PixelBuffer> m_outputBuff{};
  [[nodiscard]] static auto GetBuffs(uint32_t width, uint32_t height)
      -> std::array<std::unique_ptr<PixelBuffer>, MAX_NUM_BUFFS>;
};

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif

