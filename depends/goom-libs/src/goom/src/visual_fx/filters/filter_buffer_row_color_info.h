#pragma once

#include "goom_graphic.h"

#include <vector>

namespace GOOM::VISUAL_FX::FILTERS
{

class FilterBufferRowColorInfo
{
public:
  FilterBufferRowColorInfo() noexcept = default;

  void Reset();
  void UpdateColor(const Pixel& color);

  [[nodiscard]] static auto GetBufferAverageLuminance(
      const std::vector<FilterBufferRowColorInfo>& filterBufferColorInfo) -> float;

private:
  uint32_t m_numNonzeroInRow = 0;
  uint32_t m_sumRedInRow = 0;
  uint32_t m_sumGreenInRow = 0;
  uint32_t m_sumBlueInRow = 0;
};

inline void FilterBufferRowColorInfo::Reset()
{
  m_numNonzeroInRow = 0;
  m_sumRedInRow = 0;
  m_sumGreenInRow = 0;
  m_sumBlueInRow = 0;
}

inline void FilterBufferRowColorInfo::UpdateColor(const Pixel& color)
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
