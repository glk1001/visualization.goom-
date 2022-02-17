#pragma once

#include <cmath>
#include <cstdint>

#if __cplusplus > 201703L
#pragma message("WARNING " __FILE__ " is obsolete when compiling with C++20. Use std math headers.")
#endif

namespace STD20
{
constexpr float pi = 3.14159265358979323846264F;

template<typename _Fp>
[[nodiscard]] constexpr _Fp __lerp(_Fp __a, _Fp __b, _Fp __t) noexcept
{
  if ((__a <= 0 && __b >= 0) || (__a >= 0 && __b <= 0))
  {
    return __t * __b + (1 - __t) * __a;
  }

  if (1 == __t)
  {
    return __b; // exact
  }

  // Exact at __t=0, monotonic except near __t=1,
  // bounded, determinate, and consistent:
  const _Fp __x = __a + __t * (__b - __a);
  return (__t > 1) == (__b > __a) ? (__b < __x ? __x : __b)
                                  : (__b > __x ? __x : __b); // monotonic near __t=1
}

[[nodiscard]] constexpr auto lerp(const float __a, const float __b, const float __t) noexcept
    -> float
{
  return __lerp(__a, __b, __t);
}
[[nodiscard]] constexpr auto lerp(const double __a, const double __b, const double __t) noexcept
    -> double
{
  return __lerp(__a, __b, __t);
}
[[nodiscard]] inline auto lerp(const uint32_t a, const uint32_t b, const float t) noexcept
    -> uint32_t
{
  return static_cast<uint32_t>(
      std::round(STD20::lerp(static_cast<float>(a), static_cast<float>(b), t)));
}
[[nodiscard]] inline auto lerp(const int32_t a, const int32_t b, const float t) noexcept -> int32_t
{
  return static_cast<int32_t>(
      std::round(STD20::lerp(static_cast<float>(a), static_cast<float>(b), t)));
}

} // namespace STD20
