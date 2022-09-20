#include "zoom_filter_coefficients.h"

//#undef NO_LOGGING

#include "goom_config.h"
#include "goom_graphic.h"
#include "logging.h"

#include <algorithm>
#include <numeric>

namespace GOOM::FILTER_FX::FILTER_UTILS
{

using GOOM::UTILS::Logging; // NOLINT(misc-unused-using-decls)

auto ZoomFilterCoefficients::GetPrecalculatedCoefficients() noexcept -> FilterCoeff2dArray
{
  auto precalculatedCoeffs = FilterCoeff2dArray{};

  for (auto coeffH = 0U; coeffH < DIM_FILTER_COEFFS; ++coeffH)
  {
    for (auto coeffV = 0U; coeffV < DIM_FILTER_COEFFS; ++coeffV)
    {
      precalculatedCoeffs[coeffH][coeffV] = GetNeighborhoodCoeffArray(coeffH, coeffV);
    }
  }

  return precalculatedCoeffs;
}

auto ZoomFilterCoefficients::GetNeighborhoodCoeffArray(
    const uint32_t coeffH, const uint32_t coeffV) noexcept -> NeighborhoodCoeffArray
{
  const auto diffCoeffH = DIM_FILTER_COEFFS - coeffH;
  const auto diffCoeffV = DIM_FILTER_COEFFS - coeffV;

  // clang-format off
  auto coeffs = std::array<uint32_t, NUM_NEIGHBOR_COEFFS>{
      diffCoeffH * diffCoeffV,
      coeffH * diffCoeffV,
      diffCoeffH * coeffV,
      coeffH * coeffV
  };
  // clang-format on

  // We want to decrement just one coefficient so that the sum of
  // coefficients equals 255. We'll choose the max coefficient.
  const auto maxCoeff = *std::max_element(cbegin(coeffs), cend(coeffs));
  auto allZero        = false;
  if (0 == maxCoeff)
  {
    allZero = true;
  }
  else
  {
    for (auto& coeff : coeffs)
    {
      if (maxCoeff == coeff)
      {
        --coeff;
        break;
      }
    }
  }

  LogInfo("{:2}, {:2}:  {:3}, {:3}, {:3}, {:3} - sum: {:3}",
          coeffH,
          coeffV,
          coeffs[0],
          coeffs[1],
          coeffs[2],
          coeffs[3],
          std::accumulate(cbegin(coeffs), cend(coeffs), 0U)); // NOLINT
  Ensures(channel_limits<uint32_t>::max() == std::accumulate(cbegin(coeffs), cend(coeffs), 0U));

  return {coeffs, allZero};
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
