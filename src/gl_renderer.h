#pragma once

#include "gl_render_types.h"
#include "goom/goom_graphic.h"
#include "goom/spimpl.h"

#include <string>

namespace GOOM
{

class GoomLogger;

class GlRenderer
{
public:
  GlRenderer(const std::string& shaderDir,
             const TextureBufferDimensions& textureBufferDimensions,
             const WindowDimensions& windowDimensions,
             GOOM::GoomLogger& goomLogger);

  auto Init() -> void;
  auto Destroy() -> void;

  auto SetPixelBuffer(const PixelChannelType* pixelBuffer) noexcept -> void;
  auto SetShaderVariables(const GOOM::GoomShaderVariables& goomShaderVariables) noexcept -> void;

  auto Render() -> void;

private:
  class GLRendererImpl;
  spimpl::unique_impl_ptr<GLRendererImpl> m_pimpl;
};

} // namespace GOOM
