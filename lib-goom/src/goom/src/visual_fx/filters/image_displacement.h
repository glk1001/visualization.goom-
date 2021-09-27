#pragma once

#include "goomutils/graphics/image_bitmaps.h"
#include "v2d.h"

#include <memory>
#include <stdexcept>
#include <string>

namespace GOOM
{
namespace FILTERS
{

class ImageDisplacement
{
public:
  explicit ImageDisplacement(const std::string& imageFilename);

  auto GetImageFilename() const -> std::string;
  auto GetXColorCutoff() const -> float;
  auto GetYColorCutoff() const -> float;
  void SetXyColorCutoffs(float xColorCutoff, float yColorCutoff);

  auto GetZoomFactor() const -> float;
  void SetZoomFactor(float value);

  auto GetAmplitude() const -> float;
  void SetAmplitude(float value);

  auto GetDisplacementVector(const V2dFlt& normalizedPoint) const -> V2dFlt;

private:
  std::unique_ptr<UTILS::ImageBitmap> m_imageBuffer{};
  const std::string m_imageFilename;
  const int32_t m_xMax;
  const int32_t m_yMax;
  const float m_ratioNormalizedCoordToImageCoord;
  float m_zoomFactor = 1.0F;
  float m_amplitude = 1.0F;
  static constexpr float INITIAL_CUTOFF = 0.5F;
  float m_xColorCutoff = INITIAL_CUTOFF;
  float m_yColorCutoff = INITIAL_CUTOFF;
  auto NormalizedToImagePoint(const V2dFlt& normalizedPoint) const -> V2dInt;
};

inline auto ImageDisplacement::GetImageFilename() const -> std::string
{
  return m_imageFilename;
}

inline auto ImageDisplacement::GetXColorCutoff() const -> float
{
  return m_xColorCutoff;
}

inline auto ImageDisplacement::GetYColorCutoff() const -> float
{
  return m_yColorCutoff;
}

inline void ImageDisplacement::SetXyColorCutoffs(const float xColorCutoff, const float yColorCutoff)
{
  m_xColorCutoff = xColorCutoff;
  m_yColorCutoff = yColorCutoff;
}

inline auto ImageDisplacement::GetZoomFactor() const -> float
{
  return m_zoomFactor;
}

inline void ImageDisplacement::SetZoomFactor(const float value)
{
  if (value <= 0.0F)
  {
    throw std::logic_error("Negative zoom factor.");
  }
  m_zoomFactor = value;
}

inline auto ImageDisplacement::GetAmplitude() const -> float
{
  return m_amplitude;
}

inline void ImageDisplacement::SetAmplitude(const float value)
{
  if (value <= 0.0F)
  {
    throw std::logic_error("Negative amplitude.");
  }
  m_amplitude = value;
}

} // namespace FILTERS
} // namespace GOOM

