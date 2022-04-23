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
static const std::vector<vivid::srgb_t> Purples_9
{
  {   0.98824f,   0.98431f,   0.99216f },
  {   0.93725f,   0.92941f,   0.96078f },
  {   0.85490f,   0.85490f,   0.92157f },
  {   0.73725f,   0.74118f,   0.86275f },
  {   0.61961f,   0.60392f,   0.78431f },
  {   0.50196f,   0.49020f,   0.72941f },
  {   0.41569f,   0.31765f,   0.63922f },
  {   0.32941f,   0.15294f,   0.56078f },
  {   0.24706f,   0.00000f,   0.49020f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif