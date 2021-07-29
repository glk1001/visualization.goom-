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
static const std::vector<vivid::srgb_t> Magma_10
{
  {   0.00000f,   0.00000f,   0.01569f },
  {   0.09412f,   0.05882f,   0.23922f },
  {   0.27059f,   0.06275f,   0.46667f },
  {   0.44706f,   0.12157f,   0.50588f },
  {   0.61961f,   0.18431f,   0.49804f },
  {   0.80392f,   0.25098f,   0.44314f },
  {   0.94510f,   0.37647f,   0.36471f },
  {   0.99216f,   0.58039f,   0.40392f },
  {   0.99608f,   0.79216f,   0.55294f },
  {   0.98824f,   0.99216f,   0.74902f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif