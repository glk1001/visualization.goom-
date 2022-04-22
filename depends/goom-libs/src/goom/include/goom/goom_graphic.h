#pragma once

#include "goom_config.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <format>
#include <limits>
#include <tuple>
#include <vector>

namespace GOOM
{

struct GoomShaderEffects
{
  float exposure = 0.0F;
  float brightness = 0.0F;
  float contrast = 0.0F;
  float contrastMinChannelValue = 0.0F; // negative gives interesting effects
};

template<class T>
struct channel_limits
{
  [[nodiscard]] static constexpr auto min() noexcept -> T { return T(); }
  [[nodiscard]] static constexpr auto max() noexcept -> T { return T(); }
};
template<>
struct channel_limits<uint8_t>
{
  [[nodiscard]] static constexpr auto min() noexcept -> uint8_t { return 0; }
  [[nodiscard]] static constexpr auto max() noexcept -> uint8_t
  {
    return std::numeric_limits<uint8_t>::max();
  }
};
template<>
struct channel_limits<uint16_t>
{
  [[nodiscard]] static constexpr auto min() noexcept -> uint16_t
  {
    return channel_limits<uint8_t>::min();
  }
  [[nodiscard]] static constexpr auto max() noexcept -> uint16_t
  {
    return channel_limits<uint8_t>::max();
  }
};
template<>
struct channel_limits<uint32_t>
{
  [[nodiscard]] static constexpr auto min() noexcept -> uint32_t
  {
    return channel_limits<uint8_t>::min();
  }
  [[nodiscard]] static constexpr auto max() noexcept -> uint32_t
  {
    return channel_limits<uint8_t>::max();
  }
};
template<>
struct channel_limits<int32_t>
{
  [[nodiscard]] static constexpr auto min() noexcept -> int32_t
  {
    return channel_limits<uint8_t>::min();
  }
  [[nodiscard]] static constexpr auto max() noexcept -> int32_t
  {
    return channel_limits<uint8_t>::max();
  }
};
template<>
struct channel_limits<float>
{
  [[nodiscard]] static constexpr auto min() noexcept -> float
  {
    return channel_limits<uint8_t>::min();
  }
  [[nodiscard]] static constexpr auto max() noexcept -> float
  {
    return channel_limits<uint8_t>::max();
  }
};

using PixelChannelType = uint16_t;
using PixelIntType = uint64_t;
static constexpr uint32_t MAX_CHANNEL_VALUE_HDR = 30 * 1024;

static constexpr PixelChannelType MAX_COLOR_VAL = channel_limits<PixelChannelType>::max();
static constexpr PixelChannelType MAX_ALPHA = MAX_COLOR_VAL;

static_assert(MAX_CHANNEL_VALUE_HDR <= std::numeric_limits<PixelChannelType>::max(),
              "Invalid MAX_CHANNEL_VALUE_HDR");

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

  Pixel() noexcept;
  explicit Pixel(const RGB& color) noexcept;
  Pixel(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha) noexcept;

  [[nodiscard]] auto R() const noexcept -> PixelChannelType;
  [[nodiscard]] auto G() const noexcept -> PixelChannelType;
  [[nodiscard]] auto B() const noexcept -> PixelChannelType;
  [[nodiscard]] auto A() const noexcept -> PixelChannelType;

  auto SetR(PixelChannelType val) noexcept -> void;
  auto SetG(PixelChannelType val) noexcept -> void;
  auto SetB(PixelChannelType val) noexcept -> void;
  auto SetA(PixelChannelType val) noexcept -> void;

  [[nodiscard]] auto RFlt() const noexcept -> float;
  [[nodiscard]] auto GFlt() const noexcept -> float;
  [[nodiscard]] auto BFlt() const noexcept -> float;

  [[nodiscard]] auto Rgba() const noexcept -> PixelIntType;

  [[nodiscard]] auto ToString() const noexcept -> std::string;

  static const Pixel BLACK;
  static const Pixel WHITE;

  [[nodiscard]] auto IsBlack() const noexcept -> bool;

  friend auto operator==(const Pixel& pixel1, const Pixel& pixel2) noexcept -> bool;

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

[[nodiscard]] auto MultiplyColorChannels(PixelChannelType ch1, PixelChannelType ch2) noexcept
    -> uint32_t;
[[nodiscard]] auto MultiplyChannelColorByScalar(uint32_t scalar,
                                                PixelChannelType channelVal) noexcept -> uint32_t;

struct FXBuffSettings
{
  static constexpr float INITIAL_BUFF_INTENSITY = 0.5F;
  float buffIntensity = INITIAL_BUFF_INTENSITY;
};

class PixelBuffer
{
  using Buffer = std::vector<Pixel>;

public:
  PixelBuffer() noexcept = default;
  PixelBuffer(uint32_t width, uint32_t height) noexcept;
  PixelBuffer(const PixelBuffer&) = delete;
  PixelBuffer(PixelBuffer&&) = delete;
  auto operator=(const PixelBuffer&) -> PixelBuffer& = delete;
  auto operator=(PixelBuffer&&) -> PixelBuffer& = delete;
  virtual ~PixelBuffer() noexcept = default;

