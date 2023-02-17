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
    uint32_t minDotDiameter;
    uint32_t maxDotDiameter;
  };

  DotDiameters(const UTILS::MATH::IGoomRand& goomRand, const DotProperties& dotProperties) noexcept;

  auto ChangeDotDiameters() noexcept -> void;

  [[nodiscard]] auto GetDotDiameters() const noexcept -> const std::vector<uint32_t>&;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  uint32_t m_numDots;

  static constexpr uint32_t MIN_DIAMETER_EXTRA = 2;
  uint32_t m_minDotDiameter;
  uint32_t m_maxDotDiameter;
  struct NumDotsAndMaxDiameter
  {
    uint32_t numDots;
    uint32_t maxDotDiameter;
  };
  std::vector<uint32_t> m_dotDiameters{GetInitialDotDiameters({m_numDots, m_maxDotDiameter})};
  [[nodiscard]] static auto GetInitialDotDiameters(
      const NumDotsAndMaxDiameter& numDotsAndMaxDiameter) noexcept -> std::vector<uint32_t>;

  static constexpr float PROB_FIXED_DIAMETER = 0.0F;
  auto ChangeToFixedDotDiameters() noexcept -> void;
  auto ChangeToVariableDotDiameters() noexcept -> void;
};

inline auto DotDiameters::GetDotDiameters() const noexcept -> const std::vector<uint32_t>&
{
  return m_dotDiameters;
}

} // namespace GOOM::VISUAL_FX::CIRCLES
