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
static const std::vector<vivid::srgb_t> Greys_9
{
  {   1.00000f,   1.00000f,   1.00000f },
  {   0.94118f,   0.94118f,   0.94118f },
  {   0.85098f,   0.85098f,   0.85098f },
  {   0.74118f,   0.74118f,   0.74118f },
  {   0.58824f,   0.58824f,   0.58824f },
  {   0.45098f,   0.45098f,   0.45098f },
  {   0.32157f,   0.32157f,   0.32157f },
  {   0.14510f,   0.14510f,   0.14510f },
  {   0.00000f,   0.00000f,   0.00000f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif