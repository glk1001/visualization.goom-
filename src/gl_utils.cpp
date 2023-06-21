#include "gl_utils.h"

#include <format>
#include <string>

namespace GOOM::OPENGL
{

auto APIENTRY DebugCallback(const GLenum source,
                            const GLenum type,
                            const GLuint id,
                            const GLenum severity,
                            [[maybe_unused]] const GLsizei length,
                            const GLchar* const msg,
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

auto CheckForOpenGLError(const char* const file, const int line) -> int
{
  //
  // Returns 1 if an OpenGL error occurred, 0 otherwise.
  //
  auto retCode = 0;
  auto glErr   = glGetError();

  while (glErr != GL_NO_ERROR)
  {
    const char* message;
    switch (glErr)
    {
      case GL_INVALID_ENUM:
        message = "Invalid enum";
        break;
      case GL_INVALID_VALUE:
        message = "Invalid value";
        break;
      case GL_INVALID_OPERATION:
        message = "Invalid operation";
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        message = "Invalid framebuffer operation";
        break;
      case GL_OUT_OF_MEMORY:
        message = "Out of memory";
        break;
      default:
        message = "Unknown error";
    }

    std_fmt::println("glError in file '{}' line {}: {}", file, line, message);
    retCode = 1;

    glErr = glGetError();
  }

  return retCode;
}

auto DumpGLInfo(const bool dumpExtensions) -> void
{
  const auto* const renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
  const auto* const vendor   = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
  const auto* const version  = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  const auto* const glslVersion =
      reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

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
      std_fmt::print(
          "{}\n",
          reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i))));
    }
  }
}

} // namespace GOOM::OPENGL
