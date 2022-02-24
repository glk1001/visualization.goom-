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
using COLOR::GetIncreasedChroma;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using DRAW::TextDraw;
using UTILS::Logging;
using UTILS::MATH::GetFltFraction;
using UTILS::MATH::I_HALF;
using UTILS::MATH::IGoomRand;
using UTILS::TEXT::GetLeftAlignedPenForCentringStringAt;
using UTILS::TEXT::GetLinesOfWords;

constexpr float FONT_SIZE_FRACTION_OF_SCREEN_HEIGHT = 0.05F;
constexpr int32_t OUTLINE_FONT_WIDTH = 4;

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
    m_textColorMap{RandomColorMaps{m_goomRand}.GetRandomColorMap()},
    m_textOutlineColorMap{RandomColorMaps{m_goomRand}.GetRandomColorMap()},
    m_charColorMap{
        RandomColorMaps{m_goomRand}.GetRandomColorMap(COLOR::ColorMapGroup::DIVERGING_BLACK_SLIM)}
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
  m_textColorMap =
      randomColorMaps.GetRandomColorMap(COLOR::ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM);
  m_textOutlineColorMap = randomColorMaps.GetRandomColorMap(COLOR::ColorMapGroup::HEAT);
  m_charColorMap = randomColorMaps.GetRandomColorMap(COLOR::ColorMapGroup::DIVERGING_BLACK_SLIM);
}

inline auto GoomTitleDisplayer::GetFinalPhaseFontSize(const int32_t timeLeftOfTitleDisplay) const
    -> int32_t
{
  constexpr float MAX_FONT_SIZE_MULTIPLIER = 10.0F;
  const float fractionOfTimeLeft =
      GetFltFraction(timeLeftOfTitleDisplay, TIME_TO_START_FINAL_PHASE);
  const float t = 1.0F - std::pow(fractionOfTimeLeft, 0.7F);
  const float fontSizeMultiplier = STD20::lerp(1.0F, MAX_FONT_SIZE_MULTIPLIER, t);
  return static_cast<int32_t>(fontSizeMultiplier * static_cast<float>(GetSelectedFontSize()));
}

auto GoomTitleDisplayer::GetFinalPhaseIncrements(const std::string& title) const
    -> FinalPhaseIncrements
{
  const auto [finalXPos, finalYPos] = GetFinalPhaseCentrePenPos(title);
  return {(finalXPos - m_xPos) / static_cast<float>(TIME_TO_START_FINAL_PHASE),
          (finalYPos - m_yPos) / static_cast<float>(TIME_TO_START_FINAL_PHASE)};
}

inline auto GoomTitleDisplayer::GetFinalPhaseCentrePenPos(const std::string& str) const
    -> Point2dFlt
{
  const Point2dInt screenCentre{I_HALF * m_screenWidth, I_HALF * m_screenHeight};
  const int32_t fontSize = GetFinalPhaseFontSize(0);
  return GetLeftAlignedPenForCentringStringAt(*m_textDraw, str, fontSize, screenCentre).ToFlt();
}

// Reduce processing time by skipping some draws near the end (when the font is bigger).
inline auto GoomTitleDisplayer::SkipThisDraw() const -> bool
{
  constexpr int32_t SKIP_FREQ = 5;
  return IsFinalPhase() && (m_timeLeftOfTitleDisplay > 0) &&
         (0 == (m_timeLeftOfTitleDisplay % SKIP_FREQ));
}

void GoomTitleDisplayer::DrawText(const std::string& text)
{
  if (SkipThisDraw())
  {
    return;
  }

  const float colorT = GetColorT();
  const float fontCharColorMixT = GetFontCharColorMixT();
  const float textBrightness = GetTextBrightness();

  const auto getFontColor = [this, &colorT, &textBrightness, &fontCharColorMixT](
                                [[maybe_unused]] const size_t textIndexOfChar, const int32_t x,
                                const int32_t y, const int32_t width, const int32_t height)
  {
    const Pixel finalColor = GetFinalInteriorColor(colorT, fontCharColorMixT, x, y, width, height);
    return m_textGammaCorrect.GetCorrection(textBrightness, finalColor);
  };

  const auto getOutlineFontColor =
      [this, &colorT, &textBrightness, &fontCharColorMixT](
          [[maybe_unused]] const size_t textIndexOfChar, [[maybe_unused]] const int32_t x,
          [[maybe_unused]] const int32_t y, [[maybe_unused]] const int32_t width,
          [[maybe_unused]] const int32_t height)
  {
    const Pixel finalColor = GetFinalOutlineColor(colorT, fontCharColorMixT, x, width);
    return m_textGammaCorrect.GetCorrection(textBrightness, finalColor);
  };

  const float charSpacing = GetCharSpacing();

  const auto drawStr = [this, &charSpacing, &getFontColor, &getOutlineFontColor](
                           const std::string& str, const int32_t x, const int32_t y)
  {
    m_textDraw->SetText(str);
    m_textDraw->SetFontColorFunc(getFontColor);
    m_textDraw->SetOutlineFontColorFunc(getOutlineFontColor);
    m_textDraw->SetCharSpacing(charSpacing);
    m_textDraw->Prepare();
    m_textDraw->Draw(x, y);
  };

  constexpr size_t MAX_LINE_LENGTH = 40;
  const std::vector<std::string> textStrings = GetLinesOfWords(text, MAX_LINE_LENGTH);
  const int32_t lineSpacing = m_textDraw->GetFontSize() + m_textDraw->GetLineSpacing();
  auto y = static_cast<int32_t>(std::round(m_yPos));
  for (const auto& str : textStrings)
  {
    drawStr(str, static_cast<int32_t>(std::round(m_xPos)), y);
    y += lineSpacing;
  }
}

inline auto GoomTitleDisplayer::GetFinalInteriorColor(const float fontColorT,
                                                      const float fontCharColorMixT,
                                                      const int32_t x,
                                                      const int32_t y,
                                                      const int32_t charWidth,
                                                      const int32_t charHeight) const -> Pixel
{
  const Pixel fontColor = m_textColorMap.get().GetColor(fontColorT);
  if (IsInitialPhase())
  {
    return fontColor;
  }

  const Pixel charColor1 = m_charColorMap.get().GetColor(GetFltFraction(x, charWidth));
  const Pixel charColor2 = m_textOutlineColorMap.get().GetColor(GetFltFraction(y, charHeight));
  constexpr float CHAR_COLOR_MIX = 0.5F;
  const Pixel charColor = IColorMap::GetColorMix(charColor1, charColor2, CHAR_COLOR_MIX);

  // Favour the char color for the final phase.
  const float finalColorMixT = IsFinalPhase() ? (1.0F - fontCharColorMixT) : fontCharColorMixT;

  return GetIncreasedChroma(IColorMap::GetColorMix(fontColor, charColor, finalColorMixT));
}

inline auto GoomTitleDisplayer::GetFinalOutlineColor(const float fontColorT,
                                                     const float fontCharColorMixT,
                                                     const int32_t x,
                                                     const int32_t charWidth) const -> Pixel
{
  const Pixel outlineFontColor =
      (!IsMiddlePhase()) ? Pixel::WHITE : m_textOutlineColorMap.get().GetColor(fontColorT);

  const Pixel charColor = m_textOutlineColorMap.get().GetColor(GetFltFraction(x, charWidth));
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
  constexpr float BASE_BRIGHTNESS = 20.0F;

  if (!IsFinalPhase())
  {
    return BASE_BRIGHTNESS;
  }

  constexpr float MAX_BRIGHTNESS_FACTOR = 2.0F;
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
    constexpr float INITIAL_PHASE_T_MIX = 0.1F;
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

  constexpr float SPACE_FACTOR = 0.056F;
  const auto timeGone = static_cast<float>(
      m_timeLeftOfTitleDisplay <= 0 ? TIME_TO_START_MIDDLE_PHASE
                                    : TIME_TO_START_MIDDLE_PHASE - m_timeLeftOfTitleDisplay);

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
