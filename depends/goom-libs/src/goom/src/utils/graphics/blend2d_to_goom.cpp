#include "blend2d_to_goom.h"

#include "draw/goom_draw.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/goom_types.h"

#include <algorithm>
#include <blend2d.h> // NOLINT(misc-include-cleaner): Blend2d insists on this.
#include <blend2d/format.h>
#include <blend2d/image.h>
#include <cstddef>
#include <cstdint>
#include <execution>
#include <span> // NOLINT(misc-include-cleaner): Waiting to use C++20.

namespace GOOM::UTILS::GRAPHICS
{

Blend2dToGoom::Blend2dToGoom(const Dimensions& dimensions,
                             const DRAW::IGoomDraw::PixelBlendFunc& func) noexcept
  : m_pixelBlendFunc{func},
    m_blend2DBuffer{GetNewBlend2DBuffer(dimensions)},
    m_shiftsAndMasks{GetShiftsAndMasks(m_blend2DBuffer.blend2dImage)}
{
}

auto Blend2dToGoom::GetNewBlend2DBuffer(const Dimensions& dimensions) noexcept -> Blend2DBuffer
{
  auto blend2DBuffer = Blend2DBuffer{};

  blend2DBuffer.blend2dImage =
      BLImage{dimensions.GetIntWidth(), dimensions.GetIntHeight(), BL_FORMAT_PRGB32};
  blend2DBuffer.blend2dContext = BLContext{blend2DBuffer.blend2dImage};

  return blend2DBuffer;
}

[[nodiscard]] auto Blend2dToGoom::GetShiftsAndMasks(const BLFormatInfo& formatInfo) noexcept
    -> ShiftsAndMasks
{
  const auto rShift = static_cast<uint32_t>(formatInfo.rShift); // NOLINT: union hard to fix
  const auto gShift = static_cast<uint32_t>(formatInfo.gShift); // NOLINT: union hard to fix
  const auto bShift = static_cast<uint32_t>(formatInfo.bShift); // NOLINT: union hard to fix
  const auto aShift = static_cast<uint32_t>(formatInfo.aShift); // NOLINT: union hard to fix

  return {
      /*.rShift = */ rShift,
      /*.gShift = */ gShift,
      /*.bShift = */ bShift,
      /*.aShift = */ aShift,
      /*.rMask  = */ (MAX_BLEND2D_CHANNEL - 1) << rShift,
      /*.gMask  = */ (MAX_BLEND2D_CHANNEL - 1) << gShift,
      /*.bMask  = */ (MAX_BLEND2D_CHANNEL - 1) << bShift,
      /*.aMask  = */ (MAX_BLEND2D_CHANNEL - 1) << aShift,
  };
}

[[nodiscard]] auto Blend2dToGoom::GetShiftsAndMasks(const BLImage& blImage) noexcept
    -> ShiftsAndMasks
{
  auto srceFormatInfo = BLFormatInfo{};
  srceFormatInfo.query(blImage.format());

  return GetShiftsAndMasks(srceFormatInfo);
}

[[nodiscard]] auto Blend2dToGoom::GetBlend2dColor(const Pixel& pixel) -> uint32_t
{
  union Color
  {
    struct Argb
    {
      uint8_t blue;
      uint8_t green;
      uint8_t red;
      uint8_t alpha;
    } argb;
    uint32_t value;
  };

  auto color = Color{};

  static constexpr auto DIVISOR = 16U;
  // NOLINTBEGIN: cppcoreguidelines-pro-type-union-access: union hard to fix
  color.argb.alpha = static_cast<uint8_t>(pixel.A() / DIVISOR);
  color.argb.red   = static_cast<uint8_t>(pixel.R() / DIVISOR);
  color.argb.green = static_cast<uint8_t>(pixel.G() / DIVISOR);
  color.argb.blue  = static_cast<uint8_t>(pixel.B() / DIVISOR);
  // NOLINTEND: cppcoreguidelines-pro-type-union-access

  return color.value; // NOLINT: union hard to fix
}

[[nodiscard]] auto Blend2dToGoom::GetGoomPixel(const uint32_t blend2dColor) const noexcept -> Pixel
{
  return Pixel{
      static_cast<GOOM::PixelChannelType>(
          CHANNEL_MULTIPLIER *
          ((blend2dColor & m_shiftsAndMasks.rMask) >> m_shiftsAndMasks.rShift)),
      static_cast<GOOM::PixelChannelType>(
          CHANNEL_MULTIPLIER *
          ((blend2dColor & m_shiftsAndMasks.gMask) >> m_shiftsAndMasks.gShift)),
      static_cast<GOOM::PixelChannelType>(
          CHANNEL_MULTIPLIER *
          ((blend2dColor & m_shiftsAndMasks.bMask) >> m_shiftsAndMasks.bShift)),
      MAX_ALPHA,
      //      static_cast<GOOM::PixelChannelType>(
      //          CHANNEL_MULTIPLIER *
      //          ((blend2dColor & m_shiftsAndMasks.aMask) >> m_shiftsAndMasks.aShift)),
  };
}

auto Blend2dToGoom::UpdateGoomBuffer(PixelBuffer& goomBuffer) noexcept -> void
{
  Expects(m_pixelBlendFunc != nullptr);

  const auto blendPixel = [this](const uint32_t color, const Pixel& pixel) -> Pixel
  { return GetBlendedPixel(color, pixel); };

  const auto srceBuffer = GetPixelBuffer(m_blend2DBuffer.blend2dImage);

  std::transform(std::execution::par_unseq,
                 srceBuffer.begin(),
                 srceBuffer.end(),
                 goomBuffer.GetPixelBuffer().begin(),
                 goomBuffer.GetPixelBuffer().begin(),
                 blendPixel);
}

[[nodiscard]] auto Blend2dToGoom::GetBlendedPixel(const uint32_t blend2dColor,
                                                  const Pixel& pixel) const noexcept -> Pixel
{
  if (static constexpr auto BLACK_CUTOFF = 5U; blend2dColor < BLACK_CUTOFF)
  {
    return pixel;
  }

  const auto blend2Pixel = GetGoomPixel(blend2dColor);

  return m_pixelBlendFunc(pixel, m_intBuffIntensity, blend2Pixel, m_newAlpha);
}

[[nodiscard]] auto Blend2dToGoom::GetImageData(const BLImage& blImage) noexcept -> BLImageData
{
  auto imageData = BLImageData{};
  blImage.getData(&imageData);
  return imageData;
}

[[nodiscard]] auto Blend2dToGoom::GetPixelBuffer(const BLImage& blImage) noexcept
    -> std_spn::span<const uint32_t> // NOLINT(misc-include-cleaner): Waiting for C++20.
{
  const auto bufferSize =
      static_cast<size_t>(blImage.size().w) * static_cast<size_t>(blImage.size().h);

  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  return std_spn::span{static_cast<const uint32_t*>(GetImageData(blImage).pixelData), bufferSize};
}

} // namespace GOOM::UTILS::GRAPHICS
