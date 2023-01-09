#pragma once

namespace GOOM
{

using GLuint = unsigned int;

class ShaderStrategy
{
public:
  ShaderStrategy()                                         = default;
  ShaderStrategy(const ShaderStrategy&)                    = delete;
  ShaderStrategy(ShaderStrategy&&)                         = delete;
  virtual ~ShaderStrategy()                                = default;
  auto operator=(const ShaderStrategy&) -> ShaderStrategy& = delete;
  auto operator=(ShaderStrategy&&) -> ShaderStrategy&      = delete;

  [[nodiscard]] virtual auto GetProgramHandle() const noexcept -> GLuint = 0;

  virtual auto CreateGlShaders() -> void = 0;
  virtual auto EnableShader() -> void    = 0;
  virtual auto DisableShader() -> void   = 0;
};

} // namespace GOOM
