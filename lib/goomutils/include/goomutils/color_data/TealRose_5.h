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
static const std::vector<vivid::srgb_t> TealRose_5
{
  {   0.00000f,   0.57647f,   0.57255f },
  {   0.69412f,   0.78039f,   0.70196f },
  {   0.94510f,   0.91765f,   0.78431f },
  {   0.89804f,   0.72549f,   0.67843f },
  {   0.81569f,   0.34510f,   0.49412f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
