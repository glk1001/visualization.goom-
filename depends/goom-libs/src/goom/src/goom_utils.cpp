#include "goom_utils.h"

#include "compiler_versions.h"
#include "goom_version.h"
#include "utils/build_time.h"
#include "utils/math/randutils.h"

namespace GOOM
{

auto GetRandSeed() noexcept -> uint64_t
{
  return UTILS::MATH::RAND::GetRandSeed();
}

auto SetRandSeed(const uint64_t seed) noexcept -> void
{
  UTILS::MATH::RAND::SetRandSeed(seed);
}

auto GetGoomLibCompilerVersion() noexcept -> std::string
{
  return GetCompilerVersion();
}

auto GetGoomLibVersionInfo() noexcept -> std::string
{
  return GetFullVersionStr();
}

auto GetGoomLibBuildTime() noexcept -> std::string
{
  return UTILS::GetBuildTime();
}

} // namespace GOOM
