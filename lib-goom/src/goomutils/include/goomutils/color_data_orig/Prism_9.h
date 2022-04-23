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
static const std::vector<vivid::srgb_t> Prism_9
{
  {   0.37255f,   0.27451f,   0.56471f },
  {   0.11373f,   0.41176f,   0.58824f },
  {   0.21961f,   0.65098f,   0.64706f },
  {   0.05882f,   0.52157f,   0.32941f },
  {   0.45098f,   0.68627f,   0.28235f },
  {   0.92941f,   0.67843f,   0.03137f },
  {   0.88235f,   0.48627f,   0.01961f },
  {   0.80000f,   0.31373f,   0.24314f },
  {   0.58039f,   0.20392f,   0.43137f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif