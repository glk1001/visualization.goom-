#pragma once

#include "vivid/types.h"
#include <vector>

#if __cplusplus > 201402L
namespace GOOM::UTILS::COLOR_DATA
{
#else
namespace GOOM
{
namespace UTILS
{
namespace COLOR_DATA
{
#endif

// clang-format off
static const std::vector<vivid::srgb_t> BrownBlue10_4
{
  {   0.40000f,   0.18431f,   0.00000f },
  {   0.84706f,   0.68627f,   0.59216f },
  {   0.60000f,   0.97255f,   1.00000f },
  {   0.00000f,   0.66275f,   0.80000f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
