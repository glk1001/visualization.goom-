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
static const std::vector<vivid::srgb_t> Delta_9
{
  {   0.06667f,   0.12549f,   0.25098f },
  {   0.10980f,   0.32157f,   0.61569f },
  {   0.20000f,   0.56863f,   0.66275f },
  {   0.59216f,   0.77255f,   0.74510f },
  {   1.00000f,   0.99216f,   0.80392f },
  {   0.79608f,   0.73333f,   0.27451f },
  {   0.38039f,   0.57255f,   0.04314f },
  {   0.04314f,   0.37647f,   0.17647f },
  {   0.09020f,   0.13725f,   0.07451f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
