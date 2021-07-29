#include "goom_title_display.h"

#include "../draw/text_draw.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "goomutils/colormaps.h"
#include "goomutils/colorutils.h"
#include "goomutils/random_colormaps.h"

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
using UTILS::IColorMap;
using UTILS::RandomColorMaps;

constexpr uint32_t FONT_SIZE = 35;

GoomTitleDisplay::GoomTitleDisplay(const int32_t xStart,
                                   const int32_t yStart,
                                   const std::string& fontDirectory,
                                   const IGoomDraw* const draw)
  : m_xPos{static_cast<float>(xStart)},
    m_yPos{static_cast<float>(yStart)},
    m_textDraw{std::make_unique<TextDraw>(draw)},
    m_fontDirectory{fontDirectory},
    m_textColorMap{&(RandomColorMaps{}.GetRandomColorMap(
        UTILS::ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM))},
    m_textOutlineColorMap{&(RandomColorMaps{}.GetRandomColorMap(UTILS::ColorMapGroup::PASTEL))},
    m_charColorMap{m_textColorMap}
{
  m_textDraw->SetFontFile(m_fontDirectory + PATH_SEP + "verdana.ttf");
  m_textDraw->SetFontSize(FONT_SIZE);
  m_textDraw->SetOutlineWidth(4);
  m_textDraw->SetAlignment(TextDraw::TextAlignment::LEFT);
}

GoomTitleDisplay::~GoomTitleDisplay() noexcept = default;

void GoomTitleDisplay::Draw(const std::string& title)
{
  m_timeLeftOfTitleDisplay--;

  if (m_timeLeftOfTitleDisplay == TIME_TO_START_FINAL_PHASE)
  {
    //    m_textColorMap = &(RandomColorMaps{}.GetRandomColorMap(UTILS::ColorMapGroup::DIVERGING_BLACK));
    m_textColorMap = &(RandomColorMaps{}.GetRandomColorMap());
    m_textOutlineColorMap = &(RandomColorMaps{}.GetRandomColorMap());
    m_charColorMap = &(RandomColorMaps{}.GetRandomColorMap(ColorMapGroup::DIVERGING_BLACK));
  }
  if (m_timeLeftOfTitleDisplay < TIME_TO_START_FINAL_PHASE)
  {
    m_textDraw->SetFontSize(static_cast<int32_t>(
        std::round(std::min(20.0F, static_cast<float>(TIME_TO_START_FINAL_PHASE) /
                                       static_cast<float>(m_timeLeftOfTitleDisplay)) *
                   static_cast<float>(FONT_SIZE))));
  }

  m_xPos += GetXIncrement();
  m_yPos += GetYIncrement();

  DrawText(title);
}

void GoomTitleDisplay::DrawText(const std::string& str)
{
  const float t =
      static_cast<float>(m_timeLeftOfTitleDisplay) / static_cast<float>(MAX_TEXT_DISPLAY_TIME);

  const Pixel fontColor = m_textColorMap->GetColor(t);
  const float tMix = IsInitialPhase()
                         ? 0.05F
                         : 0.5F * (2.0F - static_cast<float>(m_timeLeftOfTitleDisplay) /
                                              static_cast<float>(TIME_TO_START_MIDDLE_PHASE));
  const float brightness = !IsFinalPhase()
                               ? 1.0F
                               : 2.5F * 0.5F *
                                     (2.0F - static_cast<float>(m_timeLeftOfTitleDisplay) /
                                                 static_cast<float>(TIME_TO_START_FINAL_PHASE));

  const Pixel outlineFontColor =
      IsInitialPhase() ? Pixel::WHITE : m_textOutlineColorMap->GetColor(t);
  const float tCharStep = 0.0001F;
  float tFontChar = 0.0F;
  const auto getFontColor = [&]([[maybe_unused]] const size_t textIndexOfChar,
                                [[maybe_unused]] const int32_t x, [[maybe_unused]] const int32_t y,
                                [[maybe_unused]] const int32_t width,
                                [[maybe_unused]] const int32_t height)
  {
    //    LogInfo("textIndexOfChar = {}, x = {}, y = {}, xPrev = {}, tChar = {}", textIndexOfChar, x, y, xPrev, tChar);
    const Pixel charColor = m_charColorMap->GetColor(tFontChar);
    tFontChar += tCharStep;
    if (tFontChar > 1.0F)
    {
      tFontChar = 0.0F;
    }
    return GetTextGammaCorrection(brightness, IColorMap::GetColorMix(fontColor, charColor, tMix));
  };

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
    const Pixel charColor = m_textOutlineColorMap->GetColor(tOutlineFontChar);
    return GetTextGammaCorrection(brightness,
                                  IColorMap::GetColorMix(outlineFontColor, charColor, tMix));
  };

  const float spacing = GetCharSpacing();

  m_textDraw->SetText(str);
  m_textDraw->SetFontColorFunc(getFontColor);
  m_textDraw->SetOutlineFontColorFunc(getOutlineFontColor);
  m_textDraw->SetCharSpacing(spacing);
  m_textDraw->Prepare();
  m_textDraw->Draw(static_cast<int32_t>(std::round(m_xPos)),
                   static_cast<int32_t>(std::round(m_yPos)));
}

auto GoomTitleDisplay::GetCharSpacing() const -> float
{
  if (IsInitialPhase())
  {
    return 0.0F;
  }

  const auto timeGone = static_cast<float>(TIME_TO_START_MIDDLE_PHASE - m_timeLeftOfTitleDisplay);
  return 0.056F * timeGone;
}

auto GoomTitleDisplay::GetXIncrement() const -> float
{
  if (IsInitialPhase())
  {
    return 0.01F;
  }
  if (IsMiddlePhase())
  {
    return 1.0F;
  }

  return 10.0F;
}

auto GoomTitleDisplay::GetYIncrement() const -> float
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

inline auto GoomTitleDisplay::GetTextGammaCorrection(const float brightness,
                                                     const Pixel& color) const -> Pixel
{
  // if constexpr (TEXT_GAMMA == 1.0F)
  if (TEXT_GAMMA == 1.0F)
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
