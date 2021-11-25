#pragma once

#include "goom_graphic.h"
#include "ifs_types.h"
#include "similitudes.h"

#include <cstdint>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace IFS
{
#else
namespace GOOM::VISUAL_FX::IFS
{
#endif

class FractalHits
{
public:
  FractalHits() noexcept = delete;
  FractalHits(uint32_t width, uint32_t height) noexcept;
  void Reset();
  void AddHit(int32_t x, int32_t y, const Similitude& simi);
  [[nodiscard]] auto GetBuffer() -> const std::vector<IfsPoint>&;
  [[nodiscard]] auto GetMaxHitCount() const -> uint32_t;

private:
  const uint32_t m_width;
  const uint32_t m_height;
  struct HitInfo
  {
    uint32_t count = 0;
    Pixel color = Pixel::BLACK;
    const Similitude* simi{};
  };
  std::vector<std::vector<HitInfo>> m_hitInfo{};
  uint32_t m_maxHitCount = 0;
  static constexpr size_t HITS_ESTIMATE = 1000;
  std::vector<IfsPoint> m_hits{};
  std::vector<IfsPoint> m_buffer{};
};

inline auto FractalHits::GetMaxHitCount() const -> uint32_t
{
  return m_maxHitCount;
}

#if __cplusplus <= 201402L
} // namespace IFS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::IFS
#endif
