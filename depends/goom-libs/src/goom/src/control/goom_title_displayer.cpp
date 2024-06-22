module;

//#undef NO_LOGGING

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

module Goom.Control.GoomTitleDisplayer;

import Goom.Color.RandomColorMaps;
import Goom.Color.ColorMaps;
import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.TextDrawer;
import Goom.Utils.Math.Misc;
import Goom.Utils.Text.DrawableText;
import Goom.Lib.GoomConfigPaths;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomPaths;
import Goom.Lib.GoomTypes;
import Goom.Lib.GoomUtils;
import Goom.Lib.Point2d;

namespace GOOM::CONTROL
{

using COLOR::ColorMaps;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using DRAW::SHAPE_DRAWERS::TextDrawer;
using UTILS::MATH::GetFltFraction;
using UTILS::MATH::I_HALF;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::NumberRange;
using UTILS::TEXT::GetLeftAlignedPenForCentringStringAt;
using UTILS::TEXT::GetLinesOfWords;

static constexpr auto INITIAL_PHASE_T_MIX   = 0.1F;
static constexpr auto BASE_BRIGHTNESS       = 10.0F;
static constexpr auto MAX_BRIGHTNESS_FACTOR = 2.0F;

static constexpr auto MAX_LINE_LENGTH                     = 40U;
static constexpr auto FONT_SIZE_FRACTION_OF_SCREEN_HEIGHT = 0.05F;
static constexpr auto MAX_FONT_SIZE_MULTIPLIER            = 10.0F;
static constexpr auto SPACE_FACTOR                        = 0.056F;
static constexpr auto OUTLINE_FONT_WIDTH                  = 4;

// To normalize: turn on logging in TextDraw, get width of prepared text for a
// sample text for each font, then normalize with 'verdana' as 1.0.
//

struct FontInfo
{
  const char* fontFilename;
  float fontSizeNormalizeFactor;
};

static constexpr auto FONT_INFO = std::array{
    FontInfo{  "AeroviasBrasilNF.ttf", 1.34F},
    FontInfo{ "AlexBrush-Regular.ttf", 1.25F},
    FontInfo{   "AvenueX-Regular.otf", 1.01F},
    FontInfo{        "CelticHand.ttf", 0.99F},
    FontInfo{         "CheapSign.ttf", 1.26F},
    FontInfo{         "EatAtJoes.ttf", 0.90F},
    FontInfo{"GreatVibes-Regular.ttf", 1.29F},
    FontInfo{          "KellsFLF.ttf", 1.23F},
    FontInfo{     "Rubik-Regular.ttf",  1.1F},
    FontInfo{           "verdana.ttf",  1.0F},
};

static constexpr auto FONT_INFO_INDEX_RANGE =
    NumberRange{0U, static_cast<uint32_t>(FONT_INFO.size()) - 1};

inline auto GoomTitleDisplayer::GetSelectedFontPath() const -> std::string
{
  return join_paths(m_fontDirectory, FONT_INFO.at(m_fontInfoIndex).fontFilename);
}

inline auto GoomTitleDisplayer::GetSelectedFontSize() const -> int32_t
{
  const auto& fontInfo   = FONT_INFO.at(m_fontInfoIndex);
  const auto maxFontSize = static_cast<int32_t>(
      (FONT_SIZE_FRACTION_OF_SCREEN_HEIGHT * static_cast<float>(m_screenHeight)) *
      fontInfo.fontSizeNormalizeFactor);
  return maxFontSize;
}

GoomTitleDisplayer::GoomTitleDisplayer(IGoomDraw& draw,
                                       const IGoomRand& goomRand,
                                       const std::string& fontDirectory)
  : m_goomRand{&goomRand},
    m_textDrawer{std::make_unique<TextDrawer>(draw)},
    m_screenWidth{draw.GetDimensions().GetIntWidth()},
    m_screenHeight{draw.GetDimensions().GetIntHeight()},
    m_fontDirectory{fontDirectory},
    m_fontInfoIndex{m_goomRand->GetRandInRange(FONT_INFO_INDEX_RANGE)}
{
  m_textDrawer->SetFontFile(GetSelectedFontPath());
  m_textDrawer->SetFontSize(GetSelectedFontSize());
  m_textDrawer->SetOutlineWidth(OUTLINE_FONT_WIDTH);
  m_textDrawer->SetAlignment(TextDrawer::TextAlignment::LEFT);
}

void GoomTitleDisplayer::DrawMovingText(const std::string& title)
{
  --m_timeLeftOfTitleDisplay;

  UpdatePositionIncrements(title);
  UpdateColorMaps();
  UpdateFontSize();
  UpdateTextPosition();
  DrawText(title);
}

inline void GoomTitleDisplayer::UpdatePositionIncrements(const std::string& title)
{
  if (m_timeLeftOfTitleDisplay == TIME_TO_START_FINAL_PHASE)
  {
    m_finalPhaseIncrements = GetFinalPhaseIncrements(title.substr(0, 1));
  }
}

inline void GoomTitleDisplayer::UpdateColorMaps()
{
  if (m_timeLeftOfTitleDisplay == TIME_TO_START_FINAL_PHASE)
  {
    SetFinalPhaseColorMaps();
  }
}

inline void GoomTitleDisplayer::UpdateFontSize()
{
  if (m_timeLeftOfTitleDisplay < 0)
  {
    return;
  }

  if (m_timeLeftOfTitleDisplay < TIME_TO_START_FINAL_PHASE)
  {
    m_textDrawer->SetFontSize(GetFinalPhaseFontSize(m_timeLeftOfTitleDisplay));
  }
}

inline void GoomTitleDisplayer::UpdateTextPosition()
{
  if (m_timeLeftOfTitleDisplay < 0)
  {
    return;
  }

  m_xPos += GetXIncrement();
  m_yPos += GetYIncrement();
}

inline void GoomTitleDisplayer::SetFinalPhaseColorMaps()
{
  const auto randomColorMaps = RandomColorMaps{DEFAULT_ALPHA, *m_goomRand};
  m_textColorMap        = randomColorMaps.GetRandomColorMap(COLOR::ColorMapGroup::DIVERGING_BLACK);
  m_textOutlineColorMap = randomColorMaps.GetRandomColorMap(COLOR::ColorMapGroup::HEAT);
  m_charColorMap        = randomColorMaps.GetRandomColorMap(COLOR::ColorMapGroup::DIVERGING_BLACK);
}

inline auto GoomTitleDisplayer::GetFinalPhaseFontSize(const int32_t timeLeftOfTitleDisplay) const
    -> int32_t
{
  const auto fractionOfTimeLeft =
      GetFltFraction<int32_t>({timeLeftOfTitleDisplay, TIME_TO_START_FINAL_PHASE});
  const auto t                  = 1.0F - std::pow(fractionOfTimeLeft, 0.7F);
  const auto fontSizeMultiplier = std::lerp(1.0F, MAX_FONT_SIZE_MULTIPLIER, t);
  return static_cast<int32_t>(fontSizeMultiplier * static_cast<float>(GetSelectedFontSize()));
}

auto GoomTitleDisplayer::GetFinalPhaseIncrements(const std::string& title) -> FinalPhaseIncrements
{
  const auto [finalXPos, finalYPos] = GetFinalPhaseCentrePenPos(title);
  return {(finalXPos - m_xPos) / static_cast<float>(TIME_TO_START_FINAL_PHASE),
          (finalYPos - m_yPos) / static_cast<float>(TIME_TO_START_FINAL_PHASE)};
}

inline auto GoomTitleDisplayer::GetFinalPhaseCentrePenPos(const std::string& str) -> Point2dFlt
{
  const auto screenCentre = Point2dInt{I_HALF * m_screenWidth, I_HALF * m_screenHeight};
  const auto fontSize     = GetFinalPhaseFontSize(0);
  return ToPoint2dFlt(
      GetLeftAlignedPenForCentringStringAt(*m_textDrawer, str, fontSize, screenCentre));
}

void GoomTitleDisplayer::DrawText(const std::string& text)
{
  if ((m_xPos < 0.0F) || (m_xPos >= static_cast<float>(m_screenWidth)))
  {
    return;
  }
  if ((m_yPos < 0.0F) || (m_yPos >= static_cast<float>(m_screenHeight)))
  {
    return;
  }

  const auto colorT            = GetColorT();
  const auto fontCharColorMixT = GetFontCharColorMixT();
  const auto textBrightness    = GetTextBrightness();

  const auto getFontColor = [this, &colorT, &textBrightness, &fontCharColorMixT](
                                [[maybe_unused]] const size_t textIndexOfChar,
                                const Point2dInt& point,
                                const Dimensions& charDimensions)
  {
    const auto color = GetInteriorColor(colorT, fontCharColorMixT, point, charDimensions);
    return m_textColorAdjust.GetAdjustment(textBrightness, color);
  };

  const auto getOutlineFontColor = [this, &colorT, &textBrightness, &fontCharColorMixT](
                                       [[maybe_unused]] const size_t textIndexOfChar,
                                       const Point2dInt& point,
                                       const Dimensions& charDimensions)
  {
    const auto color =
        GetOutlineColor(point.x, {colorT, fontCharColorMixT}, charDimensions.GetIntWidth());
    return m_textColorAdjust.GetAdjustment(textBrightness, color);
  };

  const auto charSpacing = GetCharSpacing();

  const auto drawStr = [this, &charSpacing, &getFontColor, &getOutlineFontColor](
                           const std::string& str, const Point2dInt& point)
  {
    m_textDrawer->SetText(str);
    m_textDrawer->SetFontColorFunc(getFontColor);
    m_textDrawer->SetOutlineFontColorFunc(getOutlineFontColor);
    m_textDrawer->SetCharSpacing(charSpacing);
    m_textDrawer->Prepare();
    m_textDrawer->Draw(point);
  };

  const auto textStrings = GetLinesOfWords(text, MAX_LINE_LENGTH);
  const auto lineSpacing = m_textDrawer->GetFontSize() + m_textDrawer->GetLineSpacing();
  auto y                 = static_cast<int32_t>(std::round(m_yPos));
  for (const auto& str : textStrings)
  {
    drawStr(str, {static_cast<int32_t>(std::round(m_xPos)), y});
    y += lineSpacing;
  }
}

inline auto GoomTitleDisplayer::GetInteriorColor(const float fontColorT,
                                                 const float fontCharColorMixT,
                                                 const Point2dInt& point,
                                                 const Dimensions& charDimensions) const -> Pixel
{
  if (IsInitialPhase())
  {
    return GetInitialPhaseInteriorColor(fontColorT);
  }

  if (IsMiddlePhase())
  {
    return GetMiddlePhaseInteriorColor(point, {fontColorT, fontCharColorMixT}, charDimensions);
  }

  return GetFinalPhaseInteriorColor(point, fontCharColorMixT, charDimensions);
}

inline auto GoomTitleDisplayer::GetInitialPhaseInteriorColor(const float fontColorT) const -> Pixel
{
  return m_textColorMap.GetColor(fontColorT);
}

inline auto GoomTitleDisplayer::GetMiddlePhaseInteriorColor(const Point2dInt& point,
                                                            const FontTs& fontTs,
                                                            const Dimensions& charDimensions) const
    -> Pixel
{
  const auto fontColor = m_textColorMap.GetColor(fontTs.fontColorT);
  const auto charColor1 =
      m_charColorMap.GetColor(GetFltFraction<int32_t>({point.x, charDimensions.GetIntWidth()}));
  const auto charColor2 = m_textOutlineColorMap.GetColor(
      GetFltFraction<int32_t>({point.y, charDimensions.GetIntHeight()}));
  static constexpr auto CHAR_COLOR_MIX = 0.5F;
  const auto charColor = ColorMaps::GetColorMix(charColor1, charColor2, CHAR_COLOR_MIX);

  return ColorMaps::GetColorMix(fontColor, charColor, fontTs.fontCharColorMixT);
}

inline auto GoomTitleDisplayer::GetFinalPhaseInteriorColor(const Point2dInt& point,
                                                           const float fontCharColorMixT,
                                                           const Dimensions& charDimensions) const
    -> Pixel
{
  const auto fontColor = m_textColorMap.GetColor(fontCharColorMixT);
  const auto charColor1 =
      m_charColorMap.GetColor(GetFltFraction<int32_t>({point.x, charDimensions.GetIntWidth()}));
  const auto charColor2 = m_textOutlineColorMap.GetColor(
      GetFltFraction<int32_t>({point.y, charDimensions.GetIntHeight()}));
  static constexpr auto CHAR_COLOR_MIX = 0.8F;
  const auto charColor  = ColorMaps::GetColorMix(charColor1, charColor2, CHAR_COLOR_MIX);
  const auto finalColor = ColorMaps::GetColorMix(fontColor, charColor, fontCharColorMixT);

  return finalColor;
}

inline auto GoomTitleDisplayer::GetOutlineColor(const int32_t x,
                                                const FontTs& fontTs,
                                                const int32_t charWidth) const -> Pixel
{
  if (IsFinalPhase())
  {
    return m_textOutlineColorMap.GetColor(fontTs.fontColorT);
  }

  const auto outlineFontColor =
      (not IsMiddlePhase()) ? WHITE_PIXEL : m_textOutlineColorMap.GetColor(fontTs.fontColorT);

  const auto charColor = m_textOutlineColorMap.GetColor(GetFltFraction<int32_t>({x, charWidth}));
  return ColorMaps::GetColorMix(outlineFontColor, charColor, fontTs.fontCharColorMixT);
}

inline auto GoomTitleDisplayer::GetColorT() const -> float
{
  if (m_timeLeftOfTitleDisplay <= 0)
  {
    return 0.0F;
  }

  return GetFltFraction<int32_t>({m_timeLeftOfTitleDisplay, MAX_TEXT_DISPLAY_TIME});
}

inline auto GoomTitleDisplayer::GetTextBrightness() const -> float
{
  if (IsInitialPhase())
  {
    return BASE_BRIGHTNESS;
  }
  if (IsMiddlePhase())
  {
    return BASE_BRIGHTNESS;
  }

  if (m_timeLeftOfTitleDisplay <= 0)
  {
    return BASE_BRIGHTNESS * MAX_BRIGHTNESS_FACTOR;
  }

  const auto fractionOfTimeLeft =
      GetFltFraction<int32_t>({m_timeLeftOfTitleDisplay, TIME_TO_START_FINAL_PHASE});
  return BASE_BRIGHTNESS * (MAX_BRIGHTNESS_FACTOR - fractionOfTimeLeft);
}

inline auto GoomTitleDisplayer::GetFontCharColorMixT() const -> float
{
  if (IsInitialPhase())
  {
    return INITIAL_PHASE_T_MIX;
  }
  if (IsMiddlePhase())
  {
    return GetFltFraction<int32_t>({m_timeLeftOfTitleDisplay, TIME_TO_START_MIDDLE_PHASE});
  }
  if (m_timeLeftOfTitleDisplay <= 0)
  {
    return 0.0F;
  }

  return GetFltFraction<int32_t>({m_timeLeftOfTitleDisplay, TIME_TO_START_FINAL_PHASE});
}

auto GoomTitleDisplayer::GetCharSpacing() const -> float
{
  if (IsInitialPhase())
  {
    return 0.0F;
  }

  const auto timeGone = static_cast<float>(
      m_timeLeftOfTitleDisplay <= 0 ? TIME_TO_START_MIDDLE_PHASE
                                    : (TIME_TO_START_MIDDLE_PHASE - m_timeLeftOfTitleDisplay));

  return SPACE_FACTOR * timeGone;
}

auto GoomTitleDisplayer::GetXIncrement() const -> float
{
  if (IsInitialPhase())
  {
    return INITIAL_PHASE_X_INCREMENT;
  }
  if (IsMiddlePhase())
  {
    return MIDDLE_PHASE_X_INCREMENT;
  }

  return m_finalPhaseIncrements.xIncrement;
}

auto GoomTitleDisplayer::GetYIncrement() const -> float
{
  if (IsInitialPhase())
  {
    return INITIAL_PHASE_Y_INCREMENT;
  }
  if (IsMiddlePhase())
  {
    return MIDDLE_PHASE_Y_INCREMENT;
  }

  return m_finalPhaseIncrements.yIncrement;
}

} // namespace GOOM::CONTROL
