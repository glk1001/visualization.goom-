#pragma once

#include "goom_rand_base.h"
#include "randutils.h"

#include <cstdint>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace UTILS
{
#else
namespace GOOM::UTILS
{
#endif

class GoomRand : public IGoomRand
{
public:
  [[nodiscard]] auto GetNRand(uint32_t n1) const -> uint32_t override;
  [[nodiscard]] auto GetRand() const -> uint32_t override;

  [[nodiscard]] auto GetRandInRange(uint32_t n0, uint32_t n1) const -> uint32_t override;
  [[nodiscard]] auto GetRandInRange(int32_t n0, int32_t n1) const -> int32_t override;
  [[nodiscard]] auto GetRandInRange(float n0, float n1) const -> float override;

  [[nodiscard]] auto ProbabilityOfMInN(uint32_t m, uint32_t n) const -> bool override;
  [[nodiscard]] auto ProbabilityOf(float x) const -> bool override;
};

inline auto GoomRand::GetNRand(const uint32_t n1) const -> uint32_t
{
  return RAND::GetNRand(n1);
}

inline auto GoomRand::GetRand() const -> uint32_t
{
  return RAND::GetRand();
}

inline auto GoomRand::GetRandInRange(const uint32_t n0, const uint32_t n1) const -> uint32_t
{
  return RAND::GetRandInRange(n0, n1);
}

inline auto GoomRand::GetRandInRange(const int32_t n0, const int32_t n1) const -> int32_t
{
  return RAND::GetRandInRange(n0, n1);
}

inline auto GoomRand::GetRandInRange(const float n0, const float n1) const -> float
{
  return RAND::GetRandInRange(n0, n1);
}

inline auto GoomRand::ProbabilityOfMInN(const uint32_t m, const uint32_t n) const -> bool
{
  return RAND::ProbabilityOfMInN(m, n);
}

inline auto GoomRand::ProbabilityOf(const float x) const -> bool
{
  return RAND::ProbabilityOf(x);
}

#if __cplusplus <= 201402L
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS
#endif
