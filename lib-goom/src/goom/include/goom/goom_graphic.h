#pragma once

#ifndef GOOM_DEBUG
#define GOOM_DEBUG
#endif

#include "goom_config.h"

#include <algorithm>
#include <array>
//#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <cstring>
#include <format>
#include <tuple>
#include <vector>

namespace GOOM
{

template<class T>
struct channel_limits
{
  static constexpr auto min() noexcept -> T { return T(); }
  static constexpr auto max() noexcept -> T { return T(); }
};
template<>
struct channel_limits<uint8_t>
{
  static constexpr auto min() noexcept -> uint8_t { return 0; }
  static constexpr auto max() noexcept -> uint8_t { return 255; }
};
template<>
struct channel_limits<uint32_t>
{
  static constexpr auto min() noexcept -> uint32_t { return channel_limits<uint8_t>::min(); }
  static constexpr auto max() noexcept -> uint32_t { return channel_limits<uint8_t>::max(); }
};
template<>
struct channel_limits<int32_t>
{
  static constexpr auto min() noexcept -> int32_t { return channel_limits<uint8_t>::min(); }
  static constexpr auto max() noexcept -> int32_t { return channel_limits<uint8_t>::max(); }
};
template<>
struct channel_limits<float>
{
  static constexpr auto min() noexcept -> float { return channel_limits<uint8_t>::min(); }
  static constexpr auto max() noexcept -> float { return channel_limits<uint8_t>::max(); }
};

constexpr uint8_t MAX_COLOR_VAL = channel_limits<uint8_t>::max();

class Pixel
{
public:
#ifdef COLOR_BGRA
  struct Channels
  {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;
  };
#else
  struct Channels
  {
    uint8_t a = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
  };
#endif /* COLOR_BGRA */

  Pixel();
  explicit Pixel(const Channels& c);
  explicit Pixel(uint32_t val);

  [[nodiscard]] auto R() const -> uint8_t;
  [[nodiscard]] auto G() const -> uint8_t;
  [[nodiscard]] auto B() const -> uint8_t;
  [[nodiscard]] auto A() const -> uint8_t;

  void SetR(uint8_t c);
  void SetG(uint8_t c);
  void SetB(uint8_t c);
  void SetA(uint8_t c);

  [[nodiscard]] auto RFlt() const -> float;
  [[nodiscard]] auto GFlt() const -> float;
  [[nodiscard]] auto BFlt() const -> float;

  [[nodiscard]] auto Rgba() const -> uint32_t;
  void SetRgba(uint32_t v);

  [[nodiscard]] auto ToString() const -> std::string;

  static const Pixel BLACK;
  static const Pixel WHITE;

  friend auto operator==(const Pixel& p1, const Pixel& p2) -> bool;

private:
  union Color
  {
    Channels channels{};
    uint32_t intVal;
  };
  Color m_color{};
};

#if __cplusplus > 201402L
inline const Pixel Pixel::BLACK{/*.channels*/ {/*.r = */ 0, /*.g = */ 0, /*.b = */ 0,
                                               /*.a = */ MAX_COLOR_VAL}};

inline const Pixel Pixel::WHITE{/*.channels*/ {/*.r = */ MAX_COLOR_VAL, /*.g = */ MAX_COLOR_VAL,
                                               /*.b = */ MAX_COLOR_VAL, /*.a = */ MAX_COLOR_VAL}};
#endif

struct FXBuffSettings
{
  static constexpr float INITIAL_BUFF_INTENSITY = 0.5F;
  float buffIntensity = INITIAL_BUFF_INTENSITY;
  bool allowOverexposed = true;
};

class PixelBuffer
{
  using Buffer = std::vector<Pixel>;

public:
  PixelBuffer() noexcept = default;
  PixelBuffer(uint32_t width, uint32_t height) noexcept;
  PixelBuffer(const PixelBuffer&) noexcept = delete;
  PixelBuffer(PixelBuffer&&) noexcept = delete;
  auto operator=(const PixelBuffer&) -> PixelBuffer& = delete;
  auto operator=(PixelBuffer&&) -> PixelBuffer& = delete;
  virtual ~PixelBuffer() noexcept = default;

  void Resize(size_t width, size_t height);

  [[nodiscard]] auto GetWidth() const -> uint32_t;
  [[nodiscard]] auto GetHeight() const -> uint32_t;

  void Fill(const Pixel& c);
  void CopyTo(PixelBuffer& buff) const;
  [[nodiscard]] auto GetIntBuff() const -> const uint32_t*;

  auto operator()(size_t x, size_t y) const -> const Pixel&;
  auto operator()(size_t x, size_t y) -> Pixel&;

  using iterator = Buffer::iterator;
  using const_iterator = Buffer::const_iterator;
  [[nodiscard]] auto GetRowIter(size_t y) -> std::tuple<iterator, iterator>;
  [[nodiscard]] auto GetRowIter(size_t y) const -> std::tuple<const_iterator, const_iterator>;

  static constexpr size_t NUM_NBRS = 4;
  [[nodiscard]] auto Get4RHBNeighbours(size_t x, size_t y) const -> std::array<Pixel, NUM_NBRS>;

#ifdef GOOM_DEBUG
  class RangeError : public std::logic_error
  {
  public:
    explicit RangeError(const std::string& msg) : std::logic_error{msg} {}
  };
#endif

private:
  uint32_t m_width{};
  uint32_t m_height{};
  uint32_t m_xMax{};
  uint32_t m_yMax{};
  Buffer m_buff{};

