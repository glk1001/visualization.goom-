module;

#include <cstdint>

export module Goom.GoomVisualization:Scene;

import :GlRenderTypes;

export namespace GOOM::OPENGL
{

class IScene
{
public:
  explicit IScene(const TextureBufferDimensions& textureBufferDimensions) noexcept;
  IScene(const IScene&)                    = delete;
  IScene(IScene&&)                         = delete;
  virtual ~IScene()                        = default;
  auto operator=(const IScene&) -> IScene& = delete;
  auto operator=(IScene&&) -> IScene&      = delete;

  [[nodiscard]] virtual auto StopNow() noexcept -> bool { return false; }

  auto GetWidth() const noexcept -> int32_t;
  auto GetHeight() const noexcept -> int32_t;

  auto GetFramebufferWidth() const noexcept -> int32_t;
  auto GetFramebufferHeight() const noexcept -> int32_t;
  auto SetFramebufferDimensions(const WindowDimensions& windowDimensions) noexcept -> void;

  // Load textures, initialize shaders, etc.
  virtual auto InitScene() -> void    = 0;
  virtual auto DestroyScene() -> void = 0;

  // Draw your scene.
  virtual auto Render() -> void = 0;

  // Called when screen is resized.
  virtual auto Resize(const WindowDimensions& windowDimensions) -> void = 0;

private:
  int32_t m_width;
  int32_t m_height;
  int32_t m_framebufferWidth  = m_width;
  int32_t m_framebufferHeight = m_height;
};

} // namespace GOOM::OPENGL

namespace GOOM::OPENGL
{

inline IScene::IScene(const TextureBufferDimensions& textureBufferDimensions) noexcept
  : m_width{static_cast<int32_t>(textureBufferDimensions.width)},
    m_height{static_cast<int32_t>(textureBufferDimensions.height)}
{
}

inline auto IScene::GetWidth() const noexcept -> int32_t
{
  return m_width;
}

inline auto IScene::GetHeight() const noexcept -> int32_t
{
  return m_height;
}

inline auto IScene::GetFramebufferWidth() const noexcept -> int32_t
{
  return m_framebufferWidth;
}

inline auto IScene::GetFramebufferHeight() const noexcept -> int32_t
{
  return m_framebufferHeight;
}

inline auto IScene::SetFramebufferDimensions(const WindowDimensions& windowDimensions) noexcept
    -> void
{
  m_framebufferWidth  = windowDimensions.width;
  m_framebufferHeight = windowDimensions.height;
}

} // namespace GOOM::OPENGL
