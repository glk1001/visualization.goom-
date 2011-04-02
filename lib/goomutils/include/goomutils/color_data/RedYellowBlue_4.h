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
static const std::vector<vivid::srgb_t> RedYellowBlue_4
{
  {   0.64706f,   0.00000f,   0.12941f },
  {   1.00000f,   0.67843f,   0.44706f },
  {   0.66667f,   0.96863f,   1.00000f },
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
