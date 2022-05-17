#pragma once

#include "filter_buffers.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "utils/name_value_pairs.h"
#include "utils/timer.h"

#include <string>

namespace GOOM::VISUAL_FX::FILTERS
{

class FilterColorsService
{
public:
  FilterColorsService() noexcept = default;

  void SetBrightness(float brightness) noexcept;
  void SetBuffSettings(const FXBuffSettings& settings) noexcept;
  void SetBlockyWavy(bool val) noexcept;

  using NeighborhoodCoeffArray = ZoomFilterBuffers::NeighborhoodCoeffArray;
  using NeighborhoodPixelArray = ZoomFilterBuffers::NeighborhoodPixelArray;

  [[nodiscard]] auto GetNewColor(
      const PixelBuffer& srceBuff,
      const ZoomFilterBuffers::SourcePointInfo& sourceInfo) const noexcept -> Pixel;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const noexcept
      -> UTILS::NameValuePairs;

private:
  bool m_blockyWavy = false;
  FXBuffSettings m_buffSettings{};

  static constexpr uint32_t MAX_SUM_COEFFS = channel_limits<uint32_t>::max() + 1;
  uint32_t m_coeffsAndBrightnessDivisor = MAX_SUM_COEFFS;

  [[nodiscard]] auto GetFilteredColor(const NeighborhoodCoeffArray& coeffs,
                                      const NeighborhoodPixelArray& pixels) const noexcept -> Pixel;
  [[nodiscard]] auto GetMixedColor(const NeighborhoodCoeffArray& coeffs,
                                   const NeighborhoodPixelArray& colors) const noexcept -> Pixel;
  [[nodiscard]] auto GetBlockyMixedColor(const NeighborhoodCoeffArray& coeffs,
                                         const NeighborhoodPixelArray& colors) const noexcept
      -> Pixel;
};

inline void FilterColorsService::SetBrightness(const float brightness) noexcept
{
  // In method 'GetMixedColor' we multiply an array of coefficients by an array of colors
  // and get a sum as the result. The sum is then divided by 'MAX_SUM_COEFFS'. For optimizing
  // reasons, we can use this step to also factor in integer brightness for free.
  //
  //  (c/x) * (m/n) = (c*m) / (x*n) = c / (x*n / m)

  static constexpr uint32_t X = MAX_SUM_COEFFS;
  static constexpr uint32_t N = channel_limits<uint32_t>::max();
  const uint32_t m = std::max(1U, static_cast<uint32_t>(brightness * channel_limits<float>::max()));

  m_coeffsAndBrightnessDivisor = (X * N) / m;
}

inline void FilterColorsService::SetBlockyWavy(const bool val) noexcept
{
  m_blockyWavy = val;
}

inline void FilterColorsService::SetBuffSettings(const FXBuffSettings& settings) noexcept
{
  m_buffSettings = settings;
}

inline auto FilterColorsService::GetNewColor(
    const PixelBuffer& srceBuff,
    const ZoomFilterBuffers::SourcePointInfo& sourceInfo) const noexcept -> Pixel
{
  const NeighborhoodPixelArray pixelNeighbours = srceBuff.Get4RHBNeighbours(
      static_cast<size_t>(sourceInfo.screenPoint.x), static_cast<size_t>(sourceInfo.screenPoint.y));

  return GetFilteredColor(sourceInfo.coeffs, pixelNeighbours);
}

inline auto FilterColorsService::GetFilteredColor(
    const NeighborhoodCoeffArray& coeffs, const NeighborhoodPixelArray& pixels) const noexcept
    -> Pixel
{
  if (m_blockyWavy)
  {
    return GetBlockyMixedColor(coeffs, pixels);
  }

  return GetMixedColor(coeffs, pixels);
}

inline auto FilterColorsService::GetBlockyMixedColor(
    const NeighborhoodCoeffArray& coeffs, const NeighborhoodPixelArray& colors) const noexcept
    -> Pixel
{
  // Changing the color order gives a strange blocky, wavy look.
  // The order col4, col3, col2, col1 gave a black tear - not so good.

  Expects(ZoomFilterBuffers::NUM_NEIGHBOR_COEFFS == coeffs.val.size());
  static constexpr size_t ALLOWED_NUM_NEIGHBORS = 4;
  static_assert(ALLOWED_NUM_NEIGHBORS == ZoomFilterBuffers::NUM_NEIGHBOR_COEFFS);

  const NeighborhoodPixelArray reorderedColors{colors[0], colors[2], colors[1], colors[3]};
  return GetMixedColor(coeffs, reorderedColors);
}

inline auto FilterColorsService::GetMixedColor(const NeighborhoodCoeffArray& coeffs,
                                               const NeighborhoodPixelArray& colors) const noexcept
    -> Pixel
{
  if (coeffs.isZero)
  {
    return BLACK_PIXEL;
  }

  uint32_t multR = 0;
  uint32_t multG = 0;
  uint32_t multB = 0;
  for (size_t i = 0; i < ZoomFilterBuffers::NUM_NEIGHBOR_COEFFS; ++i)
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    const uint32_t& coeff = coeffs.val[i];
    const Pixel& color = colors[i];
    multR += static_cast<uint32_t>(color.R()) * coeff;
    multG += static_cast<uint32_t>(color.G()) * coeff;
    multB += static_cast<uint32_t>(color.B()) * coeff;
  }
  const uint32_t newR = multR / m_coeffsAndBrightnessDivisor;
  const uint32_t newG = multG / m_coeffsAndBrightnessDivisor;
  const uint32_t newB = multB / m_coeffsAndBrightnessDivisor;

  return Pixel{newR, newG, newB, MAX_ALPHA};
}

} // namespace GOOM::VISUAL_FX::FILTERS

