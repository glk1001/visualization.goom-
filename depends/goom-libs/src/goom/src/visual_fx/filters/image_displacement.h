#pragma once

#include "normalized_coords.h"
#include "point2d.h"
#include "utils/goom_rand_base.h"
#include "utils/graphics/image_bitmaps.h"

#include <memory>
#include <stdexcept>

namespace GOOM::VISUAL_FX::FILTERS
{

class ImageDisplacement
{
public:
  explicit ImageDisplacement(const std::string& imageFilename, const UTILS::IGoomRand& goomRand);

  [[nodiscard]] auto GetImageFilename() const -> std::string;
  [[nodiscard]] auto GetXColorCutoff() const -> float;
  [[nodiscard]] auto GetYColorCutoff() const -> float;
  void SetXyColorCutoffs(float xColorCutoff, float yColorCutoff);

  [[nodiscard]] auto GetZoomFactor() const -> float;
  void SetZoomFactor(float value);

  [[nodiscard]] auto GetAmplitude() const -> float;
  void SetAmplitude(float value);

  [[nodiscard]] auto GetDisplacementVector(const NormalizedCoords& normalizedCoords) const
      -> Point2dFlt;

private:
  std::unique_ptr<UTILS::ImageBitmap> m_imageBuffer;
  const std::string m_imageFilename;
  const int32_t m_xMax;
  const int32_t m_yMax;
  const float m_ratioNormalizedCoordToImageCoord;
  float m_zoomFactor = 1.0F;
  float m_amplitude = 1.0F;
  static constexpr float INITIAL_CUTOFF = 0.5F;
  float m_xColorCutoff = INITIAL_CUTOFF;
  float m_yColorCutoff = INITIAL_CUTOFF;
  [[nodiscard]] auto NormalizedCoordsToImagePoint(const NormalizedCoords& normalizedCoords) const
      -> Point2dInt;
  [[nodiscard]] auto ColorToNormalizedDisplacement(const Pixel& color) const -> Point2dFlt;
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

} // namespace GOOM::VISUAL_FX::FILTERS
