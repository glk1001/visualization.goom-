#include "glsl_program.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace GOOM::OPENGL
{

namespace
{

// NOLINTNEXTLINE(cert-err58-cpp)
const std::unordered_map<std::string, int32_t> EXTENSIONS = {
    {       ".vs",          GL_VERTEX_SHADER},
    {     ".vert",          GL_VERTEX_SHADER},
    {"_vert.glsl",          GL_VERTEX_SHADER},
    {".vert.glsl",          GL_VERTEX_SHADER},
    {       ".gs",        GL_GEOMETRY_SHADER},
    {     ".geom",        GL_GEOMETRY_SHADER},
    {".geom.glsl",        GL_GEOMETRY_SHADER},
    {      ".tcs",    GL_TESS_CONTROL_SHADER},
    { ".tcs.glsl",    GL_TESS_CONTROL_SHADER},
    {      ".tes", GL_TESS_EVALUATION_SHADER},
    { ".tes.glsl", GL_TESS_EVALUATION_SHADER},
    {       ".fs",        GL_FRAGMENT_SHADER},
    {     ".frag",        GL_FRAGMENT_SHADER},
    {"_frag.glsl",        GL_FRAGMENT_SHADER},
    {".frag.glsl",        GL_FRAGMENT_SHADER},
    {       ".cs",         GL_COMPUTE_SHADER},
    {  ".cs.glsl",         GL_COMPUTE_SHADER}
};

} // namespace

GlslProgram::GlslProgram() : m_handle(0), m_linked(false)
{
}

GlslProgram::~GlslProgram()
{
  DeleteProgram();
}

auto GlslProgram::DeleteProgram() -> void
{
  if (m_handle == 0)
  {
    return;
  }
  DetachAndDeleteShaderObjects();
  glDeleteProgram(m_handle);
  m_handle = 0;
}

auto GlslProgram::DetachAndDeleteShaderObjects() const -> void
{
  // Detach and delete the shader objects (if they are not already removed).
  auto numShaders = GLint{};
  glGetProgramiv(m_handle, GL_ATTACHED_SHADERS, &numShaders);

  auto shaderNames = std::vector<GLuint>(static_cast<GLuint>(numShaders));
  glGetAttachedShaders(m_handle, numShaders, nullptr, shaderNames.data());

  for (auto shader : shaderNames)
  {
    glDetachShader(m_handle, shader);
    glDeleteShader(shader);
  }
}

auto GlslProgram::CompileShader(const std::string_view& fileName) -> void
{

  // Check the file name's extension to determine the shader type
  auto ext  = GetExtension(fileName);
  auto type = GL_VERTEX_SHADER;
  auto it   = EXTENSIONS.find(ext);
  if (it != EXTENSIONS.end())
  {
    type = it->second;
  }
  else
  {
    throw GlslProgramException("Unrecognized extension: " + ext);
  }

  // Pass the discovered shader type along
  CompileShader(fileName, static_cast<GLenum>(type));
}

auto GlslProgram::GetExtension(const std::string_view& filename) -> std::string
{
  const auto nameStr = std::string{filename};

  const auto dotLoc = nameStr.find_last_of('.');
  if (dotLoc != std::string::npos)
  {
    auto ext = nameStr.substr(dotLoc);
    if (ext == ".glsl")
    {
      auto loc = nameStr.find_last_of('.', dotLoc - 1);
      if (loc == std::string::npos)
      {
        loc = nameStr.find_last_of('_', dotLoc - 1);
      }
      if (loc != std::string::npos)
      {
        return nameStr.substr(loc);
      }
    }
    else
    {
      return ext;
    }
  }
  return "";
}

auto GlslProgram::CompileShader(const std::string_view& fileName, const GLenum type) -> void
{
  if (not FileExists(fileName))
  {
    throw GlslProgramException(std::string{"Shader: "} + fileName.data() + " not found.");
  }

  if (m_handle <= 0)
  {
    m_handle = glCreateProgram();
    if (m_handle == 0)
    {
      throw GlslProgramException("Unable to create shader program.");
    }
  }

  auto inFile = std::ifstream{fileName.data(), std::ios::in};
  if (not inFile)
  {
    throw GlslProgramException(std::string{"Unable to open: "} + fileName.data());
  }

  // Get file contents
  std::stringstream code;
  code << inFile.rdbuf();
  inFile.close();

  CompileShader(code.str(), type, fileName);
}

auto GlslProgram::CompileShader(const std::string& source,
                                const GLenum type,
                                const std::string_view& fileName) -> void
{
  if (m_handle <= 0)
  {
    m_handle = glCreateProgram();
    if (m_handle == 0)
    {
      throw GlslProgramException("Unable to create shader program.");
    }
  }

  const auto shaderHandle = glCreateShader(type);
  const auto* cCode       = source.c_str();
  glShaderSource(shaderHandle, 1, &cCode, nullptr);
  glCompileShader(shaderHandle);

  // Check for errors
  auto result = int{};
  glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result);
  if (GL_FALSE == result)
  {
    // Compile failed, get log
    auto msg = std::string{};
    if (not fileName.empty())
    {
      msg = std::string(fileName) + ": shader compilation failed\n";
    }
    else
    {
      msg = "Shader compilation failed.\n";
    }

    auto length = 0;
    glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length);
    if (length > 0)
    {
      auto log     = std::string(static_cast<size_t>(length), ' ');
      auto written = 0;
      glGetShaderInfoLog(shaderHandle, length, &written, log.data());
      msg += log;
    }
    throw GlslProgramException(msg);
  }

  // Compile succeeded, attach shader
  glAttachShader(m_handle, shaderHandle);
}

