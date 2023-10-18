#pragma once

#include "goom/goom_config.h"
#include "goom/goom_utils.h"

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format> // NOLINT: Waiting to use C++20.
#include <fstream>
#include <functional>
#include <ios>
#include <istream>
#include <ostream>
#include <span> // NOLINT: Waiting to use C++20.
#include <stdexcept>
#include <string>
#include <type_traits>

namespace GOOM::UTILS
{

template<class T, class HeaderT = std::nullptr_t>
class BufferSaver
{
public:
  struct BufferLimits
  {
    int64_t startBuffNum{};
    int64_t endBuffNum{};
  };
  static constexpr auto LARGE_END_BUFF_NUM = 1000000L;

  explicit BufferSaver(const std::string& filenamePrefix,
                       const BufferLimits& bufferLimits = {1, LARGE_END_BUFF_NUM}) noexcept;

  [[nodiscard]] auto GetCurrentFilename() const -> std::string;
  [[nodiscard]] auto GetCurrentBufferNum() const noexcept -> int64_t;
  [[nodiscard]] auto GetStartBufferNum() const noexcept -> int64_t;
  [[nodiscard]] auto GetEndBufferNum() const noexcept -> int64_t;
  [[nodiscard]] auto HaveFinishedBufferRange() const noexcept -> bool;

  auto SetMinimalFormat(bool value) noexcept -> void;

  using BufferIndexFormatter = std::function<std::string(size_t bufferIndex)>;
  auto SetBufferIndexFormatter(const BufferIndexFormatter& bufferIndexFormatter) noexcept -> void;

  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  auto Write(std_spn::span<const T> buffer, bool binaryFormat) -> void;
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  auto Write(const HeaderT& header, std_spn::span<const T> buffer, bool binaryFormat) -> void;

  [[nodiscard]] auto PeekHeader(HeaderT& header, bool binaryFormat) const -> bool;

  static auto WriteBinary(const std::string& filename,
                          int64_t tag,
                          const HeaderT& header,
                          // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
                          std_spn::span<const T> buffer) -> void;
  static auto WriteBinary(std::ostream& file,
                          int64_t tag,
                          const HeaderT& header,
                          // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
                          std_spn::span<const T> buffer) -> void;

  [[nodiscard]] static auto PeekHeaderBinary(const std::string& filename, HeaderT& header) -> bool;
  [[nodiscard]] static auto PeekHeaderBinary(std::istream& file, HeaderT& header) -> bool;

  static auto WriteFormatted(
      std::ostream& file,
      int64_t tag,
      const HeaderT& header,
      // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
      std_spn::span<const T> buffer,
      const BufferIndexFormatter& getBufferIndexString = DefaultGetBufferIndexString,
      bool minimalFormat                               = false) -> void;
  static auto WriteFormatted(
      const std::string& filename,
      int64_t tag,
      const HeaderT& header,
      // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
      std_spn::span<const T> buffer,
      const BufferIndexFormatter& getBufferIndexString = DefaultGetBufferIndexString,
      bool minimalFormat                               = false) -> void;

  [[nodiscard]] static auto PeekHeaderFormatted(const std::string& filename, HeaderT& header)
      -> bool;

protected:
  auto IncCurrentBufferNum() noexcept -> void;

private:
  std::string m_filenamePrefix;
  int64_t m_startBuffNum;
  int64_t m_endBuffNum;
  int64_t m_currentBuffNum;
  bool m_minimalFormat                        = false;
  BufferIndexFormatter m_bufferIndexFormatter = DefaultGetBufferIndexString;
  [[nodiscard]] static auto DefaultGetBufferIndexString(size_t bufferIndex) -> std::string;
  static auto WriteFormattedHeader(std::ostream& file,
                                   int64_t tag,
                                   const HeaderT& header,
                                   size_t bufferSize) -> void;
};

template<class T, class HeaderT>
inline BufferSaver<T, HeaderT>::BufferSaver(const std::string& filenamePrefix,
                                            const BufferLimits& bufferLimits) noexcept
  : m_filenamePrefix{filenamePrefix},
    m_startBuffNum{bufferLimits.startBuffNum},
    m_endBuffNum{bufferLimits.endBuffNum},
    m_currentBuffNum{bufferLimits.startBuffNum}
{
  Expects(bufferLimits.startBuffNum <= bufferLimits.endBuffNum);
  Expects(bufferLimits.startBuffNum > 0);
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::GetCurrentFilename() const -> std::string
{
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  return std_fmt::format("{}_{:05}", m_filenamePrefix, m_currentBuffNum);
}

template<class T, class HeaderT>
inline auto BufferSaver<T, HeaderT>::GetCurrentBufferNum() const noexcept -> int64_t
{
  return m_currentBuffNum;
}

template<class T, class HeaderT>
inline auto BufferSaver<T, HeaderT>::IncCurrentBufferNum() noexcept -> void
{
  ++m_currentBuffNum;
}

template<class T, class HeaderT>
inline auto BufferSaver<T, HeaderT>::GetStartBufferNum() const noexcept -> int64_t
{
  return m_startBuffNum;
}

template<class T, class HeaderT>
inline auto BufferSaver<T, HeaderT>::GetEndBufferNum() const noexcept -> int64_t
{
  return m_endBuffNum;
}

template<class T, class HeaderT>
inline auto BufferSaver<T, HeaderT>::HaveFinishedBufferRange() const noexcept -> bool
{
  return m_currentBuffNum > m_endBuffNum;
}

template<class T, class HeaderT>
inline auto BufferSaver<T, HeaderT>::SetMinimalFormat(const bool value) noexcept -> void
{
  m_minimalFormat = value;
}

template<class T, class HeaderT>
inline auto BufferSaver<T, HeaderT>::SetBufferIndexFormatter(
    const BufferIndexFormatter& bufferIndexFormatter) noexcept -> void
{
  m_bufferIndexFormatter = bufferIndexFormatter;
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::DefaultGetBufferIndexString(const size_t bufferIndex) -> std::string
{
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  return std_fmt::format("{:8d}", bufferIndex);
}

template<class T, class HeaderT>
// NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
auto BufferSaver<T, HeaderT>::Write(const std_spn::span<const T> buffer, const bool binaryFormat)
    -> void
{
  const auto ignore = HeaderT{};
  Write(ignore, buffer, binaryFormat);
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::Write(const HeaderT& header,
                                    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
                                    const std_spn::span<const T> buffer,
                                    const bool binaryFormat) -> void
{
  if ((m_currentBuffNum < m_startBuffNum) or (m_currentBuffNum > m_endBuffNum))
  {
    return;
  }

  const auto filename = GetCurrentFilename();
  if (binaryFormat)
  {
    WriteBinary(filename, m_currentBuffNum, header, buffer);
  }
  else
  {
    WriteFormatted(
        filename, m_currentBuffNum, header, buffer, m_bufferIndexFormatter, m_minimalFormat);
  }

  ++m_currentBuffNum;
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::PeekHeader(HeaderT& header, bool binaryFormat) const -> bool
{
  if ((m_currentBuffNum < m_startBuffNum) or (m_currentBuffNum > m_endBuffNum))
  {
    return false;
  }

  const auto filename = GetCurrentFilename();
  const auto done =
      binaryFormat ? PeekHeaderBinary(filename, header) : PeekHeaderFormatted(filename, header);

  return done;
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::WriteBinary(
    const std::string& filename,
    const int64_t tag,
    const HeaderT& header,
    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
    const std_spn::span<const T> buffer) -> void
{
  auto file = std::ofstream{filename, std::ios::out | std::ios::binary};
  if (not file.good())
  {
    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
    throw std::runtime_error(std_fmt::format(
        "Could not open file '{}' for binary writing. Error: {}.", filename, strerror(errno)));
  }
  WriteBinary(file, tag, header, buffer);
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::WriteBinary(
    std::ostream& file,
    const int64_t tag,
    const HeaderT& header,
    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
    const std_spn::span<const T> buffer) -> void
{
  if constexpr (not std::is_same_v<HeaderT, std::nullptr_t>)
  {
    file.write(ptr_cast<const char*>(&header), sizeof(HeaderT));
  }

  const auto bufferLen = buffer.size();
  file.write(ptr_cast<const char*>(&tag), sizeof(tag));
  file.write(ptr_cast<const char*>(&bufferLen), sizeof(bufferLen));
  file.write(ptr_cast<const char*>(buffer.data()),
             static_cast<std::streamsize>(bufferLen * sizeof(T)));
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::PeekHeaderBinary(const std::string& filename, HeaderT& header) -> bool
{
  auto file = std::ifstream{filename, std::ios::in | std::ios::binary};
  if (not file.good())
  {
    return false;
  }
  return PeekHeaderBinary(file, header);
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::PeekHeaderBinary(std::istream& file, HeaderT& header) -> bool
{
  if constexpr (not std::is_same_v<HeaderT, std::nullptr_t>)
  {
    HeaderT header1;
    file.read(ptr_cast<char*>(&header1), sizeof(HeaderT));
    header = header1;
    file.seekg(0);
  }
  return true;
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::PeekHeaderFormatted(const std::string& filename,
                                                  [[maybe_unused]] HeaderT& header) -> bool
{
  [[maybe_unused]] auto file = std::ifstream{filename, std::ios::in};
  return false;
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::WriteFormatted(
    const std::string& filename,
    const int64_t tag,
    const HeaderT& header,
    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
    const std_spn::span<const T> buffer,
    const BufferIndexFormatter& getBufferIndexString,
    const bool minimalFormat) -> void
{
  auto file = std::ofstream{filename, std::ios::out};
  if (not file.good())
  {
    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
    throw std::runtime_error(std_fmt::format(
        "Could not open file '{}' for text writing. Error: {}.", filename, strerror(errno)));
  }
  WriteFormatted(file, tag, header, buffer, getBufferIndexString, minimalFormat);
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::WriteFormatted(std::ostream& file,
                                             const int64_t tag,
                                             const HeaderT& header,
                                             // NOLINTNEXTLINE(misc-include-cleaner): Need C++20.
                                             const std_spn::span<const T> buffer,
                                             const BufferIndexFormatter& getBufferIndexString,
                                             const bool minimalFormat) -> void
{
  WriteFormattedHeader(file, tag, header, buffer.size());

  for (auto i = 0U; i < buffer.size(); ++i)
  {
    if (minimalFormat)
    {
      file << getBufferIndexString(i) << to_string(buffer[i]);
    }
    else
    {
      // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
      file << std_fmt::format("{}: {}", getBufferIndexString(i), to_string(buffer[i]));
      if (i < (buffer.size() - 1))
      {
        file << ", ";
      }
    }
    file << "\n";
  }
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::WriteFormattedHeader(std::ostream& file,
                                                   int64_t tag,
                                                   const HeaderT& header,
                                                   const size_t bufferSize) -> void
{
  static_assert(not std::is_same_v<HeaderT, std::nullptr_t>, "Write formatted header not ready.");

  file << std_fmt::format("tag:        {:>8d}\n", tag);
  file << std_fmt::format("bufferLen:  {:>8d}\n", bufferSize);
  file << std_fmt::format("bufferSize: {:>8d}\n", bufferSize * sizeof(T));
  file << "\n";

  if constexpr (not std::is_same_v<HeaderT, std::nullptr_t>)
  {
    file << std_fmt::format("width:            {:5d}\n", header.width);
    file << std_fmt::format("height:           {:5d}\n", header.height);
    file << std_fmt::format("averageLuminance: {:5.3f}\n", header.shaderValues.averageLuminance);
    file << std_fmt::format("brightness:       {:5.3f}\n", header.shaderValues.brightness);
    file << std_fmt::format("hueShift:         {:5.3f}\n", header.shaderValues.hueShift);
    file << std_fmt::format("chromaFactor:     {:5.3f}\n", header.shaderValues.chromaFactor);
    file << "\n";
  }
}

} // namespace GOOM::UTILS
