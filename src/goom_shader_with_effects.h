#pragma once

#include "shader_with_effects.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <string>

namespace GOOM
{

class GoomLogger;
struct GoomShaderVariables;

using GLenum = unsigned int;

class GoomShaderWithEffects
{
public:
  GoomShaderWithEffects(const std::string& shaderDir,
                        const std::string& shaderVertexFilename,
                        const std::string& shaderFragmentFilename,
                        glm::mat4 projModelMatrix,
                        GOOM::GoomLogger& goomLogger) noexcept;

  auto CreateGlShaders() -> void;
  [[nodiscard]] auto GetShaderHandle() const noexcept -> GLuint;

  auto SetShaderVariables(const GOOM::GoomShaderVariables& goomShaderVariables) noexcept -> void;

  auto EnableShader() -> void;
  auto UpdateShader() -> void;
  auto DisableShader() -> void;

private:
  std::string m_shaderDir;
  std::string m_shaderVertexFilename;
  std::string m_shaderFragmentFilename;
  GOOM::GoomLogger* m_goomLogger;
  GOOM::ShaderWithEffects m_shaderWithEffects;
  const GOOM::GoomShaderVariables* m_goomShaderVariables{};
  GLuint m_prog = 0;

  static constexpr const auto* DEFAULT_VERTEX_SHADER_FILENAME   = "vertex.glsl";
  static constexpr const auto* DEFAULT_FRAGMENT_SHADER_FILENAME = "fragment.glsl";
  [[nodiscard]] auto GetVertexShaderFilepath() const noexcept -> std::string;
  [[nodiscard]] auto GetFragmentShaderFilepath() const noexcept -> std::string;
  [[nodiscard]] auto CreateGlShader(GLenum shaderType, const std::string& shaderSource) -> GLuint;
  [[nodiscard]] auto GetGlShaderSource(const std::string& shaderFilename) -> std::string;
  auto CheckGlShaderStatus(GLenum shaderHandle) -> void;
};

inline auto GoomShaderWithEffects::GetShaderHandle() const noexcept -> GLuint
{
  return m_prog;
}

inline auto GoomShaderWithEffects::SetShaderVariables(
    const GOOM::GoomShaderVariables& goomShaderVariables) noexcept -> void
{
  m_goomShaderVariables = &goomShaderVariables;
}

inline auto GoomShaderWithEffects::UpdateShader() -> void
{
  m_shaderWithEffects.SetShaderVariables(*m_goomShaderVariables);
}

} // namespace GOOM