auto GlslProgram::LinkShader() -> void
{
  if (m_linked)
  {
    return;
  }
  if (m_handle <= 0)
  {
    throw GlslProgramException("Program has not been compiled.");
  }

  glLinkProgram(m_handle);

  auto status = 0;
  glGetProgramiv(m_handle, GL_LINK_STATUS, &status);

  auto errString = std::string{};
  if (GL_FALSE == status)
  {
    // Store log and return false
    auto length = 0;
    glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &length);
    errString += "Program LinkShader failed:\n";
    if (length > 0)
    {
      auto log     = std::string(static_cast<size_t>(length), ' ');
      auto written = 0;
      glGetProgramInfoLog(m_handle, length, &written, log.data());
      errString += log;
    }
  }
  else
  {
    FindUniformLocations();
    m_linked = true;
  }

  DetachAndDeleteShaderObjects();

  if (GL_FALSE == status)
  {
    throw GlslProgramException(errString);
  }
}

auto GlslProgram::FindUniformLocations() -> void
{
  m_uniformLocations.clear();

  auto numUniforms = GLint{0};
#ifdef __APPLE__
  // For OpenGL 4.1, use glGetActiveUniform
  GLint maxLen;
  GLchar* name;

  glGetProgramiv(handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
  glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &numUniforms);

  name = new GLchar[maxLen];
  for (GLuint i = 0; i < numUniforms; ++i)
  {
    GLint size;
    GLenum type;
    GLsizei written;
    glGetActiveUniform(handle, i, maxLen, &written, &size, &type, name);
    GLint location         = glGetUniformLocation(handle, name);
    uniformLocations[name] = glGetUniformLocation(handle, name);
  }
  delete[] name;
#else
  // For OpenGL 4.3 and above, use glGetProgramResource
  glGetProgramInterfaceiv(m_handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

  static constexpr auto NUM_PROPERTIES = 4;
  static constexpr auto PROPERTIES =
      std::array<GLenum, NUM_PROPERTIES>{GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX};

  for (GLint i = 0; i < numUniforms; ++i)
  {
    static constexpr auto NUM_RESULTS = 4;
    auto results                      = std::array<GLint, NUM_RESULTS>{};
    glGetProgramResourceiv(m_handle,
                           GL_UNIFORM,
                           static_cast<GLuint>(i),
                           NUM_PROPERTIES,
                           PROPERTIES.data(),
                           NUM_RESULTS,
                           nullptr,
                           results.data());

    if (results[3] != -1)
    {
      continue; // Skip uniforms in blocks
    }
    auto nameBufSize = results[0] + 1;
    auto name        = std::vector<char>(static_cast<size_t>(nameBufSize));
    glGetProgramResourceName(
        m_handle, GL_UNIFORM, static_cast<GLuint>(i), nameBufSize, nullptr, name.data());
    m_uniformLocations[name.data()] = results[2];
  }
#endif
}

auto GlslProgram::Use() const -> void
{
  if ((m_handle <= 0) or (not m_linked))
  {
    throw GlslProgramException("Shader has not been linked.");
  }
  glUseProgram(m_handle);
}

auto GlslProgram::IsInUse() const noexcept -> bool
{
  GLint currentProgram{};
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
  return m_handle == static_cast<GLuint>(currentProgram);
}

auto GlslProgram::IsLinked() const noexcept -> bool
{
  return m_linked;
}

auto GlslProgram::SetUniform(const std::string_view& name,
                             const float x,
                             const float y,
                             const float z) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform3f(loc, x, y, z);
}

