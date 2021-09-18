#include "goom_title_displayer.h"

#include "../draw/text_draw.h"
#include "goom_config.h"
#include "goom_draw.h"
#include "goom_graphic.h"
#include "goomutils/colormaps.h"
#include "goomutils/colorutils.h"
#include "goomutils/logging_control.h"
//#undef NO_LOGGING
#include "goomutils/logging.h"
#include "goomutils/random_colormaps.h"
#include "goomutils/strutils.h"

#include <cstdint>
#include <memory>
#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

using DRAW::TextDraw;
using UTILS::ColorMapGroup;
using UTILS::GetBrighterColor;
using UTILS::GetRandInRange;
using UTILS::IColorMap;
using UTILS::Logging;
using UTILS::RandomColorMaps;
using UTILS::StringSplit;

constexpr float FONT_SIZE_FRACTION_OF_SCREEN_HEIGHT = 0.05F;
constexpr int32_t OUTLINE_FONT_WIDTH = 4;

// To normalize: turn on logging in TextDraw, get width of prepared text for a
// sample text for each font, then normalize with 'verdana' as 1.0.
//
//@formatter:off
// clang-format off
const std::vector<GoomTitleDisplayer::FontInfo> GoomTitleDisplayer::s_fontInfo = {
    {"AeroviasBrasilNF.ttf", 1.34F},
    {"AlexBrush-Regular.ttf", 1.25F},
    {"AvenueX-Regular.otf", 1.01F},
    {"CelticHand.ttf", 0.99F},
    {"CheapSign.ttf", 1.26F},
    {"EatAtJoes.ttf", 0.90F},
    {"GreatVibes-Regular.ttf", 1.29F},
    {"KellsFLF.ttf", 1.23F},
    {"Rubik-Regular.ttf", 1.1F},
    {"verdana.ttf", 1.0F},
};
//@formatter:on
// clang-format on

auto GoomTitleDisplayer::GetSelectedFontPath() const -> std::string
{
  return m_fontDirectory + PATH_SEP + s_fontInfo.at(m_fontInfoIndex).fontFilename;
}

auto GoomTitleDisplayer::GetSelectedFontSize() const -> int32_t
{
  const FontInfo& fontInfo = s_fontInfo.at(m_fontInfoIndex);
  const auto maxFontSize =
      static_cast<int32_t>(FONT_SIZE_FRACTION_OF_SCREEN_HEIGHT *
                           static_cast<float>(m_screenHeight) * fontInfo.fontSizeNormalizeFactor);
  return maxFontSize;
}

GoomTitleDisplayer::GoomTitleDisplayer(const IGoomDraw& draw, const std::string& fontDirectory)
  : m_textDraw{std::make_unique<TextDraw>(draw)},
    m_screenHeight{draw.GetScreenHeight()},
    m_fontDirectory{fontDirectory},
    m_fontInfoIndex{GetRandInRange(0U, static_cast<uint32_t>(s_fontInfo.size()))},
    m_textColorMap{RandomColorMaps{}.GetRandomColorMap(
        UTILS::ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM)},
    m_textOutlineColorMap{RandomColorMaps{}.GetRandomColorMap(UTILS::ColorMapGroup::PASTEL)},
    m_charColorMap{m_textColorMap}
{
  m_textDraw->SetFontFile(GetSelectedFontPath());
  m_textDraw->SetFontSize(GetSelectedFontSize());
  m_textDraw->SetOutlineWidth(OUTLINE_FONT_WIDTH);
  m_textDraw->SetAlignment(TextDraw::TextAlignment::LEFT);
}

void GoomTitleDisplayer::Draw(const std::string& title)
{
  --m_timeLeftOfTitleDisplay;

  if (m_timeLeftOfTitleDisplay == TIME_TO_START_FINAL_PHASE)
  {
    m_textColorMap = RandomColorMaps{}.GetRandomColorMap();
    m_textOutlineColorMap = RandomColorMaps{}.GetRandomColorMap();
    m_charColorMap = RandomColorMaps{}.GetRandomColorMap(ColorMapGroup::DIVERGING_BLACK);
  }
  else if (m_timeLeftOfTitleDisplay < TIME_TO_START_FINAL_PHASE)
  {
    m_textDraw->SetFontSize(static_cast<int32_t>(
        std::round(std::min(15.0F, static_cast<float>(TIME_TO_START_FINAL_PHASE) /
                                       static_cast<float>(m_timeLeftOfTitleDisplay)) *
                   static_cast<float>(GetSelectedFontSize()))));
  }

  m_xPos += GetXIncrement();
  m_yPos += GetYIncrement();

  DrawText(title);
}

