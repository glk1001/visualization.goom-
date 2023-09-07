#include "gl_utils.h"

#include "goom/goom_utils.h"

#include <format> // NOLINT: Waiting to use C++20.
#include <string>

namespace GOOM::OPENGL
{

using GOOM::ptr_cast;

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
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

  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  std_fmt::println("{}:{}[{}]({}): {}", sourceStr, typeStr, sevStr, id, msg);
}

auto GlClearError() -> void
{
  while (glGetError() != GL_NO_ERROR)
  {
  }
}

auto CheckForOpenGLError(const char* const file, const int line) -> bool
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
        // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
        message = std_fmt::format("Unknown error: {}", glError);
    }

    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
    std_fmt::println("OpenGL error: {}. At line {}, in file '{}'.", message, line, file);
    result = false;

    glError = glGetError();
  }

  return result;
}

auto DumpGLInfo(const bool dumpExtensions) -> void
{
  const auto* const renderer    = ptr_cast<const char*>(glGetString(GL_RENDERER));
  const auto* const vendor      = ptr_cast<const char*>(glGetString(GL_VENDOR));
  const auto* const version     = ptr_cast<const char*>(glGetString(GL_VERSION));
  const auto* const glslVersion = ptr_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

  auto major = GLint{};
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  auto minor = GLint{};
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  auto samples = GLint{};
  glGetIntegerv(GL_SAMPLES, &samples);
  auto sampleBuffers = GLint{};
  glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);

  // NOLINTBEGIN(misc-include-cleaner): Waiting for C++20.
  std_fmt::println("-------------------------------------------------------------");
  std_fmt::println("GL Vendor    : {}", vendor);
  std_fmt::println("GL Renderer  : {}", renderer);
  std_fmt::println("GL Version   : {}", version);
  std_fmt::println("GL Version   : {}.{}", major, minor);
  std_fmt::println("GLSL Version : {}", glslVersion);
  std_fmt::println("MSAA samples : {}", samples);
  std_fmt::println("MSAA buffers : {}", sampleBuffers);
  std_fmt::println("-------------------------------------------------------------");
  // NOLINTEND(misc-include-cleaner)

  if (dumpExtensions)
  {
    auto nExtensions = GLint{};
    glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
    for (auto i = 0; i < nExtensions; i++)
    {
      // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
      std_fmt::print("{}\n",
                     ptr_cast<const char*>(glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i))));
    }
  }
}

} // namespace GOOM::OPENGL
