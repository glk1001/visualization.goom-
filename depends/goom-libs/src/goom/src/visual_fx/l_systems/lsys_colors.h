#pragma once

#include "../goom_visual_fx.h"
#include "color/color_adjustment.h"
#include "color/color_maps.h"
#include "color/color_utils.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "utils/graphics/pixel_utils.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/t_values.h"

namespace GOOM::VISUAL_FX::L_SYSTEM
{

class ColorShadesAndTints
{
public:
  explicit ColorShadesAndTints(const Pixel& baseColor) noexcept;

  [[nodiscard]] auto GetShade(float t) const noexcept -> Pixel;
  [[nodiscard]] auto GetTint(float t) const noexcept -> Pixel;

private:
  static constexpr auto DEFAULT_MIN_SHADE = 0.3F;
  static constexpr auto DEFAULT_MAX_SHADE = 1.0F;
  static constexpr auto DEFAULT_MIN_TINT  = 0.3F;
  static constexpr auto DEFAULT_MAX_TINT  = 1.0F;
  Pixel m_baseColor;
};

class LSysColors
{
public:
  explicit LSysColors(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetNumColors(uint32_t numColors) noexcept -> void;
  auto SetMaxNumColorSteps(uint32_t numSteps) noexcept -> void;
  auto SetProbabilityOfSimpleColors(float probability) noexcept -> void;
  auto SetGlobalBrightness(float globalBrightness) noexcept -> void;
  auto ChangeColors() noexcept -> void;
  auto SetLineWidth(uint8_t lineWidth) noexcept -> void;

  [[nodiscard]] auto GetColors(uint32_t colorNum, uint32_t lSysColor) const noexcept
      -> DRAW::MultiplePixels;
  [[nodiscard]] static auto GetBrightness(float baseBrightness, uint32_t lSysColor) noexcept
      -> float;
  [[nodiscard]] auto GetColorNumToUse(uint32_t givenColorNum, uint32_t lSysColor) const noexcept
      -> uint32_t;
  auto IncrementColorTs() noexcept -> void;

  [[nodiscard]] auto GetCurrentColorTs() const noexcept -> const std::vector<UTILS::TValue>&;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;

  float m_globalBrightness = 1.0F;
  uint8_t m_lineWidth      = 1U;

  static constexpr auto NUM_MAIN_COLORS = 5U;
  static constexpr float MIN_SATURATION = 0.5F;
  static constexpr float MAX_SATURATION = 1.0F;
  static constexpr float MIN_LIGHTNESS  = 0.5F;
  static constexpr float MAX_LIGHTNESS  = 1.0F;
  std::vector<std::shared_ptr<const COLOR::IColorMap>> m_currentMainColorMaps{};
  std::vector<std::shared_ptr<const COLOR::IColorMap>> m_currentLowColorMaps{};
  std::vector<std::shared_ptr<const COLOR::IColorMap>> m_currentThickerMainColorMaps{};
  std::vector<std::shared_ptr<const COLOR::IColorMap>> m_currentThickerLowColorMaps{};
  float m_probabilityOfSimpleColors                   = 1.0F;
  std::vector<ColorShadesAndTints> m_simpleColorsList = GetSimpleColorsList(NUM_MAIN_COLORS);
  [[nodiscard]] static auto GetSimpleColorsList(uint32_t numColors) noexcept
      -> std::vector<ColorShadesAndTints>;
  auto SetMainColorMaps() noexcept -> void;
  auto SetLowColorMaps() noexcept -> void;
  auto SetThickerMainColorMaps() noexcept -> void;
  auto SetThickerLowColorMaps() noexcept -> void;
  static auto SetNonMainColorMaps(
      std::vector<std::shared_ptr<const COLOR::IColorMap>>& colorMaps) noexcept -> void;
  bool m_useSimpleColors = true;
  std::function<Pixel(uint32_t colorNum, float t)> m_simpleColorGet{};
  auto SetSimpleColors() noexcept -> void;
  static constexpr auto MIN_NUM_COLOR_STEPS = 10U;
  static constexpr auto MAX_NUM_COLOR_STEPS = 100U;
  std::vector<UTILS::TValue> m_currentColorTs{};
  static constexpr auto MIN_NUM_THICKER_COLOR_STEPS = 10U;
  static constexpr auto MAX_NUM_THICKER_COLOR_STEPS = 1000U;
  uint32_t m_currentMaxNumColorSteps                = MAX_NUM_COLOR_STEPS;
  std::vector<UTILS::TValue> m_currentThickerColorTs{};
  auto ResetColorTs() noexcept -> void;
  static constexpr auto GAMMA = 1.0F / 2.2F;
  COLOR::ColorAdjustment m_colorAdjust{
      {GAMMA, COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };
};

inline auto LSysColors::SetMaxNumColorSteps(const uint32_t numSteps) noexcept -> void
{
  m_currentMaxNumColorSteps = numSteps;
}

inline auto LSysColors::SetProbabilityOfSimpleColors(const float probability) noexcept -> void
{
  m_probabilityOfSimpleColors = probability;
}

inline auto LSysColors::SetGlobalBrightness(const float globalBrightness) noexcept -> void
{
  m_globalBrightness = globalBrightness;
}

inline auto LSysColors::SetLineWidth(const uint8_t lineWidth) noexcept -> void
{
  m_lineWidth = lineWidth;
}

inline auto LSysColors::GetCurrentColorTs() const noexcept -> const std::vector<UTILS::TValue>&
{
  return m_currentColorTs;
}

inline ColorShadesAndTints::ColorShadesAndTints(const Pixel& baseColor) noexcept
  : m_baseColor{baseColor}
{
}

inline auto ColorShadesAndTints::GetShade(const float t) const noexcept -> Pixel
{
  const auto brightness = STD20::lerp(DEFAULT_MIN_SHADE, DEFAULT_MAX_SHADE, t);
  return COLOR::GetBrighterColor(brightness, m_baseColor);
}

inline auto ColorShadesAndTints::GetTint(const float t) const noexcept -> Pixel
{
  const auto fromWhite  = Pixel{static_cast<PixelChannelType>(WHITE_PIXEL.R() - m_baseColor.R()),
                               static_cast<PixelChannelType>(WHITE_PIXEL.B() - m_baseColor.G()),
                               static_cast<PixelChannelType>(WHITE_PIXEL.B() - m_baseColor.B()),
                               MAX_ALPHA};
  const auto brightness = STD20::lerp(DEFAULT_MIN_TINT, DEFAULT_MAX_TINT, t);
  return UTILS::GRAPHICS::GetColorAdd(m_baseColor, COLOR::GetBrighterColor(brightness, fromWhite));
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