void GoomTitleDisplayer::DrawText(const std::string& text)
{
  const float t =
      static_cast<float>(m_timeLeftOfTitleDisplay) / static_cast<float>(MAX_TEXT_DISPLAY_TIME);

  const float tMix = IsInitialPhase()
                         ? 0.05F
                         : (0.5F * (2.0F - (static_cast<float>(m_timeLeftOfTitleDisplay) /
                                            static_cast<float>(TIME_TO_START_MIDDLE_PHASE))));
  const float brightness = !IsFinalPhase()
                               ? 1.0F
                               : (2.5F * 0.5F *
                                  (2.0F - (static_cast<float>(m_timeLeftOfTitleDisplay) /
                                           static_cast<float>(TIME_TO_START_FINAL_PHASE))));
  const float tCharStep = 0.0001F;

  const Pixel fontColor = m_textColorMap.get().GetColor(t);
  float tFontChar = 0.0F;
  const auto getFontColor = [&]([[maybe_unused]] const size_t textIndexOfChar,
                                [[maybe_unused]] const int32_t x, [[maybe_unused]] const int32_t y,
                                [[maybe_unused]] const int32_t width,
                                [[maybe_unused]] const int32_t height) {
    const Pixel charColor = m_charColorMap.get().GetColor(tFontChar);
    tFontChar += tCharStep;
    if (tFontChar > 1.0F)
    {
      tFontChar = 0.0F;
    }
    return GetTextGammaCorrection(brightness, IColorMap::GetColorMix(fontColor, charColor, tMix));
  };

  const Pixel outlineFontColor =
      IsInitialPhase() ? Pixel::WHITE : m_textOutlineColorMap.get().GetColor(t);
  float tOutlineFontChar = 0.0F;
  const auto getOutlineFontColor =
      [&]([[maybe_unused]] const size_t textIndexOfChar, [[maybe_unused]] const int32_t x,
          [[maybe_unused]] const int32_t y, [[maybe_unused]] const int32_t width,
          [[maybe_unused]] const int32_t height)
  {
    tOutlineFontChar += tCharStep;
    if (tOutlineFontChar > 1.0F)
    {
      tOutlineFontChar = 0.0F;
    }
    const Pixel charColor = m_textOutlineColorMap.get().GetColor(tOutlineFontChar);
    return GetTextGammaCorrection(brightness,
                                  IColorMap::GetColorMix(outlineFontColor, charColor, tMix));
  };

  const float charSpacing = GetCharSpacing();

  const auto drawStr = [&](const std::string& str, const int32_t x, const int32_t y) {
    m_textDraw->SetText(str);
    m_textDraw->SetFontColorFunc(getFontColor);
    m_textDraw->SetOutlineFontColorFunc(getOutlineFontColor);
    m_textDraw->SetCharSpacing(charSpacing);
    m_textDraw->Prepare();
    m_textDraw->Draw(x, y);
  };

  const std::vector<std::string> textStrings = GetTextLines(text);
  const int32_t lineSpacing = m_textDraw->GetFontSize() + m_textDraw->GetLineSpacing();
  auto y = static_cast<int32_t>(std::round(m_yPos));
  for (const auto& str : textStrings)
  {
    drawStr(str, static_cast<int32_t>(std::round(m_xPos)), y);
    y += lineSpacing;
  }
}

auto GoomTitleDisplayer::GetTextLines(const std::string& text) -> std::vector<std::string>
{
  constexpr size_t MAX_LINE_LENGTH = 40;
  const size_t textLen = text.length();
  if (textLen <= MAX_LINE_LENGTH)
  {
    return {text};
  }

  const std::vector<std::string> words = StringSplit(text, " ");
  std::vector<std::string> textLines{};
  std::string str{};

  for (const auto& word : words)
  {
    str += word + " ";
    if (str.length() > MAX_LINE_LENGTH)
    {
      textLines.emplace_back(str);
      str = "";
    }
  }

  if (!str.empty())
  {
    textLines.emplace_back(str);
  }

  return textLines;
}

auto GoomTitleDisplayer::GetCharSpacing() const -> float
{
  if (IsInitialPhase())
  {
    return 0.0F;
  }

  const auto timeGone = static_cast<float>(TIME_TO_START_MIDDLE_PHASE - m_timeLeftOfTitleDisplay);
  return 0.056F * timeGone;
}

auto GoomTitleDisplayer::GetXIncrement() const -> float
{
  if (IsInitialPhase())
  {
    return 0.01F;
  }
  if (IsMiddlePhase())
  {
    return 1.0F;
  }

  return 7.0F;
}

auto GoomTitleDisplayer::GetYIncrement() const -> float
{
  if (IsInitialPhase())
  {
    return 0.0F;
  }
  if (IsMiddlePhase())
  {
    return 0.0F;
  }

  return 8.0F;
}

inline auto GoomTitleDisplayer::GetTextGammaCorrection(const float brightness,
                                                       const Pixel& color) const -> Pixel
{
  // if constexpr (TEXT_GAMMA == 1.0F)
  if (1.0F == TEXT_GAMMA)
  {
    return GetBrighterColor(brightness, color, true);
  }
  return m_textGammaCorrect.GetCorrection(brightness, color);
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
