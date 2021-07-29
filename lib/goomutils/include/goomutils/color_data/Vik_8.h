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
static const std::vector<vivid::srgb_t> Vik_8
{
  {   0.00000f,   0.07059f,   0.38039f },
  {   0.01176f,   0.29020f,   0.52157f },
  {   0.29412f,   0.56471f,   0.70196f },
  {   0.74118f,   0.83922f,   0.89020f },
  {   0.90588f,   0.77647f,   0.69804f },
  {   0.79216f,   0.50980f,   0.34510f },
  {   0.61176f,   0.21569f,   0.03529f },
  {   0.34902f,   0.00000f,   0.03137f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif