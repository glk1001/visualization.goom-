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
static const std::vector<vivid::srgb_t> Nuuk_19
{
  {   0.01961f,   0.34902f,   0.54902f },
  {   0.10196f,   0.36471f,   0.52941f },
  {   0.16078f,   0.38431f,   0.51765f },
  {   0.21961f,   0.41176f,   0.50980f },
  {   0.29020f,   0.44706f,   0.51373f },
  {   0.36078f,   0.48627f,   0.52941f },
  {   0.43529f,   0.52941f,   0.55294f },
  {   0.50588f,   0.56863f,   0.57255f },
  {   0.57255f,   0.61176f,   0.58824f },
  {   0.63137f,   0.65098f,   0.59608f },
  {   0.67059f,   0.67843f,   0.58824f },
  {   0.70196f,   0.70588f,   0.57255f },
  {   0.72941f,   0.72549f,   0.55294f },
  {   0.75294f,   0.74902f,   0.53333f },
  {   0.78039f,   0.77647f,   0.51765f },
  {   0.81961f,   0.81569f,   0.51765f },
  {   0.87843f,   0.87843f,   0.55686f },
  {   0.94118f,   0.94118f,   0.61961f },
  {   0.99608f,   0.99608f,   0.69804f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
