#include "fractal_hits.h"

#include "color/colorutils.h"

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

using COLOR::GetColorAverage;

FractalHits::FractalHits(const uint32_t width, const uint32_t height) noexcept
  : m_width{width}, m_height{height}, m_hitInfo(m_height)
{
  for (auto& xHit : m_hitInfo)
  {
    xHit.resize(m_width);
  }
  m_hits.reserve(HITS_ESTIMATE);
}

void FractalHits::Reset()
{
  m_maxHitCount = 0;
  m_hits.clear();
  for (auto& xHit : m_hitInfo)
  {
    std::fill(xHit.begin(), xHit.end(), HitInfo{0, Pixel::BLACK});
  }
}

void FractalHits::AddHit(const int32_t x, const int32_t y, const Similitude& simi)
{
  if ((x < 0) || (y < 0))
  {
    return;
  }
  if ((static_cast<uint32_t>(x) >= m_width) || (static_cast<uint32_t>(y) >= m_height))
  {
    return;
  }

  const auto ux = static_cast<uint32_t>(x);
  const auto uy = static_cast<uint32_t>(y);

  HitInfo& hitInfo = m_hitInfo[uy][ux];

  hitInfo.simi = &simi;
  hitInfo.color = GetColorAverage(hitInfo.color, simi.GetColor());
  ++hitInfo.count;
  if (hitInfo.count > m_maxHitCount)
  {
    m_maxHitCount = hitInfo.count;
  }

  if (1 == hitInfo.count)
  {
    m_hits.emplace_back(ux, uy, 1);
  }
}

auto FractalHits::GetBuffer() -> const std::vector<IfsPoint>&
{
  m_buffer.clear();
  m_buffer.reserve(m_hits.size());

  for (const auto& hit : m_hits)
  {
    IfsPoint updatedHit = hit;
    const HitInfo& hitInfo = m_hitInfo[hit.GetY()][hit.GetX()];

    updatedHit.SetCount(hitInfo.count);
    updatedHit.SetColor(hitInfo.color);
    updatedHit.SetSimi(hitInfo.simi);

    m_buffer.emplace_back(updatedHit);
  }

  return m_buffer;
}

#if __cplusplus <= 201402L
} // namespace IFS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::IFS
#endif
