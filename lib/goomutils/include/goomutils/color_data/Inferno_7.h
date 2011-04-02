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
static const std::vector<vivid::srgb_t> Inferno_7
{
  {   0.00000f,   0.00000f,   0.01569f },
  {   0.19608f,   0.03922f,   0.36863f },
  {   0.47059f,   0.10980f,   0.42745f },
  {   0.73725f,   0.21569f,   0.32941f },
  {   0.92941f,   0.41176f,   0.14510f },
  {   0.98824f,   0.70588f,   0.09412f },
  {   0.98824f,   1.00000f,   0.64314f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
