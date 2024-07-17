module;

#include <cstdint>
#include <limits>

export module Goom.Utils.Math.Rand.RandGen;

export namespace GOOM::UTILS::MATH::RAND::GEN
{

inline constexpr auto GOOM_RAND_MAX = std::numeric_limits<uint32_t>::max();

[[nodiscard]] auto GetRandSeed() noexcept -> uint64_t;
auto SetRandSeed(uint64_t seed) noexcept -> void;

// Return a random positive integer x in the range 0 <= x < n.
auto Generate(uint32_t n) noexcept -> uint32_t;

} // namespace GOOM::UTILS::MATH::RAND::GEN
