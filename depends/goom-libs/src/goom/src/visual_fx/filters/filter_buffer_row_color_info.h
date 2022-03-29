#pragma once

#include "goom_graphic.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::FILTERS
{

class FilterBufferColorInfo
{
public:
  class FilterBufferRowColorInfo
  {
  public:
    void Reset();
    void UpdateColor(size_t x, const Pixel& color);

  private:
    uint32_t m_numNonzeroInRow = 0;
    uint32_t m_sumRedInRow = 0;
    uint32_t m_sumGreenInRow = 0;
    uint32_t m_sumBlueInRow = 0;
    friend class FilterBufferColorInfo;
  };

  explicit FilterBufferColorInfo(uint32_t height) noexcept;

  [[nodiscard]] auto GetRow(size_t y) -> FilterBufferRowColorInfo&;
  [[nodiscard]] auto GetAverageLuminance() const -> float;

private:
  std::vector<FilterBufferRowColorInfo> m_filterBufferColorInfo;
};

inline FilterBufferColorInfo::FilterBufferColorInfo(const uint32_t height) noexcept
  : m_filterBufferColorInfo(height)
{
}

inline auto FilterBufferColorInfo::GetRow(const size_t y) -> FilterBufferRowColorInfo&
{
  return m_filterBufferColorInfo[y];
}

inline void FilterBufferColorInfo::FilterBufferRowColorInfo::Reset()
{
  m_numNonzeroInRow = 0;
  m_sumRedInRow = 0;
  m_sumGreenInRow = 0;
  m_sumBlueInRow = 0;
}

inline void FilterBufferColorInfo::FilterBufferRowColorInfo::UpdateColor(
    [[maybe_unused]] const size_t x, const Pixel& color)
{
  if (color.IsBlack())
  {
    return;
  }

  ++m_numNonzeroInRow;
  m_sumRedInRow += color.R();
  m_sumGreenInRow += color.G();
  m_sumBlueInRow += color.B();
}


} // namespace GOOM::VISUAL_FX::FILTERS
