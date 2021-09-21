#include "goom_dots_fx.h"

#include "goom_draw.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "goomutils/colormaps.h"
#include "goomutils/colorutils.h"
#include "goomutils/graphics/image_bitmaps.h"
#include "goomutils/logging_control.h"
//#undef NO_LOGGING
#include "goomutils/graphics/small_image_bitmaps.h"
#include "goomutils/logging.h"
#include "goomutils/mathutils.h"
#include "goomutils/random_colormaps.h"
#include "goomutils/random_colormaps_manager.h"
#include "goomutils/spimpl.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>

namespace GOOM
{

using namespace GOOM::UTILS;
using COLOR_DATA::ColorMapName;

inline auto ChangeDotColorsEvent() -> bool
{
  return ProbabilityOfMInN(1, 3);
}

class GoomDotsFx::GoomDotsFxImpl
{
public:
  GoomDotsFxImpl(const IGoomDraw& draw,
                 const PluginInfo& goomInfo,
                 const SmallImageBitmaps& smallBitmaps) noexcept;

  void Start();

  void SetWeightedColorMaps(uint32_t dotNum, std::shared_ptr<RandomColorMaps> weightedMaps);

  void ApplySingle();
  void ApplyMultiple();

private:
  const IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const SmallImageBitmaps& m_smallBitmaps;
  const uint32_t m_pointWidth;
  const uint32_t m_pointHeight;

  const float m_pointWidthDiv2;
  const float m_pointHeightDiv2;
  const float m_pointWidthDiv3;
  const float m_pointHeightDiv3;

  SmallImageBitmaps::ImageNames m_currentBitmapName{};
  static constexpr uint32_t MAX_FLOWERS_IN_ROW = 100;
  uint32_t m_numFlowersInRow = 0;
  auto GetImageBitmap(size_t size) const -> const ImageBitmap&;

  static constexpr size_t MIN_DOT_SIZE = 3;
  static constexpr size_t MAX_DOT_SIZE = 21;
  static_assert(MAX_DOT_SIZE <= SmallImageBitmaps::MAX_IMAGE_SIZE, "Max dot size mismatch.");

  std::array<std::shared_ptr<RandomColorMaps>, NUM_DOT_TYPES> m_colorMaps{};
  std::array<RandomColorMapsManager, NUM_DOT_TYPES> m_colorMapsManagers{};
  std::array<uint32_t, NUM_DOT_TYPES> m_colorMapIds{};
  Pixel m_middleColor{};
  bool m_useSingleBufferOnly = true;
  bool m_thereIsOneBuffer = true;
  bool m_useGrayScale = false;

  uint32_t m_loopVar = 0; // mouvement des points

  static constexpr float GAMMA = 1.0F / 1.0F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
  auto GetGammaCorrection(float brightness, const Pixel& color) const -> Pixel;

  void Update();

  void ChangeColors();
  [[nodiscard]] static auto GetLargeSoundFactor(const SoundInfo& soundInfo) -> float;

