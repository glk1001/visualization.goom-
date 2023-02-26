#pragma once

#include "buffer_view.h"

#include <cstdint>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace GOOM::UTILS
{

struct EmptyHeaderType
{
  int x;
};

template<class T, class HeaderT = EmptyHeaderType>
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
                       const BufferLimits& bufferLimits = {0, LARGE_END_BUFF_NUM});

  [[nodiscard]] auto GetCurrentFilename() const -> std::string;
  [[nodiscard]] auto GetCurrentBufferNum() const -> int64_t;
  [[nodiscard]] auto GetStartBufferNum() const -> int64_t;
  [[nodiscard]] auto GetEndBufferNum() const -> int64_t;
  [[nodiscard]] auto HaveFinishedBufferRange() const -> bool;

  void Write(const BufferView<T>& buffer, bool binaryFormat);
  void Write(const HeaderT& header, const BufferView<T>& buffer, bool binaryFormat);

  [[nodiscard]] auto PeekHeader(HeaderT& header, bool binaryFormat) const -> bool;

  static void WriteBinary(const std::string& filename,
                          int64_t tag,
                          const HeaderT& header,
                          const BufferView<T>& buffer);
  static void WriteBinary(std::ostream& file,
                          int64_t tag,
                          const HeaderT& header,
                          const BufferView<T>& buffer);

  [[nodiscard]] static auto PeekHeaderBinary(const std::string& filename, HeaderT& header) -> bool;
  [[nodiscard]] static auto PeekHeaderBinary(std::istream& file, HeaderT& header) -> bool;

  static void WriteFormatted(std::ostream& file,
                             int64_t tag,
                             const HeaderT& header,
                             const BufferView<T>& buffer);
  static void WriteFormatted(const std::string& filename,
                             int64_t tag,
                             const HeaderT& header,
                             const BufferView<T>& buffer);

  [[nodiscard]] static auto PeekHeaderFormatted(const std::string& filename, HeaderT& header)
      -> bool;

protected:
  auto IncCurrentBufferNum() noexcept -> void;

private:
  std::string m_filenamePrefix;
  int64_t m_startBuffNum;
  int64_t m_endBuffNum;
  int64_t m_currentBuffNum;
};

template<class T, class HeaderT>
inline BufferSaver<T, HeaderT>::BufferSaver(const std::string& filenamePrefix,
                                            const BufferLimits& bufferLimits)
  : m_filenamePrefix{filenamePrefix},
    m_startBuffNum{bufferLimits.startBuffNum},
    m_endBuffNum{bufferLimits.endBuffNum},
    m_currentBuffNum{bufferLimits.startBuffNum}
{
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::GetCurrentFilename() const -> std::string
{
  return std_fmt::format("{}_{:05}", m_filenamePrefix, m_currentBuffNum);
}

template<class T, class HeaderT>
inline auto BufferSaver<T, HeaderT>::GetCurrentBufferNum() const -> int64_t
{
  return m_currentBuffNum;
}

template<class T, class HeaderT>
inline auto BufferSaver<T, HeaderT>::IncCurrentBufferNum() noexcept -> void
{
  ++m_currentBuffNum;
}

template<class T, class HeaderT>
inline auto BufferSaver<T, HeaderT>::GetStartBufferNum() const -> int64_t
{
  return m_startBuffNum;
}

template<class T, class HeaderT>
inline auto BufferSaver<T, HeaderT>::GetEndBufferNum() const -> int64_t
{
  return m_endBuffNum;
}

template<class T, class HeaderT>
inline auto BufferSaver<T, HeaderT>::HaveFinishedBufferRange() const -> bool
{
  return m_currentBuffNum > m_endBuffNum;
}

template<class T, class HeaderT>
void BufferSaver<T, HeaderT>::Write(const BufferView<T>& buffer, const bool binaryFormat)
{
  auto ignore = HeaderT{};
  Write(ignore, buffer, binaryFormat);
}

template<class T, class HeaderT>
void BufferSaver<T, HeaderT>::Write(const HeaderT& header,
                                    const BufferView<T>& buffer,
                                    const bool binaryFormat)
{
  if ((m_currentBuffNum < m_startBuffNum) || (m_currentBuffNum > m_endBuffNum))
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
    WriteFormatted(filename, m_currentBuffNum, header, buffer);
  }
  ++m_currentBuffNum;
}