  [[nodiscard]] auto GetIntBuff() -> uint32_t*;
};

inline Pixel::Pixel() : m_color{/*.channels*/ {}}
{
}

inline Pixel::Pixel(const Channels& c) : m_color{/*.channels*/ {c}}
{
}

inline Pixel::Pixel(const uint32_t val)
{
  m_color.intVal = val;
}

inline auto operator==(const Pixel& p1, const Pixel& p2) -> bool
{
  return p1.Rgba() == p2.Rgba();
}

inline auto Pixel::R() const -> uint8_t
{
  return m_color.channels.r;
}

inline void Pixel::SetR(const uint8_t c)
{
  m_color.channels.r = c;
}

inline auto Pixel::G() const -> uint8_t
{
  return m_color.channels.g;
}

inline void Pixel::SetG(const uint8_t c)
{
  m_color.channels.g = c;
}

inline auto Pixel::B() const -> uint8_t
{
  return m_color.channels.b;
}

inline void Pixel::SetB(const uint8_t c)
{
  m_color.channels.b = c;
}

inline auto Pixel::A() const -> uint8_t
{
  return m_color.channels.a;
}

inline void Pixel::SetA(const uint8_t c)
{
  m_color.channels.a = c;
}

inline auto Pixel::RFlt() const -> float
{
  return static_cast<float>(R()) / channel_limits<float>::max();
}

inline auto Pixel::GFlt() const -> float
{
  return static_cast<float>(G()) / channel_limits<float>::max();
}

inline auto Pixel::BFlt() const -> float
{
  return static_cast<float>(B()) / channel_limits<float>::max();
}

inline auto Pixel::Rgba() const -> uint32_t
{
  return m_color.intVal;
}

inline void Pixel::SetRgba(const uint32_t v)
{
  m_color.intVal = v;
}

inline auto Pixel::ToString() const -> std::string
{
  return std20::format("({}, {}, {}, {})", R(), G(), B(), A());
}

inline PixelBuffer::PixelBuffer(const uint32_t width, const uint32_t height) noexcept
  : m_width{width},
    m_height{height},
    m_xMax{m_width - 1},
    m_yMax{m_height - 1},
    m_buff(m_width * m_height)
{
}

inline void PixelBuffer::Resize(const size_t width, const size_t height)
{
  m_width = static_cast<uint32_t>(width);
  m_height = static_cast<uint32_t>(height);
  m_xMax = m_width - 1;
  m_yMax = m_height - 1;
  m_buff.resize(m_width * m_height);
}

inline auto PixelBuffer::GetWidth() const -> uint32_t
{
  return m_width;
}

inline auto PixelBuffer::GetHeight() const -> uint32_t
{
  return m_height;
}

inline void PixelBuffer::Fill(const Pixel& c)
{
  std::fill(m_buff.begin(), m_buff.end(), c);
}

inline auto PixelBuffer::GetIntBuff() const -> const uint32_t*
{
  return reinterpret_cast<const uint32_t*>(m_buff.data());
}

inline auto PixelBuffer::GetIntBuff() -> uint32_t*
{
  return reinterpret_cast<uint32_t*>(m_buff.data());
}

inline void PixelBuffer::CopyTo(PixelBuffer& buff) const
{
  // Get the last bit of speed here and use memmove.
  // std::copy(m_buff.cbegin(), m_buff.cend(), buff.m_buff.begin());
  static_assert(sizeof(Pixel) == sizeof(uint32_t), "Invalid Pixel size.");
  std::memmove(buff.GetIntBuff(), this->GetIntBuff(), buff.m_buff.size() * sizeof(Pixel));
}

inline auto PixelBuffer::operator()(const size_t x, const size_t y) const -> const Pixel&
{
#ifdef GOOM_DEBUG
  return m_buff.at((y * m_width) + x);
#else
  return m_buff[(y * m_width) + x];
#endif
}

inline auto PixelBuffer::operator()(const size_t x, const size_t y) -> Pixel&
{
#ifdef GOOM_DEBUG
  return m_buff.at((y * m_width) + x);
#else
  return m_buff[(y * m_width) + x];
#endif
}

inline auto PixelBuffer::GetRowIter(const size_t y)
    -> std::tuple<PixelBuffer::iterator, PixelBuffer::iterator>
{
  const auto rowPos = static_cast<int32_t>(y * m_width);
  return std::make_tuple(m_buff.begin() + rowPos,
                         m_buff.begin() + rowPos + static_cast<int32_t>(m_width));
}

inline auto PixelBuffer::GetRowIter(const size_t y) const
    -> std::tuple<PixelBuffer::const_iterator, PixelBuffer::const_iterator>
{
  const auto rowPos = static_cast<int32_t>(y * m_width);
  return std::make_tuple(m_buff.begin() + rowPos,
                         m_buff.begin() + rowPos + static_cast<int32_t>(m_width));
}

inline auto PixelBuffer::Get4RHBNeighbours(const size_t x, const size_t y) const
    -> std::array<Pixel, NUM_NBRS>
{
#ifdef GOOM_DEBUG
  if (x >= m_xMax)
  {
    throw RangeError{
        std20::format("Get4RHBNeighbours range error: x = {}, m_xMax = {}", x, m_xMax)};
  }
  if (y >= m_yMax)
  {
    throw RangeError{
        std20::format("Get4RHBNeighbours range error: x = {}, m_xMax = {}", y, m_yMax)};
  }
#endif

  const size_t xPos = (y * m_width) + x;

  return {
      m_buff[xPos],
      m_buff[xPos + 1],
      m_buff[xPos + m_width],
      m_buff[xPos + m_width + 1],
  };
}

} // namespace GOOM
