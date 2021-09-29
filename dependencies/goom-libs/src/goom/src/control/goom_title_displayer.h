#pragma once

#include "color/colorutils.h"
#include "goom_graphic.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace GOOM
{

namespace DRAW
{
class IGoomDraw;
class TextDraw;
}
namespace COLOR
{
class IColorMap;
}

namespace CONTROL
{

class GoomTitleDisplayer
{
public:
  GoomTitleDisplayer() noexcept = delete;
  GoomTitleDisplayer(const DRAW::IGoomDraw& draw, const std::string& fontDirectory);

  void SetInitialPosition(int32_t xStart, int32_t yStart);

  [[nodiscard]] auto IsInitialPhase() const -> bool;
  [[nodiscard]] auto IsMiddlePhase() const -> bool;
  [[nodiscard]] auto IsFinalPhase() const -> bool;
  [[nodiscard]] auto IsFinished() const -> bool;

  void Draw(const std::string& title);

private:
  static constexpr int MAX_TEXT_DISPLAY_TIME = 200;
  static constexpr int TIME_TO_START_MIDDLE_PHASE = 100;
  static constexpr int TIME_TO_START_FINAL_PHASE = 50;
  static constexpr int32_t TIME_TO_START_FINAL_FADE = 15;
  float m_xPos = 0.0F;
  float m_yPos = 0.0F;
  int32_t m_timeLeftOfTitleDisplay = MAX_TEXT_DISPLAY_TIME;
  const std::unique_ptr<DRAW::TextDraw> m_textDraw;
  const uint32_t m_screenHeight;
  const std::string m_fontDirectory;
  struct FontInfo
  {
    std::string fontFilename;
    float fontSizeNormalizeFactor;
  };
  static const std::vector<FontInfo> s_fontInfo;
  const size_t m_fontInfoIndex;
  [[nodiscard]] auto GetSelectedFontPath() const -> std::string;
  [[nodiscard]] auto GetSelectedFontSize() const -> int32_t;
  std::reference_wrapper<const COLOR::IColorMap> m_textColorMap;
  std::reference_wrapper<const COLOR::IColorMap> m_textOutlineColorMap;
  std::reference_wrapper<const COLOR::IColorMap> m_charColorMap;
  void DrawText(const std::string& text);
  [[nodiscard]] static auto GetTextLines(const std::string& text) -> std::vector<std::string>;
  [[nodiscard]] auto GetCharSpacing() const -> float;
  [[nodiscard]] auto GetXIncrement() const -> float;
  [[nodiscard]] auto GetYIncrement() const -> float;
  static constexpr float TEXT_GAMMA = 1.0F / 1.0F;
  static constexpr float TEXT_GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  COLOR::GammaCorrection m_textGammaCorrect{TEXT_GAMMA, TEXT_GAMMA_BRIGHTNESS_THRESHOLD};
  [[nodiscard]] auto GetTextGammaCorrection(float brightness, const Pixel& color) const -> Pixel;
};

inline void GoomTitleDisplayer::SetInitialPosition(const int32_t xStart, const int32_t yStart)
{
  m_xPos = static_cast<float>(xStart);
  m_yPos = static_cast<float>(yStart);
}

inline auto GoomTitleDisplayer::IsInitialPhase() const -> bool
{
  return m_timeLeftOfTitleDisplay > TIME_TO_START_MIDDLE_PHASE;
}

inline auto GoomTitleDisplayer::IsMiddlePhase() const -> bool
{
  return (TIME_TO_START_MIDDLE_PHASE >= m_timeLeftOfTitleDisplay) &&
         (m_timeLeftOfTitleDisplay > TIME_TO_START_FINAL_PHASE);
}

inline auto GoomTitleDisplayer::IsFinalPhase() const -> bool
{
  return m_timeLeftOfTitleDisplay <= TIME_TO_START_FINAL_PHASE;
}

inline auto GoomTitleDisplayer::IsFinished() const -> bool
{
  return m_timeLeftOfTitleDisplay <= 0;
}

} // namespace CONTROL
} // namespace GOOM

