#include "goom_title_displayer.h"

//#undef NO_LOGGING

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "draw/text_draw.h"
#include "goom/logging.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/text/drawable_text.h"

#include <cstdint>
#include <memory>
#include <string>

namespace GOOM::CONTROL
{

using COLOR::ColorMapGroup;
using COLOR::GetBrighterColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using DRAW::TextDraw;
using UTILS::Logging;
using UTILS::MATH::GetFltFraction;
using UTILS::MATH::I_HALF;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::TWO_PI;
using UTILS::TEXT::GetLeftAlignedPenForCentringStringAt;
using UTILS::TEXT::GetLinesOfWords;

static constexpr float FONT_SIZE_FRACTION_OF_SCREEN_HEIGHT = 0.05F;
static constexpr int32_t OUTLINE_FONT_WIDTH = 4;

// To normalize: turn on logging in TextDraw, get width of prepared text for a
// sample text for each font, then normalize with 'verdana' as 1.0.
//
// clang-format off
const std::vector<GoomTitleDisplayer::FontInfo> GoomTitleDisplayer::S_FONT_INFO = {
    {"AeroviasBrasilNF.ttf",   1.34F},
    {"AlexBrush-Regular.ttf",  1.25F},
    {"AvenueX-Regular.otf",    1.01F},
    {"CelticHand.ttf",         0.99F},
    {"CheapSign.ttf",          1.26F},
    {"EatAtJoes.ttf",          0.90F},
    {"GreatVibes-Regular.ttf", 1.29F},
    {"KellsFLF.ttf",           1.23F},
    {"Rubik-Regular.ttf",      1.1F},
    {"verdana.ttf",            1.0F},
};
// clang-format on

inline auto GoomTitleDisplayer::GetSelectedFontPath() const -> std::string
{
  return m_fontDirectory + PATH_SEP + S_FONT_INFO.at(m_fontInfoIndex).fontFilename;
}

inline auto GoomTitleDisplayer::GetSelectedFontSize() const -> int32_t
{
  const FontInfo& fontInfo = S_FONT_INFO.at(m_fontInfoIndex);
  const auto maxFontSize = static_cast<int32_t>(
      (FONT_SIZE_FRACTION_OF_SCREEN_HEIGHT * static_cast<float>(m_screenHeight)) *
      fontInfo.fontSizeNormalizeFactor);
  return maxFontSize;
}

GoomTitleDisplayer::GoomTitleDisplayer(IGoomDraw& draw,
                                       const IGoomRand& goomRand,
                                       const std::string& fontDirectory)
  : m_goomRand{goomRand},
    m_textDraw{std::make_unique<TextDraw>(draw)},
    m_screenWidth{static_cast<int32_t>(draw.GetScreenWidth())},
    m_screenHeight{static_cast<int32_t>(draw.GetScreenHeight())},
    m_fontDirectory{fontDirectory},
    m_fontInfoIndex{m_goomRand.GetRandInRange(0U, static_cast<uint32_t>(S_FONT_INFO.size()))},
    m_textColorMap{&RandomColorMaps::GetRandomColorMap(m_goomRand)},
    m_textOutlineColorMap{&RandomColorMaps::GetRandomColorMap(m_goomRand)},
    m_charColorMap{
        &RandomColorMaps::GetRandomColorMap(m_goomRand, COLOR::ColorMapGroup::DIVERGING_BLACK)}
{
  m_textDraw->SetFontFile(GetSelectedFontPath());
  m_textDraw->SetFontSize(GetSelectedFontSize());
  m_textDraw->SetOutlineWidth(OUTLINE_FONT_WIDTH);
  m_textDraw->SetAlignment(TextDraw::TextAlignment::LEFT);
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
    m_textDraw->SetFontSize(GetFinalPhaseFontSize(m_timeLeftOfTitleDisplay));
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
  const RandomColorMaps randomColorMaps{m_goomRand};
  m_textColorMap = &randomColorMaps.GetRandomColorMap(COLOR::ColorMapGroup::DIVERGING_BLACK);
  m_textOutlineColorMap = &randomColorMaps.GetRandomColorMap(COLOR::ColorMapGroup::HEAT);
  m_charColorMap = &randomColorMaps.GetRandomColorMap(COLOR::ColorMapGroup::DIVERGING_BLACK);
}

inline auto GoomTitleDisplayer::GetFinalPhaseFontSize(const int32_t timeLeftOfTitleDisplay) const
    -> int32_t
{
  static constexpr float MAX_FONT_SIZE_MULTIPLIER = 10.0F;
  const float fractionOfTimeLeft =
      GetFltFraction(timeLeftOfTitleDisplay, TIME_TO_START_FINAL_PHASE);
  const float t = 1.0F - std::pow(fractionOfTimeLeft, 0.7F);
  const float fontSizeMultiplier = STD20::lerp(1.0F, MAX_FONT_SIZE_MULTIPLIER, t);
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
  const Point2dInt screenCentre{I_HALF * m_screenWidth, I_HALF * m_screenHeight};
  const int32_t fontSize = GetFinalPhaseFontSize(0);
  return GetLeftAlignedPenForCentringStringAt(*m_textDraw, str, fontSize, screenCentre).ToFlt();
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

  const float colorT = GetColorT();
  const float fontCharColorMixT = GetFontCharColorMixT();
  const float textBrightness = GetTextBrightness();

  const auto getFontColor = [this, &colorT, &textBrightness, &fontCharColorMixT](
                                [[maybe_unused]] const size_t textIndexOfChar,
                                const Point2dInt point, const int32_t width, const int32_t height)
  {
    const Pixel finalColor = GetFinalInteriorColor(colorT, fontCharColorMixT, point, width, height);
    return m_textColorAdjust.GetAdjustment(textBrightness, finalColor);
  };

  const auto getOutlineFontColor = [this, &colorT, &textBrightness, &fontCharColorMixT](
                                       [[maybe_unused]] const size_t textIndexOfChar,
                                       const Point2dInt point, [[maybe_unused]] const int32_t width,
                                       [[maybe_unused]] const int32_t height)
  {
    const Pixel finalColor = GetFinalOutlineColor(colorT, fontCharColorMixT, point.x, width);
    return m_textColorAdjust.GetAdjustment(textBrightness, finalColor);
  };

  const float charSpacing = GetCharSpacing();

  const auto drawStr = [this, &charSpacing, &getFontColor,
                        &getOutlineFontColor](const std::string& str, const Point2dInt point)
  {
    m_textDraw->SetText(str);
    m_textDraw->SetFontColorFunc(getFontColor);
    m_textDraw->SetOutlineFontColorFunc(getOutlineFontColor);
    m_textDraw->SetCharSpacing(charSpacing);
    m_textDraw->Prepare();
    m_textDraw->Draw(point);
  };

  static constexpr size_t MAX_LINE_LENGTH = 40;
  const std::vector<std::string> textStrings = GetLinesOfWords(text, MAX_LINE_LENGTH);
  const int32_t lineSpacing = m_textDraw->GetFontSize() + m_textDraw->GetLineSpacing();
  auto y = static_cast<int32_t>(std::round(m_yPos));
  for (const auto& str : textStrings)
  {
    drawStr(str, {static_cast<int32_t>(std::round(m_xPos)), y});
    y += lineSpacing;
  }
}

inline auto GoomTitleDisplayer::GetFinalInteriorColor(const float fontColorT,
                                                      const float fontCharColorMixT,
                                                      const Point2dInt point,
                                                      const int32_t charWidth,
                                                      const int32_t charHeight) const -> Pixel
{
  if (IsInitialPhase())
  {
    return GetInitialPhaseInteriorColor(fontColorT);
  }

  if (IsMiddlePhase())
  {
    return GetMiddlePhaseInteriorColor(fontColorT, fontCharColorMixT, point, charWidth, charHeight);
  }

  return GetFinalPhaseInteriorColor(fontCharColorMixT, point, charWidth, charHeight);
}

inline auto GoomTitleDisplayer::GetInitialPhaseInteriorColor(const float fontColorT) const -> Pixel
{
  return m_textColorMap->GetColor(fontColorT);
}

inline auto GoomTitleDisplayer::GetMiddlePhaseInteriorColor(const float fontColorT,
                                                            const float fontCharColorMixT,
                                                            const Point2dInt point,
                                                            const int32_t charWidth,
                                                            const int32_t charHeight) const -> Pixel
{
  const Pixel fontColor = m_textColorMap->GetColor(fontColorT);
  const Pixel charColor1 = m_charColorMap->GetColor(GetFltFraction(point.x, charWidth));
  const Pixel charColor2 = m_textOutlineColorMap->GetColor(GetFltFraction(point.y, charHeight));
  static constexpr float CHAR_COLOR_MIX = 0.5F;
  const Pixel charColor = IColorMap::GetColorMix(charColor1, charColor2, CHAR_COLOR_MIX);

  return IColorMap::GetColorMix(fontColor, charColor, fontCharColorMixT);
}

inline auto GoomTitleDisplayer::GetFinalPhaseInteriorColor(
    [[maybe_unused]] const float fontCharColorMixT,
    const Point2dInt point,
    const int32_t charWidth,
    const int32_t charHeight) const -> Pixel
{
  const float tX = GetFltFraction(point.x, charWidth);
  const float tY = GetFltFraction(point.y, charHeight);
  const float t = tX * tY;
  const float tReverse = (1.0F - tX) * (1.0F - tY);
  static constexpr float FREQ = 15.0F * TWO_PI;
  const Pixel fontColor = m_textColorMap->GetColor(std::sin(FREQ * t));
  const Pixel charColor = m_charColorMap->GetColor(std::sin(FREQ * tReverse));
  static constexpr float MIX_FACTOR = 0.5F;
  const float finalColorMixT = MIX_FACTOR;

  return IColorMap::GetColorMix(fontColor, charColor, finalColorMixT);
}

inline auto GoomTitleDisplayer::GetFinalOutlineColor(const float fontColorT,
                                                     const float fontCharColorMixT,
                                                     const int32_t x,
                                                     const int32_t charWidth) const -> Pixel
{
  const Pixel outlineFontColor =
      (!IsMiddlePhase()) ? WHITE_PIXEL : m_textOutlineColorMap->GetColor(fontColorT);

  const Pixel charColor = m_textOutlineColorMap->GetColor(GetFltFraction(x, charWidth));
  return IColorMap::GetColorMix(outlineFontColor, charColor, fontCharColorMixT);
}

inline auto GoomTitleDisplayer::GetColorT() const -> float
{
  if (m_timeLeftOfTitleDisplay <= 0)
  {
    return 0.0F;
  }

  return GetFltFraction(m_timeLeftOfTitleDisplay, MAX_TEXT_DISPLAY_TIME);
}

inline auto GoomTitleDisplayer::GetTextBrightness() const -> float
{
  static constexpr float BASE_BRIGHTNESS = 10.0F;

  if (IsInitialPhase())
  {
    return BASE_BRIGHTNESS;
  }
  if (IsMiddlePhase())
  {
    return BASE_BRIGHTNESS;
  }

  static constexpr float MAX_BRIGHTNESS_FACTOR = 2.0F;
  if (m_timeLeftOfTitleDisplay <= 0)
  {
    return BASE_BRIGHTNESS * MAX_BRIGHTNESS_FACTOR;
  }

  const float fractionOfTimeLeft =
      GetFltFraction(m_timeLeftOfTitleDisplay, TIME_TO_START_FINAL_PHASE);
  return BASE_BRIGHTNESS * (MAX_BRIGHTNESS_FACTOR - fractionOfTimeLeft);
}

inline auto GoomTitleDisplayer::GetFontCharColorMixT() const -> float
{
  if (IsInitialPhase())
  {
    static constexpr float INITIAL_PHASE_T_MIX = 0.1F;
    return INITIAL_PHASE_T_MIX;
  }
  if (IsMiddlePhase())
  {
    return GetFltFraction(m_timeLeftOfTitleDisplay, TIME_TO_START_MIDDLE_PHASE);
  }
  if (m_timeLeftOfTitleDisplay <= 0)
  {
    return 0.0F;
  }

  return GetFltFraction(m_timeLeftOfTitleDisplay, TIME_TO_START_FINAL_PHASE);
}

auto GoomTitleDisplayer::GetCharSpacing() const -> float
{
  if (IsInitialPhase())
  {
    return 0.0F;
  }

  static constexpr float SPACE_FACTOR = 0.056F;
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
