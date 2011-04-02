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
static const std::vector<vivid::srgb_t> Plasma_9
{
  {   0.05098f,   0.03137f,   0.52941f },
  {   0.29804f,   0.00784f,   0.63137f },
  {   0.49412f,   0.01176f,   0.65882f },
  {   0.66667f,   0.13725f,   0.58431f },
  {   0.80000f,   0.27843f,   0.47059f },
  {   0.89804f,   0.41961f,   0.36471f },
  {   0.97255f,   0.58039f,   0.25490f },
  {   0.99216f,   0.76471f,   0.15686f },
  {   0.94118f,   0.97647f,   0.12941f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
