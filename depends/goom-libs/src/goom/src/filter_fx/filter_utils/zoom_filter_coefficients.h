#pragma once

#include "goom_graphic.h"
#include "utils/math/misc.h"

#include <array>
#include <cstdint>

namespace GOOM::FILTER_FX::FILTER_UTILS
{

class ZoomFilterCoefficients
{
public:
  static constexpr auto DIM_FILTER_COEFFS_EXP = 4U;
  static constexpr auto DIM_FILTER_COEFFS     = GOOM::UTILS::MATH::PowerOf2(DIM_FILTER_COEFFS_EXP);
  static constexpr auto DIM_FILTER_COEFFS_MOD_MASK = 0xfU;
  static constexpr auto NUM_NEIGHBOR_COEFFS        = 4U;

  struct NeighborhoodCoeffArray
  {
    std::array<uint32_t, NUM_NEIGHBOR_COEFFS> val;
    bool isZero;
  };
  using NeighborhoodPixelArray = std::array<Pixel, NUM_NEIGHBOR_COEFFS>;
  using FilterCoeff2dArray =
      std::array<std::array<NeighborhoodCoeffArray, DIM_FILTER_COEFFS>, DIM_FILTER_COEFFS>;

  // TODO(glk) Can be static.
  [[nodiscard]] auto GetCoeffs() const noexcept -> const FilterCoeff2dArray&;

private:
  // modif d'optim by Jeko : precalcul des 4 coeffs resultant des 2 pos
  const FilterCoeff2dArray m_precalculatedCoeffs{GetPrecalculatedCoefficients()};
  [[nodiscard]] static auto GetPrecalculatedCoefficients() noexcept -> FilterCoeff2dArray;
  [[nodiscard]] static auto GetNeighborhoodCoeffArray(uint32_t coeffH, uint32_t coeffV) noexcept
      -> NeighborhoodCoeffArray;
};

inline auto ZoomFilterCoefficients::GetCoeffs() const noexcept -> const FilterCoeff2dArray&
{
  return m_precalculatedCoeffs;
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
