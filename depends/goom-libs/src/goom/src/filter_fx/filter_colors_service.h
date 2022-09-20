#pragma once

#include "filter_buffers.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "utils/name_value_pairs.h"
#include "utils/timer.h"

#include <libdivide.h>
#include <string>

namespace GOOM::FILTER_FX
{

class FilterColorsService
{
public:
  FilterColorsService() noexcept = default;

  auto SetBrightness(float brightness) noexcept -> void;
  auto SetBuffSettings(const FXBuffSettings& settings) noexcept -> void;
  auto SetBlockyWavy(bool val) noexcept -> void;

  using NeighborhoodCoeffArray = ZoomFilterBuffers::NeighborhoodCoeffArray;
  using NeighborhoodPixelArray = ZoomFilterBuffers::NeighborhoodPixelArray;

  [[nodiscard]] auto GetNewColor(
      const ZoomFilterBuffers::SourcePointInfo& sourcePointInfo,
      const std::array<Pixel, PixelBuffer::NUM_NBRS>& sourcePointNeighbours) const noexcept
      -> Pixel;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const noexcept
      -> UTILS::NameValuePairs;

private:
  bool m_blockyWavy = false;
  FXBuffSettings m_buffSettings{};

  static constexpr uint32_t MAX_SUM_COEFFS = channel_limits<uint32_t>::max() + 1;
  libdivide::divider<uint32_t> m_coeffsAndBrightnessFastDivisor{MAX_SUM_COEFFS};

  [[nodiscard]] auto GetFilteredColor(const NeighborhoodCoeffArray& coeffs,
                                      const NeighborhoodPixelArray& pixels) const noexcept -> Pixel;
  [[nodiscard]] auto GetMixedColor(const NeighborhoodCoeffArray& coeffs,
                                   const NeighborhoodPixelArray& colors) const noexcept -> Pixel;
  [[nodiscard]] auto GetBlockyMixedColor(const NeighborhoodCoeffArray& coeffs,
                                         const NeighborhoodPixelArray& colors) const noexcept
      -> Pixel;
};

inline auto FilterColorsService::SetBrightness(const float brightness) noexcept -> void
{
  // In method 'GetMixedColor' we multiply an array of coefficients by an array of colors
  // and get a sum as the result. The sum is then divided by 'MAX_SUM_COEFFS'. For optimizing
  // reasons, we can use this step to also factor in integer brightness for free.
  //
  //  (c/x) * (m/n) = (c*m) / (x*n) = c / (x*n / m)

  static constexpr auto X = MAX_SUM_COEFFS;
  static constexpr auto N = channel_limits<uint32_t>::max();
  const auto m = std::max(1U, static_cast<uint32_t>(brightness * channel_limits<float>::max()));

  m_coeffsAndBrightnessFastDivisor = libdivide::divider<uint32_t>{(X * N) / m};
}

inline auto FilterColorsService::SetBlockyWavy(const bool val) noexcept -> void
{
  m_blockyWavy = val;
}

inline void FilterColorsService::SetBuffSettings(const FXBuffSettings& settings) noexcept
{
  m_buffSettings = settings;
}

inline auto FilterColorsService::GetNewColor(
    const ZoomFilterBuffers::SourcePointInfo& sourcePointInfo,
    const std::array<Pixel, PixelBuffer::NUM_NBRS>& sourcePointNeighbours) const noexcept -> Pixel
{
  return GetFilteredColor(sourcePointInfo.coeffs, sourcePointNeighbours);
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
  static constexpr auto ALLOWED_NUM_NEIGHBORS = 4U;
  static_assert(ALLOWED_NUM_NEIGHBORS == ZoomFilterBuffers::NUM_NEIGHBOR_COEFFS);

  const auto reorderedColors = NeighborhoodPixelArray{colors[0], colors[2], colors[1], colors[3]};
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

  auto multR = 0U;
  auto multG = 0U;
  auto multB = 0U;
  for (auto i = 0U; i < ZoomFilterBuffers::NUM_NEIGHBOR_COEFFS; ++i)
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    const auto coeff = coeffs.val[i];
    const auto color = colors[i];
    multR += static_cast<uint32_t>(color.R()) * coeff;
    multG += static_cast<uint32_t>(color.G()) * coeff;
    multB += static_cast<uint32_t>(color.B()) * coeff;
  }

  const auto newR = static_cast<PixelChannelType>(multR / m_coeffsAndBrightnessFastDivisor);
  const auto newG = static_cast<PixelChannelType>(multG / m_coeffsAndBrightnessFastDivisor);
  const auto newB = static_cast<PixelChannelType>(multB / m_coeffsAndBrightnessFastDivisor);

  return Pixel{
      Pixel::RGB{newR, newG, newB, MAX_ALPHA}
  };
}

} // namespace GOOM::FILTER_FX
