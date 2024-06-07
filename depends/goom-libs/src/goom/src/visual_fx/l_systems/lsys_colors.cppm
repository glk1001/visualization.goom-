module;

#include "color/color_data/color_map_enums.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <vector>

module Goom.VisualFx.LSystemFx:LSysColors;

import Goom.Color.ColorAdjustment;
import Goom.Color.ColorMaps;
import Goom.Color.ColorUtils;
import Goom.Draw.GoomDrawBase;
import Goom.Utils.EnumUtils;
import Goom.Utils.Graphics.PixelUtils;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;

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
  LSysColors(const UTILS::MATH::IGoomRand& goomRand, PixelChannelType defaultAlpha) noexcept;

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

  [[nodiscard]] auto GetCurrentColorTs() const noexcept -> const std::vector<UTILS::MATH::TValue>&;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;

  float m_globalBrightness = 1.0F;
  uint8_t m_lineWidth      = 1U;

  PixelChannelType m_defaultAlpha;
  COLOR::ColorMaps m_colorMaps{m_defaultAlpha};
  static constexpr auto NUM_MAIN_COLORS = 5U;
  static constexpr float MIN_SATURATION = 0.5F;
  static constexpr float MAX_SATURATION = 1.0F;
  static constexpr float MIN_LIGHTNESS  = 0.5F;
  static constexpr float MAX_LIGHTNESS  = 1.0F;
  std::vector<COLOR::ConstColorMapSharedPtr> m_currentMainColorMapList;
  std::vector<COLOR::ConstColorMapSharedPtr> m_currentLowColorMapList;
  std::vector<COLOR::ConstColorMapSharedPtr> m_currentThickerMainColorMapList;
  std::vector<COLOR::ConstColorMapSharedPtr> m_currentThickerLowColorMapList;
  float m_probabilityOfSimpleColors                   = 1.0F;
  std::vector<ColorShadesAndTints> m_simpleColorsList = GetSimpleColorsList(NUM_MAIN_COLORS);
  [[nodiscard]] auto GetSimpleColorsList(uint32_t numColors) const noexcept
      -> std::vector<ColorShadesAndTints>;
  auto SetMainColorMaps() noexcept -> void;
  auto SetLowColorMaps() noexcept -> void;
  auto SetThickerMainColorMaps() noexcept -> void;
  auto SetThickerLowColorMaps() noexcept -> void;
  static auto SetNonMainColorMaps(
      std::vector<COLOR::ConstColorMapSharedPtr>& colorMapsList) noexcept -> void;
  bool m_useSimpleColors = true;
  std::function<Pixel(uint32_t colorNum, float t)> m_simpleColorGet;
  auto SetSimpleColors() noexcept -> void;

  static constexpr auto MIN_NUM_COLOR_STEPS = 10U;
  static constexpr auto MAX_NUM_COLOR_STEPS = 100U;
  std::vector<UTILS::MATH::TValue> m_currentColorTs;
  static constexpr auto MIN_NUM_THICKER_COLOR_STEPS = 10U;
  static constexpr auto MAX_NUM_THICKER_COLOR_STEPS = 1000U;
  uint32_t m_currentMaxNumColorSteps                = MAX_NUM_COLOR_STEPS;
  std::vector<UTILS::MATH::TValue> m_currentThickerColorTs;
  auto ResetColorTs() noexcept -> void;

  static constexpr auto GAMMA = 1.0F / 2.2F;
  COLOR::ColorAdjustment m_colorAdjust{
      {GAMMA, COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };
};

} // namespace GOOM::VISUAL_FX::L_SYSTEM

