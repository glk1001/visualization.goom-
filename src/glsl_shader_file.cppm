module;

#include <format>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

export module Goom.GoomVisualization:GlslShaderFile;

import Goom.Lib.GoomUtils;

export namespace GOOM::OPENGL
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

namespace GOOM::OPENGL
{

GlslShaderFile::GlslShaderFile(const std::string& shaderFilepath,
                               const ShaderMacros& shaderMacros,
                               const std::string& includeDirs) noexcept
  : m_shaderFilepath{shaderFilepath}, m_shaderMacros{shaderMacros}, m_includeDirs{includeDirs}
{
}

auto GlslShaderFile::WriteToFile(const std::string& outputFilepath) const -> void
{
  const auto lines = GetMacroExpandedLines();

  auto outStream = std::ofstream{outputFilepath};
  if (not outStream)
  {
    throw std::runtime_error{std::format("Could not open output file \"{}\"", outputFilepath)};
  }

  PutFileLines(outStream, lines);
}

auto GlslShaderFile::GetMacroExpandedLines() const noexcept -> std::vector<std::string>
{
  auto lines = GetFileWithExpandedIncludes(m_includeDirs, m_shaderFilepath);

  ExpandMacros(lines);

  return lines;
}

auto GlslShaderFile::ExpandMacros(std::vector<std::string>& lines) const noexcept -> void
{
  if (m_shaderMacros.empty())
  {
    return;
  }

  for (auto& line : lines)
  {
    ExpandMacros(line);
  }
}

auto GlslShaderFile::ExpandMacros(std::string& line) const noexcept -> void
{
  for (const auto& macro : m_shaderMacros)
  {
    static constexpr auto* MACRO_START = "$";
    if (line.find(MACRO_START) == std::string::npos)
    {
      return;
    }
    const auto macroName = MACRO_START + macro.first;
    FindAndReplaceAll(line, macroName, macro.second);
  }
}

} // namespace GOOM::OPENGL
