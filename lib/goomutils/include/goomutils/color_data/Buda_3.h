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
static const std::vector<vivid::srgb_t> Buda_3
{
  {   0.70196f,   0.00392f,   0.70196f },
  {   0.80392f,   0.52157f,   0.49412f },
  {   1.00000f,   1.00000f,   0.40000f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif