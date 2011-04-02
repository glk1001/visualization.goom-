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
static const std::vector<vivid::srgb_t> Berlin_9
{
  {   0.61961f,   0.69020f,   1.00000f },
  {   0.31765f,   0.62353f,   0.82745f },
  {   0.15686f,   0.40784f,   0.52549f },
  {   0.07843f,   0.18824f,   0.24314f },
  {   0.09804f,   0.04706f,   0.03529f },
  {   0.24706f,   0.07059f,   0.00392f },
  {   0.48235f,   0.19608f,   0.10980f },
  {   0.73725f,   0.42745f,   0.38039f },
  {   1.00000f,   0.67843f,   0.67843f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
