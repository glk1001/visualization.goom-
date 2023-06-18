#undef NO_LOGGING

#include "goom_shader_with_effects.h"

#include "goom/goom_logger.h"
#include "goom/goom_utils.h"

#ifdef IS_KODI_BUILD
#include <kodi/gui/gl/GL.h>
#else
#include "glad/glad.h"
#endif

namespace GOOM
{

GoomShaderWithEffects::GoomShaderWithEffects(const std::string& shaderDir,
                                             const std::string& shaderVertexFilename,
                                             const std::string& shaderFragmentFilename,
                                             glm::mat4 projModelMatrix,
                                             GoomLogger& goomLogger) noexcept
  : m_shaderDir{shaderDir},
    m_shaderVertexFilename{shaderVertexFilename},
    m_shaderFragmentFilename{shaderFragmentFilename},
    m_goomLogger{&goomLogger},
    m_shaderWithEffects{projModelMatrix}
{
}

inline auto GoomShaderWithEffects::GetVertexShaderFilepath() const noexcept -> std::string
{
  if (m_shaderVertexFilename.empty())
  {
    return m_shaderDir + "/" + DEFAULT_VERTEX_SHADER_FILENAME;
  }
  return m_shaderDir + "/" + m_shaderVertexFilename;
}

inline auto GoomShaderWithEffects::GetFragmentShaderFilepath() const noexcept -> std::string
{
  if (m_shaderFragmentFilename.empty())
  {
    return m_shaderDir + "/" + DEFAULT_FRAGMENT_SHADER_FILENAME;
  }
  return m_shaderDir + "/" + m_shaderFragmentFilename;
}

auto GoomShaderWithEffects::EnableShader() -> void
{
  glUseProgram(m_prog);
  m_shaderWithEffects.InitProjModelMatrix();
}

auto GoomShaderWithEffects::DisableShader() -> void
{
  glUseProgram(0);
}

auto GoomShaderWithEffects::CreateGlShaders() -> void
{
  const std::string vertexShader   = GetGlShaderSource(GetVertexShaderFilepath());
  const std::string fragmentShader = GetGlShaderSource(GetFragmentShaderFilepath());

  //LogDebug(m_goomLogger, "Doing CreateGlShader GL_VERTEX_SHADER: {}.", vertexShader);
  const GLuint vert = CreateGlShader(GL_VERTEX_SHADER, vertexShader);

  //LogDebug(m_goomLogger, "Doing CreateGlShader GL_FRAGMENT_SHADER: {}.", fragmentShader);
  const GLuint frag = CreateGlShader(GL_FRAGMENT_SHADER, fragmentShader);

  LogDebug(*m_goomLogger, "Start CreateGlShaders glCreateProgram.");
  m_prog = glCreateProgram();

  glAttachShader(m_prog, vert);
  glAttachShader(m_prog, frag);
  glLinkProgram(m_prog);
  glDeleteShader(vert);
  glDeleteShader(frag);
  CheckGlShaderStatus(m_prog);

  m_shaderWithEffects.SetProgramHandle(m_prog);
  m_shaderWithEffects.InitShaderVariables();
}

auto GoomShaderWithEffects::CreateGlShader(const GLenum shaderType, const std::string& shaderSource)
    -> GLuint
{
  const GLuint shaderHandle        = glCreateShader(shaderType);
  const GLchar* const shaderString = shaderSource.c_str();
  glShaderSource(shaderHandle, 1, &shaderString, nullptr);
  glCompileShader(shaderHandle);
  CheckGlShaderStatus(shaderHandle);
  return shaderHandle;
}

auto GoomShaderWithEffects::GetGlShaderSource(const std::string& shaderFilename) -> std::string
{
  static constexpr auto* SHADER_INCLUDE_DIR = "";
  return GetFileContentsWithExpandedIncludes(SHADER_INCLUDE_DIR, shaderFilename);
}

auto GoomShaderWithEffects::CheckGlShaderStatus(const GLenum shaderHandle) -> void
{
  GLboolean bStatus = GL_FALSE;
  glGetBooleanv(GL_SHADER_COMPILER, &bStatus);
  if (GL_FALSE == bStatus)
  {
    LogError(*m_goomLogger, "No shader support.");
  }

  GLint status                      = GL_FALSE;
  static constexpr auto MAX_LOG_LEN = 10U;
  GLint logLen                      = MAX_LOG_LEN;
  if (glIsShader(shaderHandle))
  {
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &status);
  }
  if (glIsProgram(shaderHandle))
  {
    glGetProgramiv(shaderHandle, GL_LINK_STATUS, &status);
  }
  if (GL_TRUE == status)
  {
    return;
  }
  LogError(*m_goomLogger, "glIsProgram returned status = FALSE");
  if (glIsShader(shaderHandle))
  {
    glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &logLen);
  }
  if (glIsProgram(shaderHandle))
  {
    glGetProgramiv(shaderHandle, GL_INFO_LOG_LENGTH, &logLen);
  }
  std::vector<char> log(static_cast<size_t>(logLen), 'E');
  if (glIsShader(shaderHandle))
  {
    glGetShaderInfoLog(shaderHandle, logLen, nullptr, log.data());
  }
  if (glIsProgram(shaderHandle))
  {
    glGetProgramInfoLog(shaderHandle, logLen, nullptr, log.data());
  }
  throw std::logic_error(std::string(log.begin(), log.end()));
}

} // namespace GOOM