namespace GOOM::VISUAL_FX::L_SYSTEM
{

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

inline auto LSysColors::GetCurrentColorTs() const noexcept
    -> const std::vector<UTILS::MATH::TValue>&
{
  return m_currentColorTs;
}

inline ColorShadesAndTints::ColorShadesAndTints(const Pixel& baseColor) noexcept
  : m_baseColor{baseColor}
{
}

inline auto ColorShadesAndTints::GetShade(const float t) const noexcept -> Pixel
{
  const auto brightness = std::lerp(DEFAULT_MIN_SHADE, DEFAULT_MAX_SHADE, t);
  return COLOR::GetBrighterColor(brightness, m_baseColor);
}

inline auto ColorShadesAndTints::GetTint(const float t) const noexcept -> Pixel
{
  const auto fromWhite  = Pixel{static_cast<PixelChannelType>(WHITE_PIXEL.R() - m_baseColor.R()),
                               static_cast<PixelChannelType>(WHITE_PIXEL.B() - m_baseColor.G()),
                               static_cast<PixelChannelType>(WHITE_PIXEL.B() - m_baseColor.B()),
                               MAX_ALPHA};
  const auto brightness = std::lerp(DEFAULT_MIN_TINT, DEFAULT_MAX_TINT, t);
  return UTILS::GRAPHICS::GetColorAdd(
      m_baseColor, COLOR::GetBrighterColor(brightness, fromWhite), m_baseColor.A());
}

using COLOR::ColorMaps;
using COLOR::ConstColorMapSharedPtr;
using COLOR::SimpleColors;
using COLOR::COLOR_DATA::ColorMapName;
using DRAW::MakePixels;
using DRAW::MultiplePixels;
using UTILS::MATH::HALF;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::TValue;

using TintProperties = ColorMaps::TintProperties;

LSysColors::LSysColors(const IGoomRand& goomRand, const PixelChannelType defaultAlpha) noexcept
  : m_goomRand{&goomRand}, m_defaultAlpha{defaultAlpha}
{
}

auto LSysColors::GetSimpleColorsList(const uint32_t numColors) const noexcept
    -> std::vector<ColorShadesAndTints>
{
  auto simpleColorsList = std::vector<ColorShadesAndTints>{};

  for (auto i = 0U; i < numColors; ++i)
  {
    simpleColorsList.emplace_back(
        GetSimpleColor(static_cast<SimpleColors>(i % UTILS::NUM<SimpleColors>), m_defaultAlpha));
  }

  return simpleColorsList;
}

auto LSysColors::SetNumColors(const uint32_t numColors) noexcept -> void
{
  Expects(numColors > 0U);

  m_currentMainColorMapList.resize(numColors, ConstColorMapSharedPtr{nullptr});
  m_currentLowColorMapList.resize(numColors, ConstColorMapSharedPtr{nullptr});
  m_currentThickerMainColorMapList.resize(numColors, ConstColorMapSharedPtr{nullptr});
  m_currentThickerLowColorMapList.resize(numColors, ConstColorMapSharedPtr{nullptr});

  m_simpleColorsList = GetSimpleColorsList(numColors);

  const auto numColorSteps =
      m_currentColorTs.empty() ? m_currentMaxNumColorSteps : m_currentColorTs.at(0).GetNumSteps();
  const auto numThickerColorSteps = m_currentThickerColorTs.empty()
                                        ? m_currentMaxNumColorSteps
                                        : m_currentThickerColorTs.at(0).GetNumSteps();
  m_currentColorTs.clear();
  m_currentThickerColorTs.clear();
  for (auto i = 0U; i < numColors; ++i)
  {
    m_currentColorTs.emplace_back(
        TValue::NumStepsProperties{TValue::StepType::CONTINUOUS_REVERSIBLE, numColorSteps});
    m_currentThickerColorTs.emplace_back(
        TValue::NumStepsProperties{TValue::StepType::CONTINUOUS_REVERSIBLE, numThickerColorSteps});
  }
}

auto LSysColors::ChangeColors() noexcept -> void
{
  SetMainColorMaps();
  SetLowColorMaps();
  SetThickerMainColorMaps();
  SetThickerLowColorMaps();

  SetSimpleColors();

  ResetColorTs();
}

auto LSysColors::SetSimpleColors() noexcept -> void
{
  if (not m_goomRand->ProbabilityOf(m_probabilityOfSimpleColors))
  {
    m_useSimpleColors = false;
    return;
  }

  m_useSimpleColors = true;

  if (static constexpr auto PROB_USE_SHADE = 0.5F; m_goomRand->ProbabilityOf(PROB_USE_SHADE))
  {
    m_simpleColorGet = [this](const uint32_t colorNum, const float t)
    { return m_simpleColorsList.at(colorNum).GetShade(t); };
  }
  else
  {
    m_simpleColorGet = [this](const uint32_t colorNum, const float t)
    { return m_simpleColorsList.at(colorNum).GetTint(t); };
  }
}

auto LSysColors::ResetColorTs() noexcept -> void
{
  const auto numColorSteps =
      m_goomRand->GetRandInRange(MIN_NUM_COLOR_STEPS, m_currentMaxNumColorSteps + 1U);
  std::for_each(begin(m_currentColorTs),
                end(m_currentColorTs),
                [&numColorSteps](auto& colorT) { colorT.SetNumSteps(numColorSteps); });
  std::for_each(
      begin(m_currentColorTs), end(m_currentColorTs), [](auto& colorT) { colorT.Reset(0.0F); });

  const auto numThickerColorSteps =
      m_goomRand->GetRandInRange(MIN_NUM_THICKER_COLOR_STEPS, MAX_NUM_THICKER_COLOR_STEPS + 1U);
  std::for_each(begin(m_currentThickerColorTs),
                end(m_currentThickerColorTs),
                [&numThickerColorSteps](auto& colorT)
                { colorT.SetNumSteps(numThickerColorSteps); });
  std::for_each(begin(m_currentThickerColorTs),
                end(m_currentThickerColorTs),
                [](auto& colorT) { colorT.Reset(0.0F); });
}

auto LSysColors::IncrementColorTs() noexcept -> void
{
  std::for_each(
      begin(m_currentColorTs), end(m_currentColorTs), [](auto& colorT) { colorT.Increment(); });
  std::for_each(begin(m_currentThickerColorTs),
                end(m_currentThickerColorTs),
                [](auto& colorT) { colorT.Increment(); });
}

auto LSysColors::GetColors(const uint32_t colorNum, const uint32_t lSysColor) const noexcept
    -> MultiplePixels
{
  static constexpr auto MAIN_BRIGHTNESS = 1.5F;
  static constexpr auto LOW_BRIGHTNESS  = 3.0F;

  const auto colorT         = m_currentColorTs.at(colorNum)();
  const auto mainBrightness = m_globalBrightness * GetBrightness(MAIN_BRIGHTNESS, lSysColor);
  const auto lowBrightness  = m_globalBrightness * GetBrightness(LOW_BRIGHTNESS, lSysColor);
  // TODO(glk) Doesn't inc right colorT?
  const auto colorNumToUse = GetColorNumToUse(colorNum, lSysColor);

  static constexpr auto LINE_WIDTH_CUTOFF = 5U;

  if (m_useSimpleColors and (m_lineWidth <= LINE_WIDTH_CUTOFF))
  {
    const auto color     = m_simpleColorGet(colorNumToUse, colorT);
    const auto mainColor = m_colorAdjust.GetAdjustment(mainBrightness, color);
    const auto lowColor  = m_colorAdjust.GetAdjustment(lowBrightness, color);

    return MakePixels(mainColor, lowColor);
  }

  if (m_lineWidth <= LINE_WIDTH_CUTOFF)
  {
    const auto mainColor = m_colorAdjust.GetAdjustment(
        mainBrightness, m_currentMainColorMapList.at(colorNumToUse)->GetColor(colorT));
    const auto lowColor = m_colorAdjust.GetAdjustment(
        lowBrightness, m_currentLowColorMapList.at(colorNumToUse)->GetColor(colorT));
    return MakePixels(mainColor, lowColor);
  }

  const auto thickerColorT = m_currentThickerColorTs.at(colorNumToUse)();
  const auto mainColor     = m_colorAdjust.GetAdjustment(
      mainBrightness, m_currentThickerMainColorMapList.at(colorNumToUse)->GetColor(thickerColorT));
  const auto lowColor = m_colorAdjust.GetAdjustment(
      lowBrightness, m_currentThickerLowColorMapList.at(colorNumToUse)->GetColor(thickerColorT));
  return MakePixels(mainColor, lowColor);
}

inline auto LSysColors::GetBrightness(const float baseBrightness, const uint32_t lSysColor) noexcept
    -> float
{
  return lSysColor != 1U ? baseBrightness : (HALF * baseBrightness);
}

inline auto LSysColors::GetColorNumToUse(const uint32_t givenColorNum,
                                         const uint32_t lSysColor) const noexcept -> uint32_t
{
  static constexpr auto SKIP_AMOUNT = 2U;
  return 0 == lSysColor ? givenColorNum
                        : ((givenColorNum + (lSysColor + SKIP_AMOUNT)) %
                           static_cast<uint32_t>(m_currentColorTs.size()));
}

auto LSysColors::SetMainColorMaps() noexcept -> void
{
  const auto tintProperties =
      TintProperties{m_goomRand->GetRandInRange(MIN_SATURATION, MAX_SATURATION),
                     m_goomRand->GetRandInRange(MIN_LIGHTNESS, MAX_LIGHTNESS)};

  auto& colorMapsList = m_currentMainColorMapList;
  //Expects(colorMaps.size() >= NUM_MAIN_COLORS);

  colorMapsList.at(0) =
      m_colorMaps.GetTintedColorMapPtr(ColorMapName::RED_BLACK_SKY, tintProperties);

  if (colorMapsList.size() > 1)
  {
    colorMapsList.at(1) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::BLUES, tintProperties);
  }
  if (colorMapsList.size() > 2)
  {
    colorMapsList.at(2) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::GREENS, tintProperties);
  }
  if (colorMapsList.size() > 3)
  {
    colorMapsList.at(3) =
        m_colorMaps.GetTintedColorMapPtr(ColorMapName::YELLOW_BLACK_BLUE, tintProperties);
  }
  if (colorMapsList.size() > 4)
  {
    colorMapsList.at(4) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::ORANGES, tintProperties);
  }

  SetNonMainColorMaps(colorMapsList);
}

auto LSysColors::SetLowColorMaps() noexcept -> void
{
  const auto tintProperties =
      TintProperties{m_goomRand->GetRandInRange(MIN_SATURATION, MAX_SATURATION),
                     m_goomRand->GetRandInRange(MIN_LIGHTNESS, MAX_LIGHTNESS)};

  auto& colorMapsList = m_currentLowColorMapList;
  //Expects(colorMaps.size() >= NUM_MAIN_COLORS);

  colorMapsList.at(0) =
      m_colorMaps.GetTintedColorMapPtr(ColorMapName::RED_BLACK_SKY, tintProperties);

  if (colorMapsList.size() > 1)
  {
    colorMapsList.at(1) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::BLUES, tintProperties);
  }
  if (colorMapsList.size() > 2)
  {
    colorMapsList.at(2) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::GREENS, tintProperties);
  }
  if (colorMapsList.size() > 3)
  {
    colorMapsList.at(3) =
        m_colorMaps.GetTintedColorMapPtr(ColorMapName::YELLOW_BLACK_BLUE, tintProperties);
  }
  if (colorMapsList.size() > 4)
  {
    colorMapsList.at(4) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::ORANGES, tintProperties);
  }

  SetNonMainColorMaps(colorMapsList);
}

auto LSysColors::SetThickerMainColorMaps() noexcept -> void
{
  const auto tintProperties =
      TintProperties{m_goomRand->GetRandInRange(MIN_SATURATION, MAX_SATURATION),
                     m_goomRand->GetRandInRange(MIN_LIGHTNESS, MAX_LIGHTNESS)};

  auto& colorMapsList = m_currentThickerMainColorMapList;
  //Expects(colorMaps.size() >= NUM_MAIN_COLORS);

  colorMapsList.at(0) =
      m_colorMaps.GetTintedColorMapPtr(ColorMapName::BROWNBLUE12_2, tintProperties);

  if (colorMapsList.size() > 1)
  {
    colorMapsList.at(1) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::CORK_10, tintProperties);
  }
  if (colorMapsList.size() > 2)
  {
    colorMapsList.at(2) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::COPPER, tintProperties);
  }
  if (colorMapsList.size() > 3)
  {
    colorMapsList.at(3) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::EARTH_2, tintProperties);
  }
  if (colorMapsList.size() > 4)
  {
    colorMapsList.at(4) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::TWILIGHT, tintProperties);
  }

  SetNonMainColorMaps(colorMapsList);
}

