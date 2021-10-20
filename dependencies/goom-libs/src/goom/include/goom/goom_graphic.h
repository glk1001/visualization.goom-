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

using PixelChannelType = uint8_t;
using PixelIntType = uint32_t;

constexpr PixelChannelType MAX_COLOR_VAL = channel_limits<PixelChannelType>::max();
constexpr PixelChannelType MAX_ALPHA = MAX_COLOR_VAL;

// TODO - maybe should be template: Pixel<uint8_t>, Pixel<uint16_t>
class Pixel
{
public:
  struct RGB
  {
    PixelChannelType red = 0;
    PixelChannelType green = 0;
    PixelChannelType blue = 0;
    PixelChannelType alpha = MAX_ALPHA;
  };

  Pixel();
  Pixel(const RGB& color);
  Pixel(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha);

  [[nodiscard]] auto R() const -> PixelChannelType;
  [[nodiscard]] auto G() const -> PixelChannelType;
  [[nodiscard]] auto B() const -> PixelChannelType;
  [[nodiscard]] auto A() const -> PixelChannelType;

  void SetR(PixelChannelType val);
  void SetG(PixelChannelType val);
  void SetB(PixelChannelType val);
  void SetA(PixelChannelType val);

  [[nodiscard]] auto RFlt() const -> float;
  [[nodiscard]] auto GFlt() const -> float;
  [[nodiscard]] auto BFlt() const -> float;

  [[nodiscard]] auto Rgba() const -> PixelIntType;

  [[nodiscard]] auto ToString() const -> std::string;

  static const Pixel BLACK;
  static const Pixel WHITE;

  friend auto operator==(const Pixel& pixel1, const Pixel& pixel2) -> bool;

private:
#ifdef COLOR_BGRA
  struct Channels
  {
    PixelChannelType r = 0;
    PixelChannelType g = 0;
    PixelChannelType b = 0;
    PixelChannelType a = 0;
  };
#else
  struct Channels
  {
    PixelChannelType a = 0;
    PixelChannelType r = 0;
    PixelChannelType g = 0;
    PixelChannelType b = 0;
  };
#endif /* COLOR_BGRA */

  union Color
  {
    Channels channels{};
    PixelIntType intVal;
  };
  Color m_color{};
};

[[nodiscard]] auto GetPixelScaledByMax(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha)
    -> Pixel;
[[nodiscard]] auto MultiplyColorChannels(PixelChannelType ch1, PixelChannelType ch2) -> uint32_t;
[[nodiscard]] auto MultiplyChannelColorByScalar(uint32_t scalar, PixelChannelType channelVal)
    -> uint32_t;
[[nodiscard]] auto DivideChannelColorByScalar(const uint32_t scalar, const uint32_t channelVal)
    -> uint32_t;
[[nodiscard]] auto ScaleChannelColorAfterIntegerMultiply(uint32_t channelVal) -> uint32_t;

#if __cplusplus > 201402L
inline const Pixel Pixel::BLACK{{/*.red = */ 0, /*.green = */ 0, /*.blue = */ 0, /*.alpha = */ 0}};

inline const Pixel Pixel::WHITE{{/*.red = */ MAX_COLOR_VAL, /*.green = */ MAX_COLOR_VAL,
                                 /*.blue = */ MAX_COLOR_VAL, /*.alpha = */ MAX_COLOR_VAL}};
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

  void Fill(const Pixel& pixel);
  void CopyTo(PixelBuffer& pixelBuffer) const;
  [[nodiscard]] static auto GetIntBufferSize(uint32_t width, uint32_t height) -> size_t;
  [[nodiscard]] auto GetIntBuff() const -> const PixelIntType*;

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
};

inline Pixel::Pixel() : m_color{/*.channels*/ {}}
{
}

inline Pixel::Pixel(const RGB& color)
  : m_color{{/*.r = */ color.red,
             /*.g = */ color.green,
             /*.b = */ color.blue,
             /*.a = */ color.alpha}}
{
}

