#pragma once

#include "filter_fx/common_types.h"
#include "filter_fx/normalized_coords.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/point2d.h"

#include <cstdint>
#include <memory>
#include <string>

import Goom.Utils.Graphics.ImageBitmaps;
import Goom.Utils.Math.GoomRandBase;

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

  [[nodiscard]] auto GetXZoomFactor() const noexcept -> float;
  [[nodiscard]] auto GetYZoomFactor() const noexcept -> float;
  auto SetZoomFactor(float xZoomFactor, float yZoomFactor) noexcept -> void;

  [[nodiscard]] auto GetAmplitude() const noexcept -> const Amplitude&;
  auto SetAmplitude(const Amplitude& value) noexcept -> void;

  [[nodiscard]] auto GetDisplacementVector(const NormalizedCoords& normalizedCoords) const noexcept
      -> Vec2dFlt;

private:
  std::unique_ptr<UTILS::GRAPHICS::ImageBitmap> m_imageBuffer;
  std::string m_imageFilename;
  int32_t m_xMax = static_cast<int32_t>(m_imageBuffer->GetWidth() - 1);
  int32_t m_yMax = static_cast<int32_t>(m_imageBuffer->GetHeight() - 1);
  NormalizedCoordsConverter m_normalizedCoordsConverter{
      {m_imageBuffer->GetWidth(), m_imageBuffer->GetHeight()},
      false
  };
  float m_xZoomFactor                   = 1.0F;
  float m_yZoomFactor                   = 1.0F;
  Amplitude m_amplitude                 = {1.0F, 1.0F};
  static constexpr float INITIAL_CUTOFF = 0.5F;
  float m_xColorCutoff                  = INITIAL_CUTOFF;
  float m_yColorCutoff                  = INITIAL_CUTOFF;
  [[nodiscard]] auto NormalizedCoordsToImagePoint(
      const NormalizedCoords& normalizedCoords) const noexcept -> Point2dInt;
  [[nodiscard]] auto ColorToNormalizedDisplacement(const Pixel& color) const noexcept -> Vec2dFlt;
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

inline auto ImageDisplacement::GetXZoomFactor() const noexcept -> float
{
  return m_xZoomFactor;
}

inline auto ImageDisplacement::GetYZoomFactor() const noexcept -> float
{
  return m_yZoomFactor;
}

inline auto ImageDisplacement::SetZoomFactor(const float xZoomFactor,
                                             const float yZoomFactor) noexcept -> void
{
  Expects(xZoomFactor > 0.0F);
  Expects(yZoomFactor > 0.0F);
  m_xZoomFactor = xZoomFactor;
  m_yZoomFactor = yZoomFactor;
}

inline auto ImageDisplacement::GetAmplitude() const noexcept -> const Amplitude&
{
  return m_amplitude;
}

inline auto ImageDisplacement::SetAmplitude(const Amplitude& value) noexcept -> void
{
  Expects(value.x > 0.0F);
  Expects(value.y > 0.0F);
  m_amplitude = value;
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
