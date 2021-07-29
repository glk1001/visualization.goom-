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
static const std::vector<vivid::srgb_t> LaPaz_9
{
  {   0.10196f,   0.04706f,   0.39216f },
  {   0.14118f,   0.19608f,   0.49412f },
  {   0.17647f,   0.32549f,   0.57647f },
  {   0.23922f,   0.44314f,   0.62745f },
  {   0.36078f,   0.54902f,   0.63922f },
  {   0.52157f,   0.61961f,   0.61176f },
  {   0.70196f,   0.67451f,   0.58824f },
  {   0.91765f,   0.80784f,   0.72549f },
  {   0.99608f,   0.94902f,   0.95294f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif