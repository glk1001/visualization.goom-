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
static const std::vector<vivid::srgb_t> PiYG_10
{
  {   0.55686f,   0.00392f,   0.32157f },
  {   0.77255f,   0.10588f,   0.49020f },
  {   0.87059f,   0.46667f,   0.68235f },
  {   0.94510f,   0.71373f,   0.85490f },
  {   0.99216f,   0.87843f,   0.93725f },
  {   0.90196f,   0.96078f,   0.81569f },
  {   0.72157f,   0.88235f,   0.52549f },
  {   0.49804f,   0.73725f,   0.25490f },
  {   0.30196f,   0.57255f,   0.12941f },
  {   0.15294f,   0.39216f,   0.09804f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif