#pragma once

#include "filter_fx/normalized_coords.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "point2d.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/math/goom_rand_base.h"

#include <cstdint>
#include <memory>
#include <string>

namespace GOOM::FILTER_FX::FILTER_UTILS
{

class ImageDisplacement
{
public:
  ImageDisplacement(const std::string& imageFilename, const UTILS::MATH::IGoomRand& goomRand);

  [[nodiscard]] auto GetImageFilename() const noexcept -> std::string;
  [[nodiscard]] auto GetXColorCutoff() const noexcept -> float;
  [[nodiscard]] auto GetYColorCutoff() const noexcept -> float;
  auto SetXyColorCutoffs(float xColorCutoff, float yColorCutoff) noexcept -> void;

  [[nodiscard]] auto GetZoomFactor() const noexcept -> float;
  auto SetZoomFactor(float value) noexcept -> void;

  [[nodiscard]] auto GetAmplitude() const noexcept -> float;
  auto SetAmplitude(float value) noexcept -> void;

  [[nodiscard]] auto GetDisplacementVector(const NormalizedCoords& normalizedCoords) const noexcept
      -> Point2dFlt;

private:
  std::unique_ptr<UTILS::GRAPHICS::ImageBitmap> m_imageBuffer;
  std::string m_imageFilename;
  int32_t m_xMax = static_cast<int32_t>(m_imageBuffer->GetWidth() - 1);
  int32_t m_yMax = static_cast<int32_t>(m_imageBuffer->GetHeight() - 1);
  NormalizedCoordsConverter m_normalizedCoordsConverter{
      {m_imageBuffer->GetWidth(), m_imageBuffer->GetHeight()},
      false
  };
  float m_zoomFactor                    = 1.0F;
  float m_amplitude                     = 1.0F;
  static constexpr float INITIAL_CUTOFF = 0.5F;
  float m_xColorCutoff                  = INITIAL_CUTOFF;
  float m_yColorCutoff                  = INITIAL_CUTOFF;
  [[nodiscard]] auto NormalizedCoordsToImagePoint(
      const NormalizedCoords& normalizedCoords) const noexcept -> Point2dInt;
  [[nodiscard]] auto ColorToNormalizedDisplacement(const Pixel& color) const noexcept -> Point2dFlt;
};

inline auto ImageDisplacement::GetImageFilename() const noexcept -> std::string
{
  return m_imageFilename;
}

inline auto ImageDisplacement::GetXColorCutoff() const noexcept -> float
{
  return m_xColorCutoff;
}

inline auto ImageDisplacement::GetYColorCutoff() const noexcept -> float
{
  return m_yColorCutoff;
}

inline auto ImageDisplacement::SetXyColorCutoffs(const float xColorCutoff,
                                                 const float yColorCutoff) noexcept -> void
{
  m_xColorCutoff = xColorCutoff;
  m_yColorCutoff = yColorCutoff;
}

inline auto ImageDisplacement::GetZoomFactor() const noexcept -> float
{
  return m_zoomFactor;
}

inline auto ImageDisplacement::SetZoomFactor(const float value) noexcept -> void
{
  Expects(value > 0.0F);
  m_zoomFactor = value;
}

inline auto ImageDisplacement::GetAmplitude() const noexcept -> float
{
  return m_amplitude;
}

inline auto ImageDisplacement::SetAmplitude(const float value) noexcept -> void
{
  Expects(value > 0.0F);
  m_amplitude = value;
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
