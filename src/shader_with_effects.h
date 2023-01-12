#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace GOOM
{

struct GoomShaderVariables;

using GLuint = unsigned int;
using GLint  = int;

class ShaderWithEffects
{
public:
  explicit ShaderWithEffects(glm::mat4 projModelMatrix) noexcept;

  auto SetProgramHandle(GLuint programHandle) noexcept;

  auto InitShaderVariables() noexcept -> void;
  auto InitProjModelMatrix() noexcept -> void;

  auto SetShaderVariables(const GoomShaderVariables& goomShaderVariables) const noexcept -> void;

private:
  GLuint m_prog = 0;
  const glm::mat4 m_projModelMatrix;
  GLint m_uProjModelMatLoc = -1;

  GLint m_uTexExposureLoc                = -1;
  GLint m_uTexBrightnessLoc              = -1;
  GLint m_uTexContrastLoc                = -1;
  GLint m_uTexContrastMinChannelValueLoc = -1;
  GLint m_uTexHueShiftLerpTLoc           = -1;
  GLint m_uTexSrceHueShiftLoc            = -1;
  GLint m_uTexDestHueShiftLoc            = -1;
  GLint m_uTimeLoc                       = -1;
};

inline ShaderWithEffects::ShaderWithEffects(glm::mat4 projModelMatrix) noexcept
  : m_projModelMatrix{projModelMatrix}
{
}

inline auto ShaderWithEffects::SetProgramHandle(GLuint programHandle) noexcept
{
  m_prog = programHandle;
}

} // namespace GOOM
