#undef NO_LOGGING

#include "kodi_shader_with_effects.h"

#include "Main.h"
#include "goom/goom_graphic.h"
#include "goom/goom_logger.h"

#include <filesystem>
#include <format>

namespace GOOM
{

using GOOM::GoomLogger;

KodiShaderWithEffects::KodiShaderWithEffects(CVisualizationGoom& cVisualizationGoom,
                                             const std::string& shaderDir,
                                             glm::mat4 projModelMatrix,
                                             GoomLogger& goomLogger) noexcept
  : m_shaderDir{shaderDir},
    m_shaderWithEffects{projModelMatrix},
    m_cVisualizationGoom{&cVisualizationGoom},
    m_goomLogger{&goomLogger}
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
  m_cVisualizationGoom->EnableShader();
  m_shaderWithEffects.InitProjModelMatrix();
}

auto KodiShaderWithEffects::DisableShader() -> void
{
  m_cVisualizationGoom->DisableShader();
}

auto KodiShaderWithEffects::CreateGlShaders() -> void
{
  const auto vertexShaderFilePath = GetVertexShaderFilepath();
  if (not std::filesystem::exists(vertexShaderFilePath))
  {
    const auto errorMsg = std_fmt::format("CVisualizationGoom: Could not find shader file '{}'.",
                                          vertexShaderFilePath);
    LogError(*m_goomLogger, errorMsg);
    throw std::runtime_error(errorMsg);
  }

  const auto fragmentShaderFilePath = GetFragmentShaderFilepath();
  if (not std::filesystem::exists(fragmentShaderFilePath))
  {
    const auto errorMsg = std_fmt::format("CVisualizationGoom: Could not find shader file '{}'.",
                                          fragmentShaderFilePath);
    LogError(*m_goomLogger, errorMsg);
    throw std::runtime_error(errorMsg);
  }

  if (not m_cVisualizationGoom->LoadShaderFiles(vertexShaderFilePath, fragmentShaderFilePath))
  {
    const auto* const errorMsg = "CVisualizationGoom: Failed to load GL shaders.";
    LogError(*m_goomLogger, errorMsg);
    throw std::runtime_error(errorMsg);
  }
  if (not m_cVisualizationGoom->CompileAndLink())
  {
    const auto* const errorMsg = "CVisualizationGoom: Failed to compile GL shaders.";
    LogError(*m_goomLogger, errorMsg);
    throw std::runtime_error(errorMsg);
  }

  m_prog = m_cVisualizationGoom->ProgramHandle();

  m_shaderWithEffects.SetProgramHandle(m_prog);
  m_shaderWithEffects.InitShaderVariables();
}

} // namespace GOOM
