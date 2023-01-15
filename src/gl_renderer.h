#pragma once

#include "gl_render_types.h"
#include "goom/goom_graphic.h"
#include "goom/spimpl.h"

namespace GOOM
{

class GoomLogger;

using GLuint = unsigned int;

class IShaderStrategy
{
public:
  IShaderStrategy()                                          = default;
  IShaderStrategy(const IShaderStrategy&)                    = delete;
  IShaderStrategy(IShaderStrategy&&)                         = delete;
  virtual ~IShaderStrategy()                                 = default;
  auto operator=(const IShaderStrategy&) -> IShaderStrategy& = delete;
  auto operator=(IShaderStrategy&&) -> IShaderStrategy&      = delete;

  virtual auto CreateGlShaders() -> void                                = 0;
  [[nodiscard]] virtual auto GetShaderHandle() const noexcept -> GLuint = 0;

  virtual auto EnableShader() -> void  = 0;
  virtual auto UpdateShader() -> void  = 0;
  virtual auto DisableShader() -> void = 0;
};

class GlRenderer
{
public:
  GlRenderer(const TextureBufferDimensions& textureBufferDimensions,
             const WindowDimensions& windowDimensions,
             IShaderStrategy* shaderStrategy,
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
