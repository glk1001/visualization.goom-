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
static const std::vector<vivid::srgb_t> Pastel_5
{
  {   0.40000f,   0.77255f,   0.80000f },
  {   0.96471f,   0.81176f,   0.44314f },
  {   0.97255f,   0.61176f,   0.45490f },
  {   0.86275f,   0.69020f,   0.94902f },
  {   0.52941f,   0.77255f,   0.37255f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
