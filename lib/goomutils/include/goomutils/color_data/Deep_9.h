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
static const std::vector<vivid::srgb_t> Deep_9
{
  {   0.99216f,   0.99608f,   0.80000f },
  {   0.73333f,   0.90196f,   0.67451f },
  {   0.47059f,   0.80784f,   0.63922f },
  {   0.33333f,   0.68235f,   0.63922f },
  {   0.28235f,   0.55686f,   0.61961f },
  {   0.24706f,   0.43529f,   0.59216f },
  {   0.25098f,   0.30196f,   0.54902f },
  {   0.23137f,   0.19608f,   0.36078f },
  {   0.15686f,   0.10196f,   0.17255f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
