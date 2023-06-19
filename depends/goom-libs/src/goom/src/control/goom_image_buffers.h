#pragma once

#include "goom_types.h"

#include <memory>

namespace GOOM
{

class PixelBuffer;

namespace CONTROL
{

class GoomImageBuffers
{
public:
  explicit GoomImageBuffers(const Dimensions& dimensions) noexcept;

  [[nodiscard]] auto GetP1() const noexcept -> PixelBuffer&;
  [[nodiscard]] auto GetP2() const noexcept -> PixelBuffer&;

  [[nodiscard]] auto GetOutputBuff() const noexcept -> PixelBuffer&;
  auto SetOutputBuff(const std::shared_ptr<PixelBuffer>& buffer) noexcept -> void;

  void RotateBuffers() noexcept;

private:
  std::unique_ptr<PixelBuffer> m_p1;
  std::unique_ptr<PixelBuffer> m_p2;
  std::shared_ptr<PixelBuffer> m_outputBuff{};
};

inline auto GoomImageBuffers::GetP1() const noexcept -> PixelBuffer&
{
  return *m_p1;
}

inline auto GoomImageBuffers::GetP2() const noexcept -> PixelBuffer&
{
  return *m_p2;
}

inline auto GoomImageBuffers::GetOutputBuff() const noexcept -> PixelBuffer&
{
  return *m_outputBuff;
}

inline auto GoomImageBuffers::SetOutputBuff(const std::shared_ptr<PixelBuffer>& buffer) noexcept
    -> void
{
  m_outputBuff = buffer;
}

} // namespace CONTROL
} // namespace GOOM
