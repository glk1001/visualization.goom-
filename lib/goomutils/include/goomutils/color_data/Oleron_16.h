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
static const std::vector<vivid::srgb_t> Oleron_16
{
  {   0.10196f,   0.14902f,   0.34902f },
  {   0.20392f,   0.25098f,   0.45098f },
  {   0.30980f,   0.36078f,   0.56078f },
  {   0.42353f,   0.47451f,   0.67451f },
  {   0.54510f,   0.59216f,   0.79216f },
  {   0.66667f,   0.71765f,   0.90980f },
  {   0.76863f,   0.81961f,   0.96471f },
  {   0.85882f,   0.90980f,   0.98824f },
  {   0.18431f,   0.32157f,   0.00000f },
  {   0.32549f,   0.36863f,   0.00784f },
  {   0.47843f,   0.44314f,   0.12157f },
  {   0.62353f,   0.53725f,   0.27059f },
  {   0.77255f,   0.64314f,   0.42353f },
  {   0.91373f,   0.76863f,   0.58431f },
  {   0.96863f,   0.88235f,   0.74510f },
  {   0.99216f,   0.99216f,   0.90196f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
