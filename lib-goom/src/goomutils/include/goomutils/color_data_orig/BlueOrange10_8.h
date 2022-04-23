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
static const std::vector<vivid::srgb_t> BlueOrange10_8
{
  {   0.00000f,   0.32941f,   1.00000f },
  {   0.19608f,   0.60000f,   1.00000f },
  {   0.60000f,   0.92941f,   1.00000f },
  {   0.80000f,   1.00000f,   1.00000f },
  {   1.00000f,   1.00000f,   0.80000f },
  {   1.00000f,   0.93333f,   0.60000f },
  {   1.00000f,   0.60000f,   0.19608f },
  {   1.00000f,   0.33333f,   0.00000f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif