#pragma once

#include "gl_render_types.h"
#include "goom/goom_graphic.h"
#include "goom/spimpl.h"

namespace GOOM
{

class GoomLogger;
class GoomShaderWithEffects;

class GlRenderer
{
public:
  GlRenderer(const TextureBufferDimensions& textureBufferDimensions,
             const WindowDimensions& windowDimensions,
             GoomShaderWithEffects& glShader,
             GOOM::GoomLogger& goomLogger);

  auto Start() -> void;
  auto Stop() -> void;

  auto SetPixelBuffer(const PixelChannelType* pixelBuffer) noexcept -> void;
  auto Render() -> void;

private:
  class GLRendererImpl;
  spimpl::unique_impl_ptr<GLRendererImpl> m_pimpl;
};

} // namespace GOOM
