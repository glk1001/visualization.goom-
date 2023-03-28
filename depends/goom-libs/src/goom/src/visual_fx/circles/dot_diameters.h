#pragma once

#include "utils/math/goom_rand_base.h"

#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

class DotDiameters
{
public:
  struct DotProperties
  {
    uint32_t numDots;
    uint32_t minDiameter;
    uint32_t maxDiameter;
  };

  DotDiameters(const UTILS::MATH::IGoomRand& goomRand, const DotProperties& dotProperties) noexcept;

  auto ChangeDiameters() noexcept -> void;

  [[nodiscard]] auto GetDiameters() const noexcept -> const std::vector<uint32_t>&;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  uint32_t m_numDots;

  static constexpr uint32_t MIN_DIAMETER_EXTRA = 2;
  uint32_t m_minDiameter;
  uint32_t m_maxDiameter;
  struct NumDotsAndMaxDiameter
  {
    uint32_t numDots;
    uint32_t maxDiameter;
  };
  std::vector<uint32_t> m_diameters{GetInitialDiameters({m_numDots, m_maxDiameter})};
  [[nodiscard]] static auto GetInitialDiameters(
      const NumDotsAndMaxDiameter& numDotsAndMaxDiameter) noexcept -> std::vector<uint32_t>;

  static constexpr float PROB_FIXED_DIAMETER = 0.0F;
  auto ChangeToFixedDiameters() noexcept -> void;
  auto ChangeToVariableDiameters() noexcept -> void;
};

inline auto DotDiameters::GetDiameters() const noexcept -> const std::vector<uint32_t>&
{
  return m_diameters;
}

} // namespace GOOM::VISUAL_FX::CIRCLES
