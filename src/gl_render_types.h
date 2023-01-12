#pragma once

#include "goom/goom_graphic.h"

#include <cstdint>
#include <functional>
#include <memory>

namespace GOOM
{

struct TextureBufferDimensions
{
  uint32_t width;
  uint32_t height;
};
struct WindowDimensions
{
  int32_t width;
  int32_t height;
};

} // namespace GOOM
