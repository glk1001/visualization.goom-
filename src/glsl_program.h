#pragma once

#ifdef WIN32
#pragma warning(disable : 4290)
#endif

#ifdef IS_KODI_BUILD
#include <kodi/gui/gl/GL.h> // NOLINT: False positive??
#else
#include "glad/glad.h"
#endif

#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace GOOM::OPENGL
{

class GlslProgramException : public std::runtime_error
{
public:
  explicit GlslProgramException(const std::string& msg) : std::runtime_error(msg) {}
};

class GlslProgram
{
public:
  GlslProgram();
  GlslProgram(const GlslProgram&) = delete;
  GlslProgram(GlslProgram&&)      = delete;
  ~GlslProgram();
  auto operator=(const GlslProgram&) -> GlslProgram& = delete;
  auto operator=(GlslProgram&&) -> GlslProgram&      = delete;

  auto DeleteProgram() -> void;

  auto CompileShader(const std::string_view& fileName) -> void;
  auto LinkShader() -> void;
  auto ValidateShader() const -> void;

  auto Use() const -> void;
  [[nodiscard]] auto IsInUse() const noexcept -> bool;

  [[nodiscard]] auto GetSubroutineIndex(GLenum shaderType,
                                        const std::string_view& name) const noexcept -> GLuint;

  auto SetUniform(const std::string_view& name, float x, float y, float z) -> void;
  auto SetUniform(const std::string_view& name, const glm::vec2& vec) -> void;
  auto SetUniform(const std::string_view& name, const glm::vec3& vec) -> void;
  auto SetUniform(const std::string_view& name, const glm::vec4& vec) -> void;
  auto SetUniform(const std::string_view& name, const glm::mat4& mat) -> void;
  auto SetUniform(const std::string_view& name, const glm::mat3& mat) -> void;
  auto SetUniform(const std::string_view& name, float val) -> void;
  auto SetUniform(const std::string_view& name, int val) -> void;
  auto SetUniform(const std::string_view& name, bool val) -> void;
  auto SetUniform(const std::string_view& name, GLuint val) -> void;

private:
  GLuint m_handle;
  bool m_linked;

  std::unordered_map<std::string, int> m_uniformLocations;
  auto FindUniformLocations() -> void;
  [[nodiscard]] auto GetUniformLocation(const std::string_view& name) -> GLint;

  auto CompileShader(const std::string_view& fileName, GLenum type) -> void;
  auto CompileShader(const std::string& source, GLenum type, const std::string_view& fileName)
      -> void;
  [[nodiscard]] auto IsLinked() const noexcept -> bool;
  auto DetachAndDeleteShaderObjects() const -> void;

  [[nodiscard]] static auto FileExists(const std::string_view& fileName) -> bool;
  [[nodiscard]] static auto GetExtension(const std::string_view& filename) -> std::string;
};

} // namespace GOOM::OPENGL
