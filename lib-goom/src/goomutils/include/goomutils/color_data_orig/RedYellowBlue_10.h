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
static const std::vector<vivid::srgb_t> RedYellowBlue_10
{
  {   0.64706f,   0.00000f,   0.12941f },
  {   0.84706f,   0.14902f,   0.19608f },
  {   0.96863f,   0.42745f,   0.36863f },
  {   1.00000f,   0.67843f,   0.44706f },
  {   1.00000f,   0.87843f,   0.60000f },
  {   0.87843f,   1.00000f,   1.00000f },
  {   0.66667f,   0.96863f,   1.00000f },
  {   0.44706f,   0.84706f,   1.00000f },
  {   0.24706f,   0.62745f,   1.00000f },
  {   0.14902f,   0.29804f,   1.00000f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif