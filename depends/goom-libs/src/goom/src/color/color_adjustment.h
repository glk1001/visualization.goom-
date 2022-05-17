#pragma once

#include "goom_graphic.h"
#include "utils/math/misc.h"

namespace GOOM::COLOR
{

class ColorAdjustment
{
public:
  ColorAdjustment(float gamma, float alterChromaFactor = 1.0F);

  [[nodiscard]] auto GetIgnoreThreshold() const -> float;
  auto SetIgnoreThreshold(float val) -> void;

  [[nodiscard]] auto GetGamma() const -> float;
  auto SetGamma(float val) -> void;

  [[nodiscard]] auto GetAlterChromaFactor() const -> float;
  auto SetAlterChromaFactor(float val) -> void;

  [[nodiscard]] auto GetAdjustment(float brightness, const Pixel& color) const -> Pixel;

  static constexpr float INCREASED_CHROMA_FACTOR = 2.0F;
  static constexpr float DECREASED_CHROMA_FACTOR = 0.5F;
  [[nodiscard]] static auto GetAlteredChroma(float lchYFactor, const Pixel& color) -> Pixel;
  [[nodiscard]] static auto GetIncreasedChroma(const Pixel& color) -> Pixel;
  [[nodiscard]] static auto GetDecreasedChroma(const Pixel& color) -> Pixel;

private:
  float m_gamma;
  float m_alterChromaFactor;
  bool m_doAlterChroma = not UTILS::MATH::FloatsEqual(1.0F, m_alterChromaFactor);
  static constexpr float DEFAULT_GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  float m_ignoreThreshold = DEFAULT_GAMMA_BRIGHTNESS_THRESHOLD;
};

inline ColorAdjustment::ColorAdjustment(const float gamma, const float alterChromaFactor)
  : m_gamma{gamma}, m_alterChromaFactor{alterChromaFactor}
{
}

inline auto ColorAdjustment::GetIgnoreThreshold() const -> float
{
  return m_ignoreThreshold;
}

inline auto ColorAdjustment::SetIgnoreThreshold(const float val) -> void
{
  m_ignoreThreshold = val;
}

inline auto ColorAdjustment::GetGamma() const -> float
{
  return m_gamma;
}

inline auto ColorAdjustment::SetGamma(const float val) -> void
{
  m_gamma = val;
}

inline auto ColorAdjustment::GetAlterChromaFactor() const -> float
{
  return m_alterChromaFactor;
}

inline auto ColorAdjustment::SetAlterChromaFactor(const float val) -> void
{
  m_alterChromaFactor = val;
  m_doAlterChroma = not UTILS::MATH::FloatsEqual(1.0F, m_alterChromaFactor);
}

inline auto ColorAdjustment::GetIncreasedChroma(const Pixel& color) -> Pixel
{
  return GetAlteredChroma(INCREASED_CHROMA_FACTOR, color);
}

inline auto ColorAdjustment::GetDecreasedChroma(const Pixel& color) -> Pixel
{
  return GetAlteredChroma(DECREASED_CHROMA_FACTOR, color);
}

} // namespace GOOM::COLOR
