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
static const std::vector<vivid::srgb_t> Aquatic1_5
{
  {   0.20392f,   0.14118f,   0.09804f },
  {   0.10980f,   0.25098f,   0.15294f },
  {   0.94510f,   0.78824f,   0.05490f },
  {   0.40000f,   0.34510f,   0.60000f },
  {   0.72157f,   0.57647f,   0.50980f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif