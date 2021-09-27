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
static const std::vector<vivid::srgb_t> TealGrn_7
{
  {   0.69020f,   0.94902f,   0.73725f },
  {   0.53725f,   0.90980f,   0.67451f },
  {   0.40392f,   0.85882f,   0.64706f },
  {   0.29804f,   0.78431f,   0.63922f },
  {   0.21961f,   0.69804f,   0.63922f },
  {   0.17255f,   0.59608f,   0.62745f },
  {   0.14510f,   0.49020f,   0.59608f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
