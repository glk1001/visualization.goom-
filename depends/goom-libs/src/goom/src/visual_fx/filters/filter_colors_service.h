#pragma once

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "filter_buffers.h"
#include "goom_graphic.h"
#include "utils/mathutils.h"
#include "utils/name_value_pairs.h"
#include "utils/t_values.h"
#include "utils/timer.h"
#include "v2d.h"

//#undef NDEBUG
#include <cassert>
#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class FilterColorsService
{
public:
  FilterColorsService() noexcept = default;

  void SetBuffSettings(const FXBuffSettings& settings);
  void SetBlockyWavy(bool val);

  using NeighborhoodCoeffArray = ZoomFilterBuffers::NeighborhoodCoeffArray;
  using NeighborhoodPixelArray = ZoomFilterBuffers::NeighborhoodPixelArray;

  [[nodiscard]] auto GetNewColor(const PixelBuffer& srceBuff,
                                 const ZoomFilterBuffers::SourcePointInfo& sourceInfo) const
      -> Pixel;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

private:
  bool m_blockyWavy = false;
  FXBuffSettings m_buffSettings{};

  [[nodiscard]] auto GetFilteredColor(const NeighborhoodCoeffArray& coeffs,
                                      const NeighborhoodPixelArray& pixels) const -> Pixel;
  [[nodiscard]] auto GetMixedColor(const NeighborhoodCoeffArray& coeffs,
                                   const NeighborhoodPixelArray& colors) const -> Pixel;
  [[nodiscard]] auto GetBlockyMixedColor(const NeighborhoodCoeffArray& coeffs,
                                         const NeighborhoodPixelArray& colors) const -> Pixel;
};

inline void FilterColorsService::SetBlockyWavy(const bool val)
{
  m_blockyWavy = val;
}

inline void FilterColorsService::SetBuffSettings(const FXBuffSettings& settings)
{
  m_buffSettings = settings;
}

inline auto FilterColorsService::GetNewColor(
    const PixelBuffer& srceBuff, const ZoomFilterBuffers::SourcePointInfo& sourceInfo) const
    -> Pixel
{
  const NeighborhoodPixelArray pixelNeighbours = srceBuff.Get4RHBNeighbours(
      static_cast<size_t>(sourceInfo.screenPoint.x), static_cast<size_t>(sourceInfo.screenPoint.y));

  return GetFilteredColor(sourceInfo.coeffs, pixelNeighbours);
}

inline auto FilterColorsService::GetFilteredColor(const NeighborhoodCoeffArray& coeffs,
                                                  const NeighborhoodPixelArray& pixels) const
    -> Pixel
{
  if (m_blockyWavy)
  {
    return GetBlockyMixedColor(coeffs, pixels);
  }

  return GetMixedColor(coeffs, pixels);
}

inline auto FilterColorsService::GetBlockyMixedColor(const NeighborhoodCoeffArray& coeffs,
                                                     const NeighborhoodPixelArray& colors) const
    -> Pixel
{
  // Changing the color order gives a strange blocky, wavy look.
  // The order col4, col3, col2, col1 gave a black tear - no so good.
  static_assert(4 == ZoomFilterBuffers::NUM_NEIGHBOR_COEFFS, "NUM_NEIGHBOR_COEFFS must be 4.");
  assert(ZoomFilterBuffers::NUM_NEIGHBOR_COEFFS == coeffs.val.size());
  const NeighborhoodPixelArray reorderedColors{colors[0], colors[2], colors[1], colors[3]};
  return GetMixedColor(coeffs, reorderedColors);
}

inline auto FilterColorsService::GetMixedColor(const NeighborhoodCoeffArray& coeffs,
                                               const NeighborhoodPixelArray& colors) const -> Pixel
{
  if (coeffs.isZero)
  {
    return Pixel::BLACK;
  }

  uint32_t multR = 0;
  uint32_t multG = 0;
  uint32_t multB = 0;
  for (size_t i = 0; i < coeffs.val.size(); ++i)
  {
    const uint32_t& coeff = coeffs.val[i];
    const Pixel& color = colors[i];
    multR += static_cast<uint32_t>(color.R()) * coeff;
    multG += static_cast<uint32_t>(color.G()) * coeff;
    multB += static_cast<uint32_t>(color.B()) * coeff;
  }
  constexpr uint32_t MAX_SUM_COEFFS = channel_limits<uint32_t>::max() + 1;
  uint32_t newR = multR / MAX_SUM_COEFFS;
  uint32_t newG = multG / MAX_SUM_COEFFS;
  uint32_t newB = multB / MAX_SUM_COEFFS;

  if (!m_buffSettings.allowOverexposed)
  {
    return GetPixelScaledByMax(newR, newG, newB, MAX_ALPHA);
  }

  return Pixel{newR, newG, newB, MAX_ALPHA};
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

