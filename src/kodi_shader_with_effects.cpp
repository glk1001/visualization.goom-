#undef NO_LOGGING

#include "kodi_shader_with_effects.h"

#include "Main.h"
#include "goom/goom_graphic.h"
#include "goom/goom_logger.h"

namespace GOOM
{

KodiShaderWithEffects::KodiShaderWithEffects(CVisualizationGoom& cVisualizationGoom,
                                             const std::string& shaderDir,
                                             glm::mat4 projModelMatrix) noexcept
  : m_shaderDir{shaderDir},
    m_shaderWithEffects{projModelMatrix},
    m_cVisualizationGoom{cVisualizationGoom}
{
}

inline auto KodiShaderWithEffects::GetVertexShaderFilepath() const noexcept -> std::string
{
  return m_shaderDir + "/" + VERTEX_SHADER_FILENAME;
}

inline auto KodiShaderWithEffects::GetFragmentShaderFilepath() const noexcept -> std::string
{
  return m_shaderDir + "/" + FRAGMENT_SHADER_FILENAME;
}

auto KodiShaderWithEffects::EnableShader() -> void
{
  m_cVisualizationGoom.EnableShader();
  m_shaderWithEffects.InitProjModelMatrix();
}

auto KodiShaderWithEffects::DisableShader() -> void
{
  m_cVisualizationGoom.DisableShader();
}

auto KodiShaderWithEffects::CreateGlShaders() -> void
{
  const std::string vertexShaderFilePath   = GetVertexShaderFilepath();
  const std::string fragmentShaderFilePath = GetFragmentShaderFilepath();

  if (not m_cVisualizationGoom.LoadShaderFiles(vertexShaderFilePath, fragmentShaderFilePath))
  {
    throw std::runtime_error("CVisualizationGoom: Failed to load GL shaders.");
  }
  if (not m_cVisualizationGoom.CompileAndLink())
  {
    throw std::runtime_error("CVisualizationGoom: Failed to compile GL shaders.");
  }

  m_prog = m_cVisualizationGoom.ProgramHandle();

  m_shaderWithEffects.SetProgramHandle(m_prog);
  m_shaderWithEffects.InitShaderVariables();
}

} // namespace GOOM
