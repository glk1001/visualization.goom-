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
struct GoomShaderVariables;
}

namespace GOOM
{

using GLenum = unsigned int;


class KodiShaderWithEffects : public IShaderStrategy
{
public:
  KodiShaderWithEffects(CVisualizationGoom& cVisualizationGoom,
                        const std::string& shaderDir,
                        glm::mat4 projModelMatrix) noexcept;

  auto CreateGlShaders() -> void override;
  [[nodiscard]] auto GetShaderHandle() const noexcept -> GLuint override;

  auto SetShaderVariables(const GoomShaderVariables& goomShaderVariables) noexcept -> void;

  auto EnableShader() -> void override;
  auto UpdateShader() -> void override;
  auto DisableShader() -> void override;

private:
  std::string m_shaderDir;
  ShaderWithEffects m_shaderWithEffects;
  GLuint m_prog = 0;
  CVisualizationGoom& m_cVisualizationGoom;
  const GoomShaderVariables* m_goomShaderVariables{};

  static constexpr const auto* VERTEX_SHADER_FILENAME   = "vertex.glsl";
  static constexpr const auto* FRAGMENT_SHADER_FILENAME = "fragment.glsl";
  [[nodiscard]] auto GetVertexShaderFilepath() const noexcept -> std::string;
  [[nodiscard]] auto GetFragmentShaderFilepath() const noexcept -> std::string;
};

inline auto KodiShaderWithEffects::GetShaderHandle() const noexcept -> GLuint
{
  return m_prog;
}

inline auto KodiShaderWithEffects::SetShaderVariables(
    const GoomShaderVariables& goomShaderVariables) noexcept -> void
{
  m_goomShaderVariables = &goomShaderVariables;
}

inline auto KodiShaderWithEffects::UpdateShader() -> void
{
  m_shaderWithEffects.SetShaderVariables(*m_goomShaderVariables);
}

} // namespace GOOM