template<class T, class HeaderT>
auto BufferSaver<T, HeaderT>::PeekHeader(HeaderT& header, bool binaryFormat) const -> bool
{
  if ((m_currentBuffNum < m_startBuffNum) || (m_currentBuffNum > m_endBuffNum))
  {
    return false;
  }

  const auto filename = GetCurrentFilename();
  const auto done =
      binaryFormat ? PeekHeaderBinary(filename, header) : PeekHeaderFormatted(filename, header);

  return done;
}

template<class T, class HeaderT>
void BufferSaver<T, HeaderT>::WriteBinary(const std::string& filename,
                                          const int64_t tag,
                                          const HeaderT& header,
                                          const BufferView<T>& buffer)
{
  auto file = std::ofstream{filename, std::ios::out | std::ios::binary};
  if (not file.good())
  {
    throw std::runtime_error(std_fmt::format("Could not open file for writing: '{}'.", filename));
  }
  WriteBinary(file, tag, header, buffer);
}

template<class T, class HeaderT>
void BufferSaver<T, HeaderT>::WriteBinary(std::ostream& file,
                                          const int64_t tag,
                                          const HeaderT& header,
                                          const BufferView<T>& buffer)
{
  if (typeid(HeaderT) != typeid(EmptyHeaderType))
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): Wait for C++20 std::bitcast?
    file.write(reinterpret_cast<const char*>(&header), sizeof(HeaderT));
  }

  const auto bufferLen = buffer.GetBufferLen();
  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast): Wait for C++20 std::bitcast?
  file.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
  file.write(reinterpret_cast<const char*>(&bufferLen), sizeof(bufferLen));
  file.write(reinterpret_cast<const char*>(buffer.Data()),
             static_cast<std::streamsize>(bufferLen * sizeof(T)));
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast): Wait for C++20 std::bitcast?
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
  if (typeid(HeaderT) != typeid(EmptyHeaderType))
  {
    HeaderT header1;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): Wait for C++20 std::bitcast?
    file.read(reinterpret_cast<char*>(&header1), sizeof(HeaderT));
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
void BufferSaver<T, HeaderT>::WriteFormatted(const std::string& filename,
                                             const int64_t tag,
                                             const HeaderT& header,
                                             const BufferView<T>& buffer)
{
  auto file = std::ofstream{filename, std::ios::out};
  WriteFormatted(file, tag, header, buffer);
}

template<class T, class HeaderT>
void BufferSaver<T, HeaderT>::WriteFormatted(std::ostream& file,
                                             const int64_t tag,
                                             [[maybe_unused]] const HeaderT& header,
                                             const BufferView<T>& buffer)
{
  if (typeid(HeaderT) != typeid(EmptyHeaderType))
  {
    throw std::runtime_error("Write formatted header not ready.");
  }

  file << "tag: " << tag << "\n";
  file << "bufferLen: " << buffer.GetBufferLen() << "\n";
  file << "bufferSize: " << (buffer.GetBufferLen() * sizeof(T)) << "\n";
  for (auto i = 0U; i < buffer.GetBufferLen(); ++i)
  {
    //    file << std20::format("{:#018x}", buffer[i]);
    file << std_fmt::format("{}", buffer[i]);
    if (i < (buffer.GetBufferLen() - 1))
    {
      file << ", ";
    }
    file << "\n";
  }
  file << "\n";
}

} // namespace GOOM::UTILS
