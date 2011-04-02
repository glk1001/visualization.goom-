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
static const std::vector<vivid::srgb_t> Tempo_6
{
  {   1.00000f,   0.96471f,   0.95686f },
  {   0.71373f,   0.79608f,   0.68627f },
  {   0.36471f,   0.65490f,   0.52549f },
  {   0.06667f,   0.49020f,   0.47451f },
  {   0.10980f,   0.30196f,   0.38039f },
  {   0.08235f,   0.11373f,   0.26667f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
