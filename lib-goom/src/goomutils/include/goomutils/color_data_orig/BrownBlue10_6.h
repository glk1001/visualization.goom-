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
static const std::vector<vivid::srgb_t> BrownBlue10_6
{
  {   0.40000f,   0.18431f,   0.00000f },
  {   0.80000f,   0.60784f,   0.47843f },
  {   0.94902f,   0.85490f,   0.80392f },
  {   0.80000f,   0.99216f,   1.00000f },
  {   0.39608f,   0.93725f,   1.00000f },
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
