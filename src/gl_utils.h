#pragma once

#ifdef IS_KODI_BUILD
#include <kodi/gui/gl/GL.h> // NOLINT: Too tricky with Kodi GL.
#else
#include "glad/glad.h"
#endif

namespace GOOM::OPENGL
{

auto CheckForOpenGLError(const char* file, int line) -> bool;

auto DumpGLInfo(bool dumpExtensions = false) -> void;

auto APIENTRY DebugCallback(GLenum source,
                            GLenum type,
                            GLuint id,
                            GLenum severity,
                            GLsizei length,
                            const GLchar* msg,
                            const void* param) -> void;

auto GlClearError() -> void;

// NOLINTNEXTLINE: Happy with this case for my GL macro.
#define GlCall(x) \
  GlClearError(); \
  x; \
  assert(CheckForOpenGLError(__FILE__, __LINE__))

} // namespace GOOM::OPENGL