inline Pixel::Pixel(const uint32_t red,
                    const uint32_t green,
                    const uint32_t blue,
                    const uint32_t alpha)
  : m_color{
        {/*.r = */ static_cast<PixelChannelType>(std::min(channel_limits<uint32_t>::max(), red)),
         /*.g = */ static_cast<PixelChannelType>(std::min(channel_limits<uint32_t>::max(), green)),
         /*.b = */ static_cast<PixelChannelType>(std::min(channel_limits<uint32_t>::max(), blue)),
         /*.a = */ static_cast<PixelChannelType>(std::min(channel_limits<uint32_t>::max(), alpha))}}
{
}

inline auto GetPixelScaledByMax(uint32_t red, uint32_t green, uint32_t blue, const uint32_t alpha)
    -> Pixel
{
  const uint32_t maxVal = std::max({red, green, blue});

  if (maxVal > channel_limits<uint32_t>::max())
  {
    // scale all channels back
    red = DivideChannelColorByScalar(maxVal, red);
    green = DivideChannelColorByScalar(maxVal, green);
    blue = DivideChannelColorByScalar(maxVal, blue);
  }

  return {red, green, blue, alpha};
}

inline auto MultiplyColorChannels(const PixelChannelType ch1, const PixelChannelType ch2)
    -> uint32_t
{
  return ScaleChannelColorAfterIntegerMultiply(static_cast<uint32_t>(ch1) *
                                               static_cast<uint32_t>(ch2));
}

inline auto MultiplyChannelColorByScalar(const uint32_t scalar, const PixelChannelType channelVal)
    -> uint32_t
{
  return ScaleChannelColorAfterIntegerMultiply((scalar + 1) * static_cast<uint32_t>(channelVal));
}

inline auto DivideChannelColorByScalar(const uint32_t scalar, const uint32_t channelVal) -> uint32_t
{
  return (channelVal * channel_limits<uint32_t>::max()) / scalar;
  ;
}

inline auto ScaleChannelColorAfterIntegerMultiply(const uint32_t channelVal) -> uint32_t
{
  return channelVal / channel_limits<uint32_t>::max();
}

inline auto operator==(const Pixel& pixel1, const Pixel& pixel2) -> bool
{
  return pixel1.m_color.intVal == pixel2.m_color.intVal;
}

inline auto Pixel::R() const -> PixelChannelType
{
  return m_color.channels.r;
}

inline void Pixel::SetR(const PixelChannelType val)
{
  m_color.channels.r = val;
}

inline auto Pixel::G() const -> PixelChannelType
{
  return m_color.channels.g;
}

inline void Pixel::SetG(const PixelChannelType val)
{
  m_color.channels.g = val;
}

inline auto Pixel::B() const -> PixelChannelType
{
  return m_color.channels.b;
}

inline void Pixel::SetB(const PixelChannelType val)
{
  m_color.channels.b = val;
}

inline auto Pixel::A() const -> PixelChannelType
{
  return m_color.channels.a;
}

inline void Pixel::SetA(const PixelChannelType val)
{
  m_color.channels.a = val;
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

inline auto Pixel::Rgba() const -> PixelIntType
{
  return m_color.intVal;
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
    m_buff(static_cast<size_t>(m_width) * static_cast<size_t>(m_height))
{
}

inline void PixelBuffer::Resize(const size_t width, const size_t height)
{
  m_width = static_cast<uint32_t>(width);
  m_height = static_cast<uint32_t>(height);
  m_xMax = m_width - 1;
  m_yMax = m_height - 1;
  m_buff.resize(static_cast<size_t>(m_width) * static_cast<size_t>(m_height));
}

inline auto PixelBuffer::GetWidth() const -> uint32_t
{
  return m_width;
}

inline auto PixelBuffer::GetHeight() const -> uint32_t
{
  return m_height;
}

inline void PixelBuffer::Fill(const Pixel& pixel)
{
  std::fill(m_buff.begin(), m_buff.end(), pixel);
}

inline auto PixelBuffer::GetIntBufferSize(const uint32_t width, const uint32_t height) -> size_t
{
  return static_cast<size_t>(width * height) * sizeof(Pixel);
}

inline auto PixelBuffer::GetIntBuff() const -> const PixelIntType*
{
  return reinterpret_cast<const PixelIntType*>(m_buff.data());
}

inline void PixelBuffer::CopyTo(PixelBuffer& pixelBuffer) const
{
  std::copy(cbegin(m_buff), cend(m_buff), begin(pixelBuffer.m_buff));
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