auto LSysColors::SetThickerLowColorMaps() noexcept -> void
{
  const auto tintProperties =
      TintProperties{m_goomRand->GetRandInRange(MIN_SATURATION, MAX_SATURATION),
                     m_goomRand->GetRandInRange(MIN_LIGHTNESS, MAX_LIGHTNESS)};

  auto& colorMapsList = m_currentThickerLowColorMapList;
  //Expects(colorMaps.size() >= NUM_MAIN_COLORS);

  colorMapsList.at(0) =
      m_colorMaps.GetTintedColorMapPtr(ColorMapName::BROWNBLUE12_2, tintProperties);

  if (colorMapsList.size() > 1)
  {
    colorMapsList.at(1) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::CORK_10, tintProperties);
  }
  if (colorMapsList.size() > 2)
  {
    colorMapsList.at(2) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::COPPER, tintProperties);
  }
  if (colorMapsList.size() > 3)
  {
    colorMapsList.at(3) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::EARTH_2, tintProperties);
  }
  if (colorMapsList.size() > 4)
  {
    colorMapsList.at(4) = m_colorMaps.GetTintedColorMapPtr(ColorMapName::TWILIGHT, tintProperties);
  }

  SetNonMainColorMaps(colorMapsList);
}

auto LSysColors::SetNonMainColorMaps(std::vector<ConstColorMapSharedPtr>& colorMapsList) noexcept
    -> void
{
  for (auto i = NUM_MAIN_COLORS; i < colorMapsList.size(); ++i)
  {
    colorMapsList.at(i) = colorMapsList.at(i % NUM_MAIN_COLORS);
  }
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
