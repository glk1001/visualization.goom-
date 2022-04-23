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
static const std::vector<vivid::srgb_t> Davos_7
{
  {   0.00000f,   0.01961f,   0.29020f },
  {   0.10980f,   0.24706f,   0.51373f },
  {   0.26275f,   0.43922f,   0.61569f },
  {   0.42353f,   0.55686f,   0.57647f },
  {   0.60000f,   0.67843f,   0.53333f },
  {   0.88235f,   0.89804f,   0.71373f },
  {   0.99608f,   0.99608f,   0.99608f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif