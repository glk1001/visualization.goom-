#undef NO_LOGGING

#include "kodi_shader_with_effects.h"

#include "Main.h"
#include "goom/goom_logger.h"
#include "goom/goom_utils.h"

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
  const auto vertexShaderFilepath = GetVertexShaderFilepath();
  if (not std::filesystem::exists(vertexShaderFilepath))
  {
    const auto errorMsg = std_fmt::format("CVisualizationGoom: Could not find shader file '{}'.",
                                          vertexShaderFilepath);
    LogError(*m_goomLogger, errorMsg);
    throw std::runtime_error(errorMsg);
  }

  const auto fragmentShaderFilepath = GetFragmentShaderFilepath();
  if (not std::filesystem::exists(fragmentShaderFilepath))
  {
    const auto errorMsg = std_fmt::format("CVisualizationGoom: Could not find shader file '{}'.",
                                          fragmentShaderFilepath);
    LogError(*m_goomLogger, errorMsg);
    throw std::runtime_error(errorMsg);
  }

  static constexpr auto* SHADER_INCLUDE_DIR = "";

  const auto vertexShaderStr =
      GetFileContentsWithExpandedIncludes(SHADER_INCLUDE_DIR, vertexShaderFilepath);
  const auto fragmentShaderStr =
      GetFileContentsWithExpandedIncludes(SHADER_INCLUDE_DIR, fragmentShaderFilepath);

  if (static constexpr auto* EXTRA_END = ""; not m_cVisualizationGoom->CompileAndLink(
          vertexShaderStr, EXTRA_END, fragmentShaderStr, EXTRA_END))
  {
    static constexpr auto* ERROR_MSG = "CVisualizationGoom: Failed to compile GL shaders.";
    LogError(*m_goomLogger, ERROR_MSG);
    throw std::runtime_error(ERROR_MSG);
  }

  m_prog = m_cVisualizationGoom->ProgramHandle();

  m_shaderWithEffects.SetProgramHandle(m_prog);
  m_shaderWithEffects.InitShaderVariables();
}

} // namespace GOOM