auto GlslProgram::SetUniform(const std::string_view& name, const glm::vec3& vec) -> void
{
  SetUniform(name, vec.x, vec.y, vec.z); // NOLINT: union hard to fix here
}

auto GlslProgram::SetUniform(const std::string_view& name, const glm::vec4& vec) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform4f(loc, vec.x, vec.y, vec.z, vec.w); // NOLINT: union hard to fix here
}

auto GlslProgram::SetUniform(const std::string_view& name, const glm::vec2& vec) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform2f(loc, vec.x, vec.y); // NOLINT: union hard to fix here
}

auto GlslProgram::SetUniform(const std::string_view& name, const glm::mat4& mat) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}

auto GlslProgram::SetUniform(const std::string_view& name, const glm::mat3& mat) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniformMatrix3fv(loc, 1, GL_FALSE, &mat[0][0]);
}

auto GlslProgram::SetUniform(const std::string_view& name, const float val) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform1f(loc, val);
}

auto GlslProgram::SetUniform(const std::string_view& name, const std::vector<float>& vals) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform1fv(loc, static_cast<int32_t>(vals.size()), vals.data());
}

auto GlslProgram::SetUniform(const std::string_view& name, const int val) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform1i(loc, val);
}

auto GlslProgram::SetUniform(const std::string_view& name, const GLuint val) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform1ui(loc, val);
}

auto GlslProgram::SetUniform(const std::string_view& name, const bool val) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform1i(loc, static_cast<GLint>(val));
}

auto GlslProgram::ValidateShader() const -> void
{
  if (not IsLinked())
  {
    throw GlslProgramException("Program is not linked");
  }

  auto status = GLint{};
  glValidateProgram(m_handle);
  glGetProgramiv(m_handle, GL_VALIDATE_STATUS, &status);

  if (GL_FALSE == status)
  {
    // Store log and return false
    auto length    = 0;
    auto logString = std::string{};

    glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &length);

    if (length > 0)
    {
      auto cLog    = std::vector<char>(static_cast<size_t>(length));
      auto written = 0;
      glGetProgramInfoLog(m_handle, length, &written, cLog.data());
      logString = cLog.data();
    }

    throw GlslProgramException(std::string{"Program failed to ValidateShader\n"} + logString);
  }
}

auto GlslProgram::FileExists(const std::string_view& fileName) -> bool
{
  return std::filesystem::exists(fileName);
}

auto GlslProgram::GetSubroutineIndex(const GLenum shaderType,
                                     const std::string_view& name) const noexcept -> GLuint
{
  return glGetSubroutineIndex(m_handle, shaderType, name.data());
}

auto GlslProgram::GetUniformLocation(const std::string_view& name) -> GLint
{
  auto pos = m_uniformLocations.find(name.data());

  if (pos == m_uniformLocations.end())
  {
    auto loc                        = glGetUniformLocation(m_handle, name.data());
    m_uniformLocations[name.data()] = loc;
    return loc;
  }

  return pos->second;
}

} // namespace GOOM::OPENGL
