#include "shader_with_effects.h"

#include "goom/goom_graphic.h"

#include <glm/ext.hpp>
#include <kodi/gui/gl/GL.h>

namespace GOOM
{

auto ShaderWithEffects::InitProjModelMatrix() noexcept -> void
{
  glUniformMatrix4fv(m_uProjModelMatLoc, 1, GL_FALSE, glm::value_ptr(m_projModelMatrix));
}

auto ShaderWithEffects::InitShaderVariables() noexcept -> void
{
  m_uProjModelMatLoc               = glGetUniformLocation(m_prog, "u_projModelMat");
  m_uTexExposureLoc                = glGetUniformLocation(m_prog, "u_texExposure");
  m_uTexBrightnessLoc              = glGetUniformLocation(m_prog, "u_texBrightness");
  m_uTexContrastLoc                = glGetUniformLocation(m_prog, "u_texContrast");
  m_uTexContrastMinChannelValueLoc = glGetUniformLocation(m_prog, "u_texContrastMinChan");
  m_uTexHueShiftLerpTLoc           = glGetUniformLocation(m_prog, "u_texHueShiftLerpT");
  m_uTexSrceHueShiftLoc            = glGetUniformLocation(m_prog, "u_texSrceHueShift");
  m_uTexDestHueShiftLoc            = glGetUniformLocation(m_prog, "u_texDestHueShift");
  m_uTimeLoc                       = glGetUniformLocation(m_prog, "u_time");
}

auto ShaderWithEffects::SetShaderEffects(const GoomShaderEffects& goomShaderEffects) const noexcept
    -> void
{
  if (goomShaderEffects.exposure > 0.0F)
  {
    glUniform1f(m_uTexExposureLoc, goomShaderEffects.exposure);
  }
  if (goomShaderEffects.brightness >= 0.0F)
  {
    glUniform1f(m_uTexBrightnessLoc, goomShaderEffects.brightness);
  }
  if (goomShaderEffects.contrast > 0.0F)
  {
    glUniform1f(m_uTexContrastLoc, goomShaderEffects.contrast);
    glUniform1f(m_uTexContrastMinChannelValueLoc, goomShaderEffects.contrastMinChannelValue);
  }

  glUniform1f(m_uTexHueShiftLerpTLoc, goomShaderEffects.hueShiftLerpT);
  glUniform1f(m_uTexSrceHueShiftLoc, goomShaderEffects.srceHueShift);
  glUniform1f(m_uTexDestHueShiftLoc, goomShaderEffects.destHueShift);

  static GLint s_time = 0;
  ++s_time;
  glUniform1i(m_uTimeLoc, s_time);
}

} // namespace GOOM