  auto Resize(size_t width, size_t height) noexcept -> void;

  [[nodiscard]] auto GetWidth() const noexcept -> uint32_t;
  [[nodiscard]] auto GetHeight() const noexcept -> uint32_t;

  auto Fill(const Pixel& pixel) noexcept -> void;
  auto CopyTo(PixelBuffer& pixelBuffer) const noexcept -> void;
  [[nodiscard]] static auto GetIntBufferSize(uint32_t width, uint32_t height) noexcept -> size_t;
  [[nodiscard]] auto GetIntBuff() const noexcept -> const PixelIntType*;

  auto operator()(size_t x, size_t y) const noexcept -> const Pixel&;
  auto operator()(size_t x, size_t y) noexcept -> Pixel&;

  using iterator = Buffer::iterator;
  using const_iterator = Buffer::const_iterator;
  [[nodiscard]] auto GetRowIter(size_t y) noexcept -> std::tuple<iterator, iterator>;
  [[nodiscard]] auto GetRowIter(size_t y) const noexcept
      -> std::tuple<const_iterator, const_iterator>;

  static constexpr size_t NUM_NBRS = 4;
  [[nodiscard]] auto Get4RHBNeighbours(size_t x, size_t y) const noexcept
      -> std::array<Pixel, NUM_NBRS>;

#ifdef GOOM_DEBUG
  class RangeError : public std::logic_error
  {
  public:
    using std::logic_error::logic_error;
  };
#endif

private:
  uint32_t m_width = 0;
  uint32_t m_height = 0;
  uint32_t m_xMax = 0;
  uint32_t m_yMax = 0;
  Buffer m_buff{};
};

static_assert(sizeof(Pixel) == sizeof(PixelIntType), "Invalid Pixel size.");

inline Pixel::Pixel() noexcept : m_color{Channels{}}
{
}

inline Pixel::Pixel(const RGB& color) noexcept
  : m_color{
        {color.red, color.green, color.blue, color.alpha}
}
{
}

inline Pixel::Pixel(const uint32_t red,
                    const uint32_t green,
                    const uint32_t blue,
                    const uint32_t alpha) noexcept
  : m_color{
        {static_cast<PixelChannelType>(std::min(MAX_CHANNEL_VALUE_HDR, red)),
         static_cast<PixelChannelType>(std::min(MAX_CHANNEL_VALUE_HDR, green)),
         static_cast<PixelChannelType>(std::min(MAX_CHANNEL_VALUE_HDR, blue)),
         static_cast<PixelChannelType>(std::min(static_cast<uint32_t>(MAX_ALPHA), alpha))}
}
{
}

inline auto MultiplyColorChannels(const PixelChannelType ch1, const PixelChannelType ch2) noexcept
    -> uint32_t
{
  return (static_cast<uint32_t>(ch1) * static_cast<uint32_t>(ch2)) /
         channel_limits<uint32_t>::max();
}

inline auto MultiplyChannelColorByScalar(const uint32_t scalar,
                                         const PixelChannelType channelVal) noexcept -> uint32_t
{
  return (scalar * static_cast<uint32_t>(channelVal)) / channel_limits<uint32_t>::max();
}

inline auto operator==(const Pixel& pixel1, const Pixel& pixel2) noexcept -> bool
{
  return pixel1.m_color.intVal == pixel2.m_color.intVal;
}

inline auto Pixel::R() const noexcept -> PixelChannelType
{
  return m_color.channels.r;
}

inline auto Pixel::SetR(const PixelChannelType val) noexcept -> void
{
  m_color.channels.r = val;
}

inline auto Pixel::G() const noexcept -> PixelChannelType
{
  return m_color.channels.g;
}

inline auto Pixel::SetG(const PixelChannelType val) noexcept -> void
{
  m_color.channels.g = val;
}

inline auto Pixel::B() const noexcept -> PixelChannelType
{
  return m_color.channels.b;
}

inline auto Pixel::SetB(const PixelChannelType val) noexcept -> void
{
  m_color.channels.b = val;
}

inline auto Pixel::A() const noexcept -> PixelChannelType
{
  return m_color.channels.a;
}

inline auto Pixel::SetA(const PixelChannelType val) noexcept -> void
{
  m_color.channels.a = val;
}

inline auto Pixel::RFlt() const noexcept -> float
{
  return static_cast<float>(R()) / channel_limits<float>::max();
}

inline auto Pixel::GFlt() const noexcept -> float
{
  return static_cast<float>(G()) / channel_limits<float>::max();
}

inline auto Pixel::BFlt() const noexcept -> float
{
  return static_cast<float>(B()) / channel_limits<float>::max();
}

inline auto Pixel::Rgba() const noexcept -> PixelIntType
{
  return m_color.intVal;
}

inline auto Pixel::IsBlack() const noexcept -> bool
{
  return 0 == m_color.intVal;
}

inline auto Pixel::ToString() const noexcept -> std::string
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

inline auto PixelBuffer::Resize(const size_t width, const size_t height) noexcept -> void
{
  m_width = static_cast<uint32_t>(width);
  m_height = static_cast<uint32_t>(height);
  m_xMax = m_width - 1;
  m_yMax = m_height - 1;
  m_buff.resize(static_cast<size_t>(m_width) * static_cast<size_t>(m_height));
}

inline auto PixelBuffer::GetWidth() const noexcept -> uint32_t
{
  return m_width;
}

inline auto PixelBuffer::GetHeight() const noexcept -> uint32_t
{
  return m_height;
}

inline auto PixelBuffer::Fill(const Pixel& pixel) noexcept -> void
{
  std::fill(m_buff.begin(), m_buff.end(), pixel);
}

inline auto PixelBuffer::GetIntBufferSize(const uint32_t width, const uint32_t height) noexcept
    -> size_t
{
  return static_cast<size_t>(width) * static_cast<size_t>(height) * sizeof(Pixel);
}

inline auto PixelBuffer::GetIntBuff() const noexcept -> const PixelIntType*
{
  return reinterpret_cast<const PixelIntType*>(m_buff.data());
}

inline auto PixelBuffer::CopyTo(PixelBuffer& pixelBuffer) const noexcept -> void
{
  std::copy(cbegin(m_buff), cend(m_buff), begin(pixelBuffer.m_buff));
}

inline auto PixelBuffer::operator()(const size_t x, const size_t y) const noexcept -> const Pixel&
{
#ifdef GOOM_DEBUG
  return m_buff.at((y * m_width) + x);
#else
  return m_buff[(y * m_width) + x];
#endif
}

inline auto PixelBuffer::operator()(const size_t x, const size_t y) noexcept -> Pixel&
{
#ifdef GOOM_DEBUG
  return m_buff.at((y * m_width) + x);
#else
  return m_buff[(y * m_width) + x];
#endif
}

inline auto PixelBuffer::GetRowIter(const size_t y) noexcept
    -> std::tuple<PixelBuffer::iterator, PixelBuffer::iterator>
{
  const auto rowPos = static_cast<int32_t>(y * m_width);
  return std::make_tuple(m_buff.begin() + rowPos,
                         m_buff.begin() + rowPos + static_cast<int32_t>(m_width));
}

inline auto PixelBuffer::GetRowIter(const size_t y) const noexcept
    -> std::tuple<PixelBuffer::const_iterator, PixelBuffer::const_iterator>
{
  const auto rowPos = static_cast<int32_t>(y * m_width);
  return std::make_tuple(m_buff.begin() + rowPos,
                         m_buff.begin() + rowPos + static_cast<int32_t>(m_width));
}

inline auto PixelBuffer::Get4RHBNeighbours(const size_t x, const size_t y) const noexcept
    -> std::array<Pixel, NUM_NBRS>
{
  assert(x < m_xMax);
  assert(y < m_yMax);

  const size_t xPos = (y * m_width) + x;

  // This is a serious hotspot so the following ugly access optimization
  // saves a few milliseconds per update.

  /** Simple, clean code, but slightly slower.
  return {
      {
        m_buff[xPos],
        m_buff[xPos + 1],
        m_buff[xPos + m_width],
        m_buff[xPos + m_width + 1]
      }
  };
   **/

  // Ugly code, but slightly faster.
  struct TwoPixels
  {
    Pixel pixel1;
    Pixel pixel2;
  };
  std::array<Pixel, NUM_NBRS> neighbours{};

  *reinterpret_cast<TwoPixels*>(&(neighbours[0])) =
      *reinterpret_cast<const TwoPixels*>(&(m_buff[xPos]));
  *reinterpret_cast<TwoPixels*>(&(neighbours[2])) =
      *reinterpret_cast<const TwoPixels*>(&(m_buff[xPos + m_width]));

  return neighbours;
}

} // namespace GOOM