  void DotFilter(
      const Pixel& color, float t1, float t2, float t3, float t4, uint32_t cycle, uint32_t radius);
  void SetNextCurrentBitmapName();
};

GoomDotsFx::GoomDotsFx(const IGoomDraw& draw,
                       const PluginInfo& goomInfo,
                       const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<GoomDotsFxImpl>(draw, goomInfo, smallBitmaps)}
{
}

void GoomDotsFx::SetWeightedColorMaps(const uint32_t dotNum,
                                      const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedColorMaps(dotNum, weightedMaps);
}

void GoomDotsFx::Start()
{
  m_fxImpl->Start();
}

void GoomDotsFx::Resume()
{
}

void GoomDotsFx::Suspend()
{
}

void GoomDotsFx::Finish()
{
}

auto GoomDotsFx::GetFxName() const -> std::string
{
  return "goom dots";
}

void GoomDotsFx::ApplySingle()
{
  m_fxImpl->ApplySingle();
}

void GoomDotsFx::ApplyMultiple()
{
  m_fxImpl->ApplyMultiple();
}


GoomDotsFx::GoomDotsFxImpl::GoomDotsFxImpl(const IGoomDraw& draw,
                                           const PluginInfo& goomInfo,
                                           const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{draw},
    m_goomInfo{goomInfo},
    m_smallBitmaps{smallBitmaps},
    m_pointWidth{(m_goomInfo.GetScreenInfo().width * 2) / 5},
    m_pointHeight{(m_goomInfo.GetScreenInfo().height * 2) / 5},
    m_pointWidthDiv2{static_cast<float>(m_pointWidth) / 2.0F},
    m_pointHeightDiv2{static_cast<float>(m_pointHeight) / 2.0F},
    m_pointWidthDiv3{static_cast<float>(m_pointWidth) / 3.0F},
    m_pointHeightDiv3{static_cast<float>(m_pointHeight) / 3.0F}
{
}

void GoomDotsFx::GoomDotsFxImpl::Start()
{
  ChangeColors();
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetImageBitmap(const size_t size) const
    -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(m_currentBitmapName,
                                       stdnew::clamp(size, MIN_DOT_SIZE, MAX_DOT_SIZE));
}

void GoomDotsFx::GoomDotsFxImpl::ChangeColors()
{
  for (auto& colorMapsManager : m_colorMapsManagers)
  {
    colorMapsManager.ChangeAllColorMapsNow();
  }

  m_middleColor = RandomColorMaps::GetRandomColor(
      *m_colorMaps[0]->GetRandomColorMapPtr(ColorMapGroup::MISC, RandomColorMaps::ALL), 0.1F, 1.0F);

  m_useSingleBufferOnly = ProbabilityOfMInN(0, 2);
  m_useGrayScale = ProbabilityOfMInN(0, 10);
}

inline void GoomDotsFx::GoomDotsFxImpl::SetWeightedColorMaps(
    const uint32_t dotNum, const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_colorMaps.at(dotNum) = weightedMaps;
  m_colorMapIds[dotNum] = m_colorMapsManagers[dotNum].AddColorMapInfo(
      {m_colorMaps[dotNum],
       m_colorMaps[dotNum]->GetRandomColorMapName(m_colorMaps[dotNum]->GetRandomGroup()),
       RandomColorMaps::ALL});
}

void GoomDotsFx::GoomDotsFxImpl::ApplySingle()
{
  m_thereIsOneBuffer = true;
  Update();
}

void GoomDotsFx::GoomDotsFxImpl::ApplyMultiple()
{
  m_thereIsOneBuffer = false;
  Update();
}

void GoomDotsFx::GoomDotsFxImpl::Update()
{
  uint32_t radius = MIN_DOT_SIZE / 2;
  if ((0 == m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom()) || ChangeDotColorsEvent())
  {
    ChangeColors();
    radius = GetRandInRange(radius, (MAX_DOT_SIZE / 2) + 1);
    SetNextCurrentBitmapName();
  }

  const float largeFactor = GetLargeSoundFactor(m_goomInfo.GetSoundInfo());
  const auto speedvarMult80Plus15 =
      static_cast<uint32_t>((m_goomInfo.GetSoundInfo().GetSpeed() * 80.0F) + 15.0F);
  const auto speedvarMult50Plus1 =
      static_cast<uint32_t>((m_goomInfo.GetSoundInfo().GetSpeed() * 50.0F) + 1.0F);

  const float pointWidthDiv2MultLarge = m_pointWidthDiv2 * largeFactor;
  const float pointHeightDiv2MultLarge = m_pointHeightDiv2 * largeFactor;
  const float pointWidthDiv3MultLarge = (m_pointWidthDiv3 + 5.0F) * largeFactor;
  const float pointHeightDiv3MultLarge = (m_pointHeightDiv3 + 5.0F) * largeFactor;
  const float pointWidthMultLarge = static_cast<float>(m_pointWidth) * largeFactor;
  const float pointHeightMultLarge = static_cast<float>(m_pointHeight) * largeFactor;

  const float color0T1 = ((static_cast<float>(m_pointWidth) - 6.0F) * largeFactor) + 5.0F;
  const float color0T2 = ((static_cast<float>(m_pointHeight) - 6.0F) * largeFactor) + 5.0F;
  const float color3T1 = (m_pointHeightDiv3 * largeFactor) + 20.0F;
  const float color3T2 = color3T1;

  const size_t speedvarMult80Plus15Div15 = speedvarMult80Plus15 / 15;
  constexpr float T_MIN = 0.5F;
  constexpr float T_MAX = 1.0F;
  const float t_step = (T_MAX - T_MIN) / static_cast<float>(speedvarMult80Plus15Div15);

  float t = T_MIN;
  for (uint32_t i = 1; i <= speedvarMult80Plus15Div15; ++i)
  {
    m_loopVar += speedvarMult50Plus1;

    const uint32_t loopvarDivI = m_loopVar / i;
    const float iMult10 = 10.0F * static_cast<float>(i);

    const Pixel colors0 = m_colorMapsManagers[0].GetColorMap(m_colorMapIds[0]).GetColor(t);
    const float color0T3 = static_cast<float>(i) * 152.0F;
    const float color0T4 = 128.0F;
    const uint32_t color0Cycle = m_loopVar + (i * 2032);

    const Pixel colors1 = m_colorMapsManagers[1].GetColorMap(m_colorMapIds[1]).GetColor(t);
    const float color1T1 = (pointWidthDiv2MultLarge / static_cast<float>(i)) + iMult10;
    const float color1T2 = (pointHeightDiv2MultLarge / static_cast<float>(i)) + iMult10;
    const float color1T3 = 96.0F;
    const float color1T4 = static_cast<float>(i) * 80.0F;
    const uint32_t color1Cycle = loopvarDivI;

    const Pixel colors2 = m_colorMapsManagers[2].GetColorMap(m_colorMapIds[2]).GetColor(t);
    const float color2T1 = (pointWidthDiv3MultLarge / static_cast<float>(i)) + iMult10;
    const float color2T2 = (pointHeightDiv3MultLarge / static_cast<float>(i)) + iMult10;
    const float color2T3 = static_cast<float>(i) + 122.0F;
    const float color2T4 = 134.0F;
    const uint32_t color2Cycle = loopvarDivI;

    const Pixel colors3 = m_colorMapsManagers[3].GetColorMap(m_colorMapIds[3]).GetColor(t);
    const float color3T3 = 58.0F;
    const float color3T4 = static_cast<float>(i) * 66.0F;
    const uint32_t color3Cycle = loopvarDivI;

    const Pixel colors4 = m_colorMapsManagers[4].GetColorMap(m_colorMapIds[4]).GetColor(t);
    const float color4T1 = (pointWidthMultLarge + iMult10) / static_cast<float>(i);
    const float color4T2 = (pointHeightMultLarge + iMult10) / static_cast<float>(i);
    const float color4T3 = 66.0F;
    const float color4T4 = 74.0F;
    const uint32_t color4Cycle = m_loopVar + (i * 500);

    DotFilter(colors0, color0T1, color0T2, color0T3, color0T4, color0Cycle, radius);
    DotFilter(colors1, color1T1, color1T2, color1T3, color1T4, color1Cycle, radius);
    DotFilter(colors2, color2T1, color2T2, color2T3, color2T4, color2Cycle, radius);
    DotFilter(colors3, color3T1, color3T2, color3T3, color3T4, color3Cycle, radius);
    DotFilter(colors4, color4T1, color4T2, color4T3, color4T4, color4Cycle, radius);

    t += t_step;
  }
}

void GoomDotsFx::GoomDotsFxImpl::SetNextCurrentBitmapName()
{
  if (m_numFlowersInRow > 0)
  {
    ++m_numFlowersInRow;
    if (m_numFlowersInRow > MAX_FLOWERS_IN_ROW)
    {
      m_numFlowersInRow = 0;
    }
    if (ProbabilityOfMInN(1, 3))
    {
      m_currentBitmapName = SmallImageBitmaps::ImageNames::RED_FLOWER;
    }
    else if (ProbabilityOfMInN(1, 3))
    {
      m_currentBitmapName = SmallImageBitmaps::ImageNames::ORANGE_FLOWER;
    }
    else
    {
      m_currentBitmapName = SmallImageBitmaps::ImageNames::WHITE_FLOWER;
    }
  }
  else if (ProbabilityOfMInN(1, 50))
  {
    m_numFlowersInRow = 1;
    SetNextCurrentBitmapName();
  }
  else
  {
    static const Weights<SmallImageBitmaps::ImageNames> s_dotTypes{{
        {SmallImageBitmaps::ImageNames::SPHERE, 50},
        {SmallImageBitmaps::ImageNames::CIRCLE, 20},
        {SmallImageBitmaps::ImageNames::RED_FLOWER, 5},
        {SmallImageBitmaps::ImageNames::ORANGE_FLOWER, 5},
        {SmallImageBitmaps::ImageNames::WHITE_FLOWER, 5},
    }};
    m_currentBitmapName = s_dotTypes.GetRandomWeighted();
  }
}

auto GoomDotsFx::GoomDotsFxImpl::GetLargeSoundFactor(const SoundInfo& soundInfo) -> float
{
  float largeFactor = (soundInfo.GetSpeed() / 150.0F) + (soundInfo.GetVolume() / 1.5F);
  if (largeFactor > 1.5F)
  {
    largeFactor = 1.5F;
  }
  return largeFactor;
}

void GoomDotsFx::GoomDotsFxImpl::DotFilter(const Pixel& color,
                                           const float t1,
                                           const float t2,
                                           const float t3,
                                           const float t4,
                                           const uint32_t cycle,
                                           const uint32_t radius)
{
  const auto xOffset = static_cast<uint32_t>(t1 * std::cos(static_cast<float>(cycle) / t3));
  const auto yOffset = static_cast<uint32_t>(t2 * std::sin(static_cast<float>(cycle) / t4));
  const auto x0 = static_cast<int32_t>((m_goomInfo.GetScreenInfo().width / 2) + xOffset);
  const auto y0 = static_cast<int32_t>((m_goomInfo.GetScreenInfo().height / 2) + yOffset);

  const uint32_t diameter = (2 * radius) + 1; // must be odd
  const auto screenWidthLessDiameter =
      static_cast<int32_t>(m_goomInfo.GetScreenInfo().width - diameter);
  const auto screenHeightLessDiameter =
      static_cast<int32_t>(m_goomInfo.GetScreenInfo().height - diameter);

  if ((x0 < static_cast<int32_t>(diameter)) || (y0 < static_cast<int32_t>(diameter)) ||
      (x0 >= screenWidthLessDiameter) || (y0 >= screenHeightLessDiameter))
  {
    return;
  }

  const auto xMid = x0 + static_cast<int32_t>(radius);
  const auto yMid = y0 + static_cast<int32_t>(radius);
  constexpr float BRIGHTNESS = 3.0F;
  const auto getColor1 = [&]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y,
                             const Pixel& b) -> Pixel {
    // const Pixel newColor = x == xMid && y == yMid ? m_middleColor : color;
    if (0 == b.A())
    {
      return Pixel::BLACK;
    }
    return GetGammaCorrection(BRIGHTNESS, GetColorMultiply(b, color, m_draw.GetAllowOverexposed()));
  };
  const auto getColor2 = [&]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y,
                             [[maybe_unused]] const Pixel& b) -> Pixel {
    if (0 == b.A())
    {
      return Pixel::BLACK;
    }
    return GetGammaCorrection(BRIGHTNESS, color);
  };

  if (m_thereIsOneBuffer || m_useSingleBufferOnly)
  {
    m_draw.Bitmap(xMid, yMid, GetImageBitmap(diameter), getColor1);
  }
  else
  {
    m_draw.Bitmap(xMid, yMid, GetImageBitmap(diameter), {getColor1, getColor2});
  }
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetGammaCorrection(const float brightness,
                                                           const Pixel& color) const -> Pixel
{
  // if constexpr (GAMMA == 1.0F)
  if (1.0F == GAMMA)
  {
    return GetBrighterColor(brightness, color, true);
  }
  return m_gammaCorrect.GetCorrection(brightness, color);
}

} // namespace GOOM
