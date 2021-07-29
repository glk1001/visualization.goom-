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
static const std::vector<vivid::srgb_t> RdYlGn_5
{
  {   0.84314f,   0.09804f,   0.10980f },
  {   0.99216f,   0.68235f,   0.38039f },
  {   1.00000f,   1.00000f,   0.74902f },
  {   0.65098f,   0.85098f,   0.41569f },
  {   0.10196f,   0.58824f,   0.25490f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif