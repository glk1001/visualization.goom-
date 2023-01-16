#include "lsys_colors.h"

namespace GOOM::VISUAL_FX::L_SYSTEM
{

using COLOR::ColorMaps;
using COLOR::IColorMap;
using COLOR::SimpleColors;
using COLOR::COLOR_DATA::ColorMapName;
using DRAW::MakePixels;
using DRAW::MultiplePixels;
using UTILS::TValue;
using UTILS::MATH::IGoomRand;

LSysColors::LSysColors(const IGoomRand& goomRand) noexcept : m_goomRand{goomRand}
{
}

LSysColors::~LSysColors() noexcept = default;

auto LSysColors::SetWeightedColorMaps(
    const IVisualFx::WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  Expects(weightedColorMaps.mainColorMaps != nullptr);
  m_mainColorMaps = weightedColorMaps.mainColorMaps;

  Expects(weightedColorMaps.lowColorMaps != nullptr);
  m_lowColorMaps = weightedColorMaps.lowColorMaps;
}

auto LSysColors::GetSimpleColorsList(const uint32_t numColors) noexcept
    -> std::vector<ColorShadesAndTints>
{
  auto simpleColorsList = std::vector<ColorShadesAndTints>{};

  for (auto i = 0U; i < numColors; ++i)
  {
    simpleColorsList.emplace_back(
        GetSimpleColor(static_cast<SimpleColors>(i % UTILS::NUM<SimpleColors>)));
  }

  return simpleColorsList;
}

auto LSysColors::SetNumColors(const uint32_t numColors) noexcept -> void
{
  Expects(numColors > 0U);

  m_currentMainColorMaps.resize(numColors);
  m_currentLowColorMaps.resize(numColors);
  m_currentThickerMainColorMaps.resize(numColors);
  m_currentThickerLowColorMaps.resize(numColors);

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
    m_currentColorTs.emplace_back(TValue::StepType::CONTINUOUS_REVERSIBLE, numColorSteps);
    m_currentThickerColorTs.emplace_back(TValue::StepType::CONTINUOUS_REVERSIBLE,
                                         numThickerColorSteps);
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
  if (not m_goomRand.ProbabilityOf(m_probabilityOfSimpleColors))
  {
    m_useSimpleColors = false;
    return;
  }

  m_useSimpleColors = true;

  if (static constexpr auto PROB_USE_SHADE = 0.5F; m_goomRand.ProbabilityOf(PROB_USE_SHADE))
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
      m_goomRand.GetRandInRange(MIN_NUM_COLOR_STEPS, m_currentMaxNumColorSteps + 1U);
  std::for_each(begin(m_currentColorTs),
                end(m_currentColorTs),
                [&numColorSteps](auto& colorT) { colorT.SetNumSteps(numColorSteps); });
  std::for_each(
      begin(m_currentColorTs), end(m_currentColorTs), [](auto& colorT) { colorT.Reset(0.0F); });

  const auto numThickerColorSteps =
      m_goomRand.GetRandInRange(MIN_NUM_THICKER_COLOR_STEPS, MAX_NUM_THICKER_COLOR_STEPS + 1U);
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

auto LSysColors::GetColors(const uint32_t colorNum,
                           const uint32_t lSysColor,
                           const uint8_t lineWidth) const noexcept -> MultiplePixels
{
  static constexpr auto MAIN_BRIGHTNESS = 1.0F;
  static constexpr auto LOW_BRIGHTNESS  = 2.5F;

  const auto colorT         = m_currentColorTs.at(colorNum)();
  const auto mainBrightness = m_globalBrightness * GetBrightness(MAIN_BRIGHTNESS, lSysColor);
  const auto lowBrightness  = m_globalBrightness * GetBrightness(LOW_BRIGHTNESS, lSysColor);
  // TODO(glk) Doesn't inc right colorT?
  const auto colorNumToUse = GetColorNumToUse(colorNum, lSysColor);

  static constexpr auto LINE_WIDTH_CUTOFF = 5U;

  if (m_useSimpleColors and (lineWidth <= LINE_WIDTH_CUTOFF))
  {
    const auto color     = m_simpleColorGet(colorNumToUse, colorT);
    const auto mainColor = m_colorAdjust.GetAdjustment(mainBrightness, color);
    const auto lowColor  = m_colorAdjust.GetAdjustment(lowBrightness, color);

    return MakePixels(mainColor, lowColor);
  }

  if (lineWidth <= LINE_WIDTH_CUTOFF)
  {
    const auto mainColor = m_colorAdjust.GetAdjustment(
        mainBrightness, m_currentMainColorMaps.at(colorNumToUse)->GetColor(colorT));
    const auto lowColor = m_colorAdjust.GetAdjustment(
        lowBrightness, m_currentLowColorMaps.at(colorNumToUse)->GetColor(colorT));
    return MakePixels(mainColor, lowColor);
  }

  const auto thickerColorT = m_currentThickerColorTs.at(colorNumToUse)();
  const auto mainColor     = m_colorAdjust.GetAdjustment(
      mainBrightness, m_currentThickerMainColorMaps.at(colorNumToUse)->GetColor(thickerColorT));
  const auto lowColor = m_colorAdjust.GetAdjustment(
      lowBrightness, m_currentThickerLowColorMaps.at(colorNumToUse)->GetColor(thickerColorT));
  return MakePixels(mainColor, lowColor);
}

inline auto LSysColors::GetBrightness(const float baseBrightness,
                                      const uint32_t lSysColor) const noexcept -> float
{
  return lSysColor != 1U ? baseBrightness : (0.5F * baseBrightness);
}

inline auto LSysColors::GetColorNumToUse(const uint32_t givenColorNum,
                                         const uint32_t lSysColor) const noexcept -> uint32_t
{
  return lSysColor == 0
             ? givenColorNum
             : (givenColorNum + (lSysColor + 2U)) % static_cast<uint32_t>(m_currentColorTs.size());
}

auto LSysColors::SetMainColorMaps() noexcept -> void
{
  const auto saturation = m_goomRand.GetRandInRange(MIN_SATURATION, MAX_SATURATION);
  const auto lightness  = m_goomRand.GetRandInRange(MIN_LIGHTNESS, MAX_LIGHTNESS);

  auto& colorMaps = m_currentMainColorMaps;
  //Expects(colorMaps.size() >= NUM_MAIN_COLORS);

  colorMaps.at(0) =
      ColorMaps::GetTintedColorMapPtr(ColorMapName::RED_BLACK_SKY, saturation, lightness);

  if (colorMaps.size() > 1)
  {
    colorMaps.at(1) = ColorMaps::GetTintedColorMapPtr(ColorMapName::BLUES, saturation, lightness);
  }
  if (colorMaps.size() > 2)
  {
    colorMaps.at(2) = ColorMaps::GetTintedColorMapPtr(ColorMapName::GREENS, saturation, lightness);
  }
  if (colorMaps.size() > 3)
  {
    colorMaps.at(3) =
        ColorMaps::GetTintedColorMapPtr(ColorMapName::YELLOW_BLACK_BLUE, saturation, lightness);
  }
  if (colorMaps.size() > 4)
  {
    colorMaps.at(4) = ColorMaps::GetTintedColorMapPtr(ColorMapName::ORANGES, saturation, lightness);
  }

  SetNonMainColorMaps(colorMaps);
}

auto LSysColors::SetLowColorMaps() noexcept -> void
{
  const auto saturation = m_goomRand.GetRandInRange(MIN_SATURATION, MAX_SATURATION);
  const auto lightness  = m_goomRand.GetRandInRange(MIN_LIGHTNESS, MAX_LIGHTNESS);

  auto& colorMaps = m_currentLowColorMaps;
  //Expects(colorMaps.size() >= NUM_MAIN_COLORS);

  colorMaps.at(0) =
      ColorMaps::GetTintedColorMapPtr(ColorMapName::RED_BLACK_SKY, saturation, lightness);

  if (colorMaps.size() > 1)
  {
    colorMaps.at(1) = ColorMaps::GetTintedColorMapPtr(ColorMapName::BLUES, saturation, lightness);
  }
  if (colorMaps.size() > 2)
  {
    colorMaps.at(2) = ColorMaps::GetTintedColorMapPtr(ColorMapName::GREENS, saturation, lightness);
  }
  if (colorMaps.size() > 3)
  {
    colorMaps.at(3) =
        ColorMaps::GetTintedColorMapPtr(ColorMapName::YELLOW_BLACK_BLUE, saturation, lightness);
  }
  if (colorMaps.size() > 4)
  {
    colorMaps.at(4) = ColorMaps::GetTintedColorMapPtr(ColorMapName::ORANGES, saturation, lightness);
  }

  SetNonMainColorMaps(colorMaps);
}

auto LSysColors::SetThickerMainColorMaps() noexcept -> void
{
  const auto saturation = m_goomRand.GetRandInRange(MIN_SATURATION, MAX_SATURATION);
  const auto lightness  = m_goomRand.GetRandInRange(MIN_LIGHTNESS, MAX_LIGHTNESS);

  auto& colorMaps = m_currentThickerMainColorMaps;
  //Expects(colorMaps.size() >= NUM_MAIN_COLORS);

  colorMaps.at(0) =
      ColorMaps::GetTintedColorMapPtr(ColorMapName::BROWNBLUE12_2, saturation, lightness);

  if (colorMaps.size() > 1)
  {
    colorMaps.at(1) = ColorMaps::GetTintedColorMapPtr(ColorMapName::CORK_10, saturation, lightness);
  }
  if (colorMaps.size() > 2)
  {
    colorMaps.at(2) = ColorMaps::GetTintedColorMapPtr(ColorMapName::COPPER, saturation, lightness);
  }
  if (colorMaps.size() > 3)
  {
    colorMaps.at(3) = ColorMaps::GetTintedColorMapPtr(ColorMapName::EARTH_2, saturation, lightness);
  }
  if (colorMaps.size() > 4)
  {
    colorMaps.at(4) =
        ColorMaps::GetTintedColorMapPtr(ColorMapName::TWILIGHT, saturation, lightness);
  }

  SetNonMainColorMaps(colorMaps);
}

auto LSysColors::SetThickerLowColorMaps() noexcept -> void
{
  const auto saturation = m_goomRand.GetRandInRange(MIN_SATURATION, MAX_SATURATION);
  const auto lightness  = m_goomRand.GetRandInRange(MIN_LIGHTNESS, MAX_LIGHTNESS);

  auto& colorMaps = m_currentThickerLowColorMaps;
  //Expects(colorMaps.size() >= NUM_MAIN_COLORS);

  colorMaps.at(0) =
      ColorMaps::GetTintedColorMapPtr(ColorMapName::BROWNBLUE12_2, saturation, lightness);

  if (colorMaps.size() > 1)
  {
    colorMaps.at(1) = ColorMaps::GetTintedColorMapPtr(ColorMapName::CORK_10, saturation, lightness);
  }
  if (colorMaps.size() > 2)
  {
    colorMaps.at(2) = ColorMaps::GetTintedColorMapPtr(ColorMapName::COPPER, saturation, lightness);
  }
  if (colorMaps.size() > 3)
  {
    colorMaps.at(3) = ColorMaps::GetTintedColorMapPtr(ColorMapName::EARTH_2, saturation, lightness);
  }
  if (colorMaps.size() > 4)
  {
    colorMaps.at(4) =
        ColorMaps::GetTintedColorMapPtr(ColorMapName::TWILIGHT, saturation, lightness);
  }

  SetNonMainColorMaps(colorMaps);
}

auto LSysColors::SetNonMainColorMaps(
    std::vector<std::shared_ptr<const IColorMap>>& colorMaps) noexcept -> void
{
  for (auto i = NUM_MAIN_COLORS; i < colorMaps.size(); ++i)
  {
    colorMaps.at(i) = colorMaps.at(i % NUM_MAIN_COLORS);
  }
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM