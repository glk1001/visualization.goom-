module;

#include "goom/goom_config.h"

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <fstream>
#include <functional>
#include <ios>
#include <istream>
#include <ostream>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>

export module Goom.Utils.BufferSaver;

import Goom.Lib.GoomUtils;

export namespace GOOM::UTILS
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

  using BufferIndexFormatter    = std::function<std::string(size_t bufferIndex)>;
  using BufferToStringFormatter = std::function<std::string(const T& bufferValue)>;
  struct Formatters
  {
    bool minimalFormat                              = false;
    BufferIndexFormatter bufferIndexFormatter       = DefaultGetBufferIndexString;
    BufferToStringFormatter bufferToStringFormatter = DefaultBufferToString;
  };
  auto SetFormatters(const Formatters& formatters) noexcept -> void;

  auto Write(std::span<const T> buffer, bool binaryFormat) -> void;
  auto Write(const HeaderT& header, std::span<const T> buffer, bool binaryFormat) -> void;

  [[nodiscard]] auto PeekHeader(HeaderT& header, bool binaryFormat) const -> bool;

  static auto WriteBinary(const std::string& filename,
                          int64_t tag,
                          const HeaderT& header,
                          std::span<const T> buffer) -> void;
  static auto WriteBinary(std::ostream& file,
                          int64_t tag,
                          const HeaderT& header,
                          std::span<const T> buffer) -> void;

  [[nodiscard]] static auto PeekHeaderBinary(const std::string& filename, HeaderT& header) -> bool;
  [[nodiscard]] static auto PeekHeaderBinary(std::istream& file, HeaderT& header) -> bool;

  static auto WriteFormatted(std::ostream& file,
                             int64_t tag,
                             const HeaderT& header,
                             std::span<const T> buffer,
                             const Formatters& formatters = Formatters{}) -> void;
  static auto WriteFormatted(const std::string& filename,
                             int64_t tag,
                             const HeaderT& header,
                             std::span<const T> buffer,
                             const Formatters& formatters = Formatters{}) -> void;

  [[nodiscard]] static auto PeekHeaderFormatted(const std::string& filename, HeaderT& header)
      -> bool;

protected:
  auto IncCurrentBufferNum() noexcept -> void;

private:
  std::string m_filenamePrefix;
  int64_t m_startBuffNum;
  int64_t m_endBuffNum;
  int64_t m_currentBuffNum;
  Formatters m_formatters{};
  [[nodiscard]] static auto DefaultGetBufferIndexString(size_t bufferIndex) -> std::string;
  [[nodiscard]] static auto DefaultBufferToString(const T& bufferValue) -> std::string;
  static auto WriteFormattedHeader(std::ostream& file,
                                   int64_t tag,
                                   const HeaderT& header,
                                   size_t bufferSize) -> void;
};

template<class HeaderT>
[[nodiscard]] auto GetFormattedBufferHeader(const HeaderT& nullHeader) -> std::string;
template<class T>
[[nodiscard]] auto GetBufferValueToString(const T& bufferValue) -> std::string;

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
  return std::format("{}_{:05}", m_filenamePrefix, m_currentBuffNum);
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
inline auto BufferSaver<T, HeaderT>::SetFormatters(const Formatters& formatters) noexcept -> void
{
  m_formatters = formatters;
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::DefaultGetBufferIndexString(const size_t bufferIndex) -> std::string
{
  return std::format("{:8d}", bufferIndex);
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::Write(const std::span<const T> buffer, const bool binaryFormat)
    -> void
{
  const auto ignore = HeaderT{};
  Write(ignore, buffer, binaryFormat);
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::Write(const HeaderT& header,
                                    const std::span<const T> buffer,
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
    WriteFormatted(filename, m_currentBuffNum, header, buffer, m_formatters);
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
auto BufferSaver<T, HeaderT>::WriteBinary(const std::string& filename,
                                          const int64_t tag,
                                          const HeaderT& header,
                                          const std::span<const T> buffer) -> void
{
  auto file = std::ofstream{filename, std::ios::out | std::ios::binary};
  if (not file.good())
  {
    throw std::runtime_error(std::format(
        "Could not open file '{}' for binary writing. Error: {}.", filename, strerror(errno)));
  }
  WriteBinary(file, tag, header, buffer);
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::WriteBinary(std::ostream& file,
                                          const int64_t tag,
                                          const HeaderT& header,
                                          const std::span<const T> buffer) -> void
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
auto BufferSaver<T, HeaderT>::WriteFormatted(const std::string& filename,
                                             const int64_t tag,
                                             const HeaderT& header,
                                             const std::span<const T> buffer,
                                             const Formatters& formatters) -> void
{
  auto file = std::ofstream{filename, std::ios::out};
  if (not file.good())
  {
    throw std::runtime_error(std::format(
        "Could not open file '{}' for text writing. Error: {}.", filename, strerror(errno)));
  }
  WriteFormatted(file, tag, header, buffer, formatters);
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::WriteFormatted(std::ostream& file,
                                             const int64_t tag,
                                             const HeaderT& header,
                                             const std::span<const T> buffer,
                                             const Formatters& formatters) -> void
{
  WriteFormattedHeader(file, tag, header, buffer.size());

  for (auto i = 0U; i < buffer.size(); ++i)
  {
    if (formatters.minimalFormat)
    {
      file << formatters.bufferIndexFormatter(i) << formatters.bufferToStringFormatter(buffer[i]);
    }
    else
    {
      file << std::format("{}: {}",
                          formatters.bufferIndexFormatter(i),
                          formatters.bufferToStringFormatter(buffer[i]));
      if (i < (buffer.size() - 1))
      {
        file << ", ";
      }
    }
    file << "\n";
  }
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::DefaultBufferToString(const T& bufferValue) -> std::string
{
  return GetBufferValueToString(bufferValue);
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::WriteFormattedHeader(std::ostream& file,
                                                   int64_t tag,
                                                   const HeaderT& header,
                                                   const size_t bufferSize) -> void
{
  static_assert(not std::is_same_v<HeaderT, std::nullptr_t>, "Write formatted header not ready.");

  file << std::format("tag:        {:>8d}\n", tag);
  file << std::format("bufferLen:  {:>8d}\n", bufferSize);
  file << std::format("bufferSize: {:>8d}\n", bufferSize * sizeof(T));
  file << "\n";

  file << GetFormattedBufferHeader(header);
}

template<class HeaderT>
auto GetFormattedBufferHeader([[maybe_unused]] const HeaderT& nullHeader) -> std::string
{
  return "";
}

template<class T>
auto GetBufferValueToString(const T& bufferValue) -> std::string
{
  return std::to_string(bufferValue);
}

} // namespace GOOM::UTILS
