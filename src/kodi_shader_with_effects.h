#pragma once

#include "gl_renderer.h"
#include "shader_with_effects.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <string>

class CVisualizationGoom;

namespace GOOM
{
class GoomLogger;
struct GoomShaderEffects;
}

namespace GOOM
{

using GLenum = unsigned int;


class KodiShaderWithEffects : public ShaderStrategy
{
public:
  KodiShaderWithEffects(CVisualizationGoom& cVisualizationGoom,
                        const std::string& shaderDir,
                        glm::mat4 projModelMatrix) noexcept;

  [[nodiscard]] auto GetProgramHandle() const noexcept -> GLuint override;

  auto CreateGlShaders() -> void override;
  auto EnableShader() -> void override;
  auto DisableShader() -> void override;

  auto SetShaderEffects(const GoomShaderEffects& goomShaderEffects) const noexcept -> void;

private:
  std::string m_shaderDir;
  ShaderWithEffects m_shaderWithEffects;
  GLuint m_prog = 0;
  CVisualizationGoom& m_cVisualizationGoom;

  static constexpr const auto* VERTEX_SHADER_FILENAME   = "vertex.glsl";
  static constexpr const auto* FRAGMENT_SHADER_FILENAME = "fragment.glsl";
  [[nodiscard]] auto GetVertexShaderFilepath() const noexcept -> std::string;
  [[nodiscard]] auto GetFragmentShaderFilepath() const noexcept -> std::string;
};

inline auto KodiShaderWithEffects::SetShaderEffects(
    const GoomShaderEffects& goomShaderEffects) const noexcept -> void
{
  m_shaderWithEffects.SetShaderEffects(goomShaderEffects);
}

inline auto KodiShaderWithEffects::GetProgramHandle() const noexcept -> GLuint
{
  return m_prog;
}

} // namespace GOOM
