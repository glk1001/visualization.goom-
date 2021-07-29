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
static const std::vector<vivid::srgb_t> Purples_5
{
  {   0.94902f,   0.94118f,   0.96863f },
  {   0.79608f,   0.78824f,   0.88627f },
  {   0.61961f,   0.60392f,   0.78431f },
  {   0.45882f,   0.41961f,   0.69412f },
  {   0.32941f,   0.15294f,   0.56078f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif