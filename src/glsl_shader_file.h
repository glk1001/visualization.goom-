#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace GOOM::OPENGL
{

class GlslShaderFile
{
public:
  using ShaderMacros = std::unordered_map<std::string, std::string>;

  GlslShaderFile(const std::string& shaderFilepath,
                 const ShaderMacros& shaderMacros,
                 const std::string& includeDirs) noexcept;

  auto WriteToFile(const std::string& outputFilepath) const -> void;

private:
  std::string m_shaderFilepath;
  ShaderMacros m_shaderMacros;
  std::string m_includeDirs;
  [[nodiscard]] auto GetMacroExpandedLines() const noexcept -> std::vector<std::string>;
  auto ExpandMacros(std::vector<std::string>& lines) const noexcept -> void;
  auto ExpandMacros(std::string& line) const noexcept -> void;
};

} // namespace GOOM::OPENGL
