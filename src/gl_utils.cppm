module;

#include "goom_gl.h"

#include <format>
#include <format_23>
#include <string>

export module Goom.GoomVisualization.GlUtils;

import Goom.Lib.GoomUtils;

export namespace GOOM::OPENGL
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

} // namespace GOOM::OPENGL

namespace GOOM::OPENGL
{

using GOOM::ptr_cast;

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto APIENTRY DebugCallback(const GLenum source,
                            const GLenum type,
                            [[maybe_unused]] const GLuint id,
                            const GLenum severity,
                            [[maybe_unused]] const GLsizei length,
                            [[maybe_unused]] const GLchar* const msg,
                            [[maybe_unused]] const void* const param) -> void
{
  std::string sourceStr;
  switch (source)
  {
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      sourceStr = "WindowSys";
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      sourceStr = "App";
      break;
    case GL_DEBUG_SOURCE_API:
      sourceStr = "OpenGL";
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      sourceStr = "ShaderCompiler";
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      sourceStr = "3rdParty";
      break;
    case GL_DEBUG_SOURCE_OTHER:
      sourceStr = "Other";
      break;
    default:
      sourceStr = "Unknown";
  }

  std::string typeStr;
  switch (type)
  {
    case GL_DEBUG_TYPE_ERROR:
      typeStr = "Error";
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      typeStr = "Deprecated";
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      typeStr = "Undefined";
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      typeStr = "Portability";
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      typeStr = "Performance";
      break;
    case GL_DEBUG_TYPE_MARKER:
      typeStr = "Marker";
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      typeStr = "PushGrp";
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      typeStr = "PopGrp";
      break;
    case GL_DEBUG_TYPE_OTHER:
      typeStr = "Other";
      break;
    default:
      typeStr = "Unknown";
  }

  std::string sevStr;
  switch (severity)
  {
    case GL_DEBUG_SEVERITY_HIGH:
      sevStr = "HIGH";
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      sevStr = "MED";
      break;
    case GL_DEBUG_SEVERITY_LOW:
      sevStr = "LOW";
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      sevStr = "NOTIFY";
      break;
    default:
      sevStr = "UNK";
  }

  std_fmt::println("{}:{}[{}]({}): {}", sourceStr, typeStr, sevStr, id, msg);
}

auto GlClearError() -> void
{
  while (glGetError() != GL_NO_ERROR)
  {
  }
}

auto CheckForOpenGLError([[maybe_unused]] const char* const file, [[maybe_unused]] const int line)
    -> bool
{
  auto result  = true;
  auto glError = glGetError();

  while (glError != GL_NO_ERROR)
  {
    std::string message{};
    switch (glError)
    {
      case GL_INVALID_ENUM:
        message = "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument";
        break;
      case GL_INVALID_VALUE:
        message = "GL_INVALID_VALUE: A numeric argument is out of range";
        break;
      case GL_INVALID_OPERATION:
        message = "GL_INVALID_OPERATION: A numeric argument is out of range";
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        message = "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete";
        break;
      case GL_OUT_OF_MEMORY:
        message = "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command";
        break;
      case GL_STACK_UNDERFLOW:
        message = "GL_STACK_UNDERFLOW: An attempt has been made to perform an operation that would"
                  " cause an internal stack to underflow";
        break;
      case GL_STACK_OVERFLOW:
        message = "GL_STACK_OVERFLOW: An attempt has been made to perform an operation that would"
                  " cause an internal stack to overflow";
        break;
      default:
        message = std::format("Unknown error: {}", glError);
    }

    std_fmt::println("OpenGL error: {}. At line {}, in file '{}'.", message, line, file);
    result = false;

    glError = glGetError();
  }

  return result;
}

auto DumpGLInfo(const bool dumpExtensions) -> void
{
  [[maybe_unused]] const auto* const renderer = ptr_cast<const char*>(glGetString(GL_RENDERER));
  [[maybe_unused]] const auto* const vendor   = ptr_cast<const char*>(glGetString(GL_VENDOR));
  [[maybe_unused]] const auto* const version  = ptr_cast<const char*>(glGetString(GL_VERSION));
  [[maybe_unused]] const auto* const glslVersion =
      ptr_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

  auto major = GLint{};
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  auto minor = GLint{};
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  auto samples = GLint{};
  glGetIntegerv(GL_SAMPLES, &samples);
  auto sampleBuffers = GLint{};
  glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);

  std_fmt::println("-------------------------------------------------------------");
  std_fmt::println("GL Vendor    : {}", vendor);
  std_fmt::println("GL Renderer  : {}", renderer);
  std_fmt::println("GL Version   : {}", version);
  std_fmt::println("GL Version   : {}.{}", major, minor);
  std_fmt::println("GLSL Version : {}", glslVersion);
  std_fmt::println("MSAA samples : {}", samples);
  std_fmt::println("MSAA buffers : {}", sampleBuffers);
  std_fmt::println("-------------------------------------------------------------");

  if (dumpExtensions)
  {
    auto nExtensions = GLint{};
    glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
    for (auto i = 0; i < nExtensions; i++)
    {
      //      std_fmt::print("{}\n",
      //                     ptr_cast<const char*>(glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i))));
    }
  }
}

} // namespace GOOM::OPENGL
