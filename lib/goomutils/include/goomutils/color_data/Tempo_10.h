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
static const std::vector<vivid::srgb_t> Tempo_10
{
  {   1.00000f,   0.96471f,   0.95686f },
  {   0.84706f,   0.86667f,   0.80000f },
  {   0.67451f,   0.77647f,   0.66275f },
  {   0.49020f,   0.70196f,   0.56471f },
  {   0.27843f,   0.62353f,   0.50980f },
  {   0.08627f,   0.52941f,   0.48627f },
  {   0.08235f,   0.42745f,   0.45098f },
  {   0.10980f,   0.32549f,   0.39216f },
  {   0.10588f,   0.21961f,   0.32549f },
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
