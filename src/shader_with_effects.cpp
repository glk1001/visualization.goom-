#if defined(__apple_build_version__) || __clang_major__ == 14
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include "shader_with_effects.h"

#include "goom/goom_graphic.h"

#include <glm/ext.hpp>

#ifdef IS_KODI_BUILD
#include <kodi/gui/gl/GL.h>
#else
#include "glad/glad.h"
#endif

namespace GOOM
{

auto ShaderWithEffects::InitProjModelMatrix() noexcept -> void
{
  glUniformMatrix4fv(m_uProjModelMatLoc, 1, GL_FALSE, glm::value_ptr(m_projModelMatrix));
}

auto ShaderWithEffects::InitShaderVariables() noexcept -> void
{
  m_uProjModelMatLoc               = ::glGetUniformLocation(m_prog, "u_projModelMat");
  m_uTexExposureLoc                = ::glGetUniformLocation(m_prog, "u_texExposure");
  m_uTexBrightnessLoc              = ::glGetUniformLocation(m_prog, "u_texBrightness");
  m_uTexContrastLoc                = ::glGetUniformLocation(m_prog, "u_texContrast");
  m_uTexContrastMinChannelValueLoc = ::glGetUniformLocation(m_prog, "u_texContrastMinChan");
  m_uTexHueShiftLerpTLoc           = ::glGetUniformLocation(m_prog, "u_texHueShiftLerpT");
  m_uTexSrceHueShiftLoc            = ::glGetUniformLocation(m_prog, "u_texSrceHueShift");
  m_uTexDestHueShiftLoc            = ::glGetUniformLocation(m_prog, "u_texDestHueShift");
  m_uTimeLoc                       = ::glGetUniformLocation(m_prog, "u_time");
}

auto ShaderWithEffects::SetShaderVariables(
    const GoomShaderVariables& goomShaderVariables) const noexcept -> void
{
  if (goomShaderVariables.exposure > 0.0F)
  {
    ::glUniform1f(m_uTexExposureLoc, goomShaderVariables.exposure);
  }
  if (goomShaderVariables.brightness >= 0.0F)
  {
    ::glUniform1f(m_uTexBrightnessLoc, goomShaderVariables.brightness);
  }
  if (goomShaderVariables.contrast > 0.0F)
  {
    ::glUniform1f(m_uTexContrastLoc, goomShaderVariables.contrast);
    ::glUniform1f(m_uTexContrastMinChannelValueLoc, goomShaderVariables.contrastMinChannelValue);
  }

  ::glUniform1f(m_uTexHueShiftLerpTLoc, goomShaderVariables.hueShiftLerpT);
  ::glUniform1f(m_uTexSrceHueShiftLoc, goomShaderVariables.srceHueShift);
  ::glUniform1f(m_uTexDestHueShiftLoc, goomShaderVariables.destHueShift);

  static GLint s_time = 0;
  ++s_time;
  ::glUniform1i(m_uTimeLoc, s_time);
}

} // namespace GOOM

#if defined(__apple_build_version__) || __clang_major__ == 14
#pragma clang diagnostic pop
#endif
