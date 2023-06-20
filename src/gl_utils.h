#pragma once

#ifdef IS_KODI_BUILD
#include <kodi/gui/gl/GL.h>
#else
#include "glad/glad.h"
#endif

namespace GOOM::OPENGL
{

auto CheckForOpenGLError(const char* file, int line) -> int;

auto DumpGLInfo(bool dumpExtensions = false) -> void;

auto APIENTRY DebugCallback(GLenum source,
                            GLenum type,
                            GLuint id,
                            GLenum severity,
                            GLsizei length,
                            const GLchar* msg,
                            const void* param) -> void;

} // namespace GOOM::OPENGL
