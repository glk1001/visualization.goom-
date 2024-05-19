module;

#include <cstdint>

export module Goom.GoomVisualization:GlRenderTypes;

export namespace GOOM
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
