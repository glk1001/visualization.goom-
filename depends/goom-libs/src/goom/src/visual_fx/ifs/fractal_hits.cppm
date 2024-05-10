module;

#include "goom/goom_config.h"

#include <cstddef>
#include <cstdint>
#include <vector>

module Goom.VisualFx.IfsDancersFx:FractalHits;

import Goom.Color.ColorUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import :IfsTypes;
import :Similitudes;

namespace GOOM::VISUAL_FX::IFS
{

class FractalHits
{
public:
  explicit FractalHits(const Dimensions& dimensions) noexcept;
  FractalHits(const FractalHits&) noexcept = default;
  FractalHits(FractalHits&&) noexcept      = default;
  ~FractalHits() noexcept;
  auto operator=(const FractalHits&) noexcept -> FractalHits& = delete;
  auto operator=(FractalHits&&) noexcept -> FractalHits&      = delete;

  void Reset();
  void AddHit(int32_t x, int32_t y, const Similitude& simi);
  [[nodiscard]] auto GetBuffer() -> const std::vector<IfsPoint>&;
  [[nodiscard]] auto GetMaxHitCount() const -> uint32_t;

private:
  Dimensions m_dimensions;
  struct HitInfo
  {
    uint32_t count = 0;
    Pixel color    = BLACK_PIXEL;
    const Similitude* simi{};
  };
  std::vector<std::vector<HitInfo>> m_hitInfo{GetHitInfo(m_dimensions)};
  [[nodiscard]] static auto GetHitInfo(const Dimensions& dimensions) noexcept
      -> std::vector<std::vector<HitInfo>>;
  uint32_t m_maxHitCount                = 0;
  static constexpr size_t HITS_ESTIMATE = 1000;
  std::vector<IfsPoint> m_hits{};
  std::vector<IfsPoint> m_buffer{};
};

inline auto FractalHits::GetMaxHitCount() const -> uint32_t
{
  return m_maxHitCount;
}

} // namespace GOOM::VISUAL_FX::IFS

namespace GOOM::VISUAL_FX::IFS
{

using COLOR::GetColorAverage;

FractalHits::FractalHits(const Dimensions& dimensions) noexcept : m_dimensions{dimensions}
{
  m_hits.reserve(HITS_ESTIMATE);
}

FractalHits::~FractalHits() noexcept = default;

auto FractalHits::GetHitInfo(const Dimensions& dimensions) noexcept
    -> std::vector<std::vector<HitInfo>>
{
  auto hitInfo = std::vector<std::vector<HitInfo>>(dimensions.GetHeight());
  for (auto& xHit : hitInfo)
  {
    xHit.resize(dimensions.GetWidth());
  }
  return hitInfo;
}

void FractalHits::Reset()
{
  for (const auto& hit : m_hits)
  {
    auto& hitInfo = m_hitInfo[hit.GetY()][hit.GetX()];
    hitInfo.count = 0;
  }

  m_hits.clear();
  m_maxHitCount = 0;
}

void FractalHits::AddHit(const int32_t x, const int32_t y, const Similitude& simi)
{
  if ((x < 0) or (y < 0))
  {
    return;
  }
  if ((x >= m_dimensions.GetIntWidth()) or (y >= m_dimensions.GetIntHeight()))
  {
    return;
  }

  auto& hitInfo = m_hitInfo[static_cast<uint32_t>(y)][static_cast<uint32_t>(x)];

  ++hitInfo.count;
  if (hitInfo.count > m_maxHitCount)
  {
    m_maxHitCount = hitInfo.count;
  }

  hitInfo.simi = &simi;
  if (hitInfo.count > 1)
  {
    hitInfo.color = GetColorAverage(hitInfo.color, simi.GetColor());
  }
  else
  {
    hitInfo.color = simi.GetColor();
    m_hits.emplace_back(Point2dInt{x, y}, 1);
  }
}

auto FractalHits::GetBuffer() -> const std::vector<IfsPoint>&
{
  m_buffer.clear();
  m_buffer.reserve(m_hits.size());

  for (const auto& hit : m_hits)
  {
    auto updatedHit     = hit;
    const auto& hitInfo = m_hitInfo[hit.GetY()][hit.GetX()];

    updatedHit.SetCount(hitInfo.count);
    updatedHit.SetColor(hitInfo.color);
    updatedHit.SetSimi(hitInfo.simi);

    m_buffer.emplace_back(updatedHit);
  }

  return m_buffer;
}

} // namespace GOOM::VISUAL_FX::IFS
