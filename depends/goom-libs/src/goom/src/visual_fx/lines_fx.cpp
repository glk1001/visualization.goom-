/*
 *  Goom
 *  Copyright (c) 2000-2003 iOS-software. All rights reserved.
 *
 *  - converted to C++17 2021-02-01 (glk)
 */

#include "lines_fx.h"

//#undef NO_LOGGING

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "fx_utils/dot_drawer.h"
#include "fx_utils/lines.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "sound_info.h"
#include "utils/enumutils.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#undef NDEBUG
#include <cassert>
#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

namespace GOOM::VISUAL_FX
{

using COLOR::GammaCorrection;
using COLOR::GetAllSlimMaps;
using COLOR::GetBrighterColor;
using COLOR::GetIncreasedChroma;
using COLOR::GetLightenedColor;
using COLOR::GetSimpleColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using COLOR::RandomColorMapsManager;
using COLOR::SimpleColors;
using COLOR::COLOR_DATA::ColorMapName;
using DRAW::IGoomDraw;
using FX_UTILS::DotDrawer;
using FX_UTILS::DotSizes;
using FX_UTILS::GetCircularLinePoints;
using FX_UTILS::GetHorizontalLinePoints;
using FX_UTILS::GetVerticalLinePoints;
using FX_UTILS::LinePoint;
using FX_UTILS::SmoothTheCircleJoinAtEnds;
using UTILS::NUM;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::FloatsEqual;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::SMALL_FLOAT;

static constexpr float MIN_MAX_NORMALIZED_PEAK = 100.0F;
static constexpr float MAX_MAX_NORMALIZED_PEAK = 400.0F;

class LinesFx::LinesImpl
{
public:
  // construit un effet de line (une ligne horitontale pour commencer)
  // builds a line effect (a horizontal line to start with)
  LinesImpl(const FxHelper& fxHelper,
            const SmallImageBitmaps& smallBitmaps,
            LineType srceLineType,
            float srceParam,
            const Pixel& srceColor,
            LineType destLineType,
            float destParam,
            const Pixel& destColor);

  void SetWeightedColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps);

  void Start();

  [[nodiscard]] auto GetLineColorPower() const -> float;
  void SetLineColorPower(float val);

  [[nodiscard]] auto CanResetDestLine() const -> bool;
  void ResetDestLine(LineType newLineType,
                     float newParam,
                     float newAmplitude,
                     const Pixel& newColor);

  void DrawLines(const AudioSamples::SampleArray& soundData,
                 const AudioSamples::MaxMinValues& soundMinMax);

  void Finish();

  [[nodiscard]] auto GetRandomLineColor() const -> Pixel;
  [[nodiscard]] static auto GetBlackLineColor() -> Pixel;
  [[nodiscard]] static auto GetGreenLineColor() -> Pixel;
  [[nodiscard]] static auto GetRedLineColor() -> Pixel;

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;

  std::shared_ptr<RandomColorMaps> m_colorMaps;
  RandomColorMapsManager m_randomColorMapsManager{};
  uint32_t m_currentColorMapID{};
  float m_currentBrightness = 1.0F;

  static constexpr float GAMMA = 1.0F / 2.0F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.1F;
  GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
  [[nodiscard]] auto GetGammaCorrection(float brightness, const Pixel& color) const -> Pixel;

  std::vector<LinePoint> m_srcePoints{};
  std::vector<LinePoint> m_srcePointsCopy{};
  LineType m_srceLineType;
  const float m_srceParam;
  std::vector<LinePoint> m_destPoints{};
  LineType m_destLineType = LineType::CIRCLE;
  float m_destParam;
  static constexpr float LINE_LERP_FINISHED_VAL = 1.1F;
  static constexpr float LINE_LERP_INC = 1.0F / static_cast<float>(MIN_LINE_DURATION - 1);
  float m_lineLerpParam = 0.0F;
  void MoveSrceLineCloserToDest();
  [[nodiscard]] auto GetFreshLine(LineType lineType, float lineParam) const
      -> std::vector<LinePoint>;

  // pour l'instant je stocke la couleur a terme, on stockera le mode couleur et l'on animera
  Pixel m_srceColor = Pixel::BLACK;
  Pixel m_destColor = Pixel::BLACK;
  bool m_useLineColor = true;
  bool m_useIncreasedChroma = false;
  float m_lineColorPower = 0.0F;
  float m_lineColorPowerIncrement = 0.0F;
  [[nodiscard]] auto GetRandomColorMap() const -> const IColorMap&;
  [[nodiscard]] auto GetFinalColor(const Pixel& color) const -> Pixel;
  [[nodiscard]] auto GetFinalLineColor(const Pixel& color) const -> Pixel;
  void UpdateColorInfo();

  // This factor gives the maximum height to the peaks of the audio samples lines.
  // This value seems pleasing.
  float m_maxNormalizedPeak = MIN_MAX_NORMALIZED_PEAK;

  DotDrawer m_dotDrawer;
  float m_newLinePeakAmplitude = 1.0F;
  float m_linePeakAmplitude = 1.0F;
  float m_audioRange = 0.0F;
  float m_minAudioValue = 0.0F;
  struct PointAndColor
  {
    Point2dInt point;
    Pixel color;
  };
  [[nodiscard]] auto GetAudioPoints(const Pixel& lineColor,
                                    const std::vector<float>& audioData) const
      -> std::vector<PointAndColor>;
  [[nodiscard]] auto GetNextPointData(const LinePoint& pt,
                                      const Pixel& mainColor,
                                      const Pixel& randColor,
                                      float dataVal) const -> PointAndColor;

  [[nodiscard]] auto GetMainColor(const Pixel& lineColor, float t) const -> Pixel;
  void DrawFlatLine(const Pixel& lineColor);
};

LinesFx::LinesFx(const FxHelper& fxHelper,
                 const SmallImageBitmaps& smallBitmaps,
                 const LineType srceLineType,
                 const float srceParam,
                 const Pixel& srceColor,
                 const LineType destLineType,
                 const float destParam,
                 const Pixel& destColor) noexcept
  : m_fxImpl{spimpl::make_unique_impl<LinesImpl>(fxHelper,
                                                 smallBitmaps,
                                                 srceLineType,
                                                 srceParam,
                                                 srceColor,
                                                 destLineType,
                                                 destParam,
                                                 destColor)}
{
}

auto LinesFx::GetFxName() const -> std::string
{
  return "lines";
}

void LinesFx::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedColorMaps(weightedMaps);
}

void LinesFx::Start()
{
  m_fxImpl->Start();
}

void LinesFx::Finish()
{
  m_fxImpl->Finish();
}

auto LinesFx::GetBlackLineColor() -> Pixel
{
  return LinesFx::LinesImpl::GetBlackLineColor();
}

auto LinesFx::GetGreenLineColor() -> Pixel
{
  return LinesFx::LinesImpl::GetGreenLineColor();
}

auto LinesFx::GetRedLineColor() -> Pixel
{
  return LinesFx::LinesImpl::GetRedLineColor();
}

auto LinesFx::GetRandomLineColor() const -> Pixel
{
  return m_fxImpl->GetRandomLineColor();
}

auto LinesFx::GetLineColorPower() const -> float
{
  return m_fxImpl->GetLineColorPower();
}

void LinesFx::SetLineColorPower(const float val)
{
  m_fxImpl->SetLineColorPower(val);
}

auto LinesFx::CanResetDestLine() const -> bool
{
  return m_fxImpl->CanResetDestLine();
}

void LinesFx::ResetDestLine(const LineType newLineType,
                            const float newParam,
                            const float newAmplitude,
                            const Pixel& newColor)
{
  m_fxImpl->ResetDestLine(newLineType, newParam, newAmplitude, newColor);
}

void LinesFx::DrawLines(const AudioSamples::SampleArray& soundData,
                        const AudioSamples::MaxMinValues& soundMinMax)
{
  m_fxImpl->DrawLines(soundData, soundMinMax);
}

// clang-format off
static constexpr float MIN_DOT_SIZE01_WEIGHT = 100.0F;
static constexpr float MIN_DOT_SIZE02_WEIGHT =  50.0F;
static constexpr float MIN_DOT_SIZE03_WEIGHT =  20.0F;
static constexpr float MIN_DOT_SIZE04_WEIGHT =  10.0F;

static constexpr float NORMAL_DOT_SIZE01_WEIGHT = 50.0F;
static constexpr float NORMAL_DOT_SIZE02_WEIGHT = 20.0F;
static constexpr float NORMAL_DOT_SIZE03_WEIGHT = 20.0F;
static constexpr float NORMAL_DOT_SIZE04_WEIGHT = 10.0F;
static constexpr float NORMAL_DOT_SIZE05_WEIGHT = 10.0F;
static constexpr float NORMAL_DOT_SIZE06_WEIGHT = 10.0F;
static constexpr float NORMAL_DOT_SIZE07_WEIGHT = 10.0F;
static constexpr float NORMAL_DOT_SIZE08_WEIGHT = 10.0F;
// clang-format on

LinesFx::LinesImpl::LinesImpl(const FxHelper& fxHelper,
                              const SmallImageBitmaps& smallBitmaps,
                              const LineType srceLineType,
                              const float srceParam,
                              const Pixel& srceColor,
                              const LineType destLineType,
                              const float destParam,
                              const Pixel& destColor)
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_colorMaps{GetAllSlimMaps(m_goomRand)},
    m_srcePoints(AudioSamples::AUDIO_SAMPLE_LEN),
    m_srcePointsCopy(AudioSamples::AUDIO_SAMPLE_LEN),
    m_srceLineType{srceLineType},
    m_srceParam{srceParam},
    m_destPoints(AudioSamples::AUDIO_SAMPLE_LEN),
    m_destLineType{destLineType},
    m_destParam{destParam},
    m_srceColor{srceColor},
    m_destColor{destColor},
    m_dotDrawer{
        m_draw,
        m_goomRand,
        smallBitmaps,
        // clang-format off
        // min dot sizes
        {
            m_goomRand,
            {
                {DotSizes::DOT_SIZE01, MIN_DOT_SIZE01_WEIGHT},
                {DotSizes::DOT_SIZE02, MIN_DOT_SIZE02_WEIGHT},
                {DotSizes::DOT_SIZE03, MIN_DOT_SIZE03_WEIGHT},
                {DotSizes::DOT_SIZE04, MIN_DOT_SIZE04_WEIGHT},
            }
        },
        // normal dot sizes
        {
            m_goomRand,
            {
                {DotSizes::DOT_SIZE01, NORMAL_DOT_SIZE01_WEIGHT},
                {DotSizes::DOT_SIZE02, NORMAL_DOT_SIZE02_WEIGHT},
                {DotSizes::DOT_SIZE03, NORMAL_DOT_SIZE03_WEIGHT},
                {DotSizes::DOT_SIZE04, NORMAL_DOT_SIZE04_WEIGHT},
                {DotSizes::DOT_SIZE05, NORMAL_DOT_SIZE05_WEIGHT},
                {DotSizes::DOT_SIZE06, NORMAL_DOT_SIZE06_WEIGHT},
                {DotSizes::DOT_SIZE07, NORMAL_DOT_SIZE07_WEIGHT},
                {DotSizes::DOT_SIZE08, NORMAL_DOT_SIZE08_WEIGHT},
            }
        }
        // clang-format on
    }
{
}

auto LinesFx::LinesImpl::GetRandomColorMap() const -> const IColorMap&
{
  assert(m_colorMaps);
  return m_colorMaps->GetRandomColorMap(m_colorMaps->GetRandomGroup());
}

void LinesFx::LinesImpl::Start()
{
  UpdateColorInfo();
  m_srcePoints = GetFreshLine(m_srceLineType, m_srceParam);
  m_srcePointsCopy = m_srcePoints;
  ResetDestLine(m_destLineType, m_destParam, 1.0F, m_destColor);
}

void LinesFx::LinesImpl::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_colorMaps = weightedMaps;
  m_currentColorMapID = m_randomColorMapsManager.AddColorMapInfo(
      {m_colorMaps, ColorMapName::_NULL, RandomColorMaps::ALL_COLOR_MAP_TYPES});
}

void LinesFx::LinesImpl::Finish()
{
  // nothing to do
}

inline void LinesFx::LinesImpl::UpdateColorInfo()
{
  m_randomColorMapsManager.ChangeAllColorMapsNow();

  static constexpr float PROB_USE_LINE_COLOR = 0.5F;
  m_useLineColor = m_goomRand.ProbabilityOf(PROB_USE_LINE_COLOR);
  static constexpr float PROB_USE_INCREASED_CHROMA = 0.3F;
  m_useIncreasedChroma = m_goomRand.ProbabilityOf(PROB_USE_INCREASED_CHROMA);
}

inline auto LinesFx::LinesImpl::GetFreshLine(const LineType lineType, const float lineParam) const
    -> std::vector<LinePoint>
{
  switch (lineType)
  {
    case LineType::H_LINE:
      return GetHorizontalLinePoints(AudioSamples::AUDIO_SAMPLE_LEN,
                                     m_goomInfo.GetScreenInfo().width, lineParam);
    case LineType::V_LINE:
      return GetVerticalLinePoints(AudioSamples::AUDIO_SAMPLE_LEN,
                                   m_goomInfo.GetScreenInfo().height, lineParam);
    case LineType::CIRCLE:
      return GetCircularLinePoints(AudioSamples::AUDIO_SAMPLE_LEN, m_goomInfo.GetScreenInfo().width,
                                   m_goomInfo.GetScreenInfo().height, lineParam);
    default:
      throw std::logic_error("Unknown LineType enum.");
  }
}

void LinesFx::LinesImpl::MoveSrceLineCloserToDest()
{
  m_lineLerpParam += LINE_LERP_INC;
  const float t = std::min(1.0F, m_lineLerpParam);
  for (uint32_t i = 0; i < AudioSamples::AUDIO_SAMPLE_LEN; ++i)
  {
    m_srcePoints[i].point = lerp(m_srcePointsCopy[i].point, m_destPoints[i].point, t);
    m_srcePoints[i].angle = STD20::lerp(m_srcePointsCopy[i].angle, m_destPoints[i].angle, t);
  }
  if (m_lineLerpParam >= 1.0F)
  {
    m_srceLineType = m_destLineType;
    static constexpr float MIN_BRIGHTNESS = 2.5F;
    static constexpr float MAX_BRIGHTNESS = 4.0F;
    m_currentBrightness = m_goomRand.GetRandInRange(MIN_BRIGHTNESS, MAX_BRIGHTNESS);
  }

  assert(m_srceLineType != LineType::CIRCLE || m_lineLerpParam < 1.0F ||
         (FloatsEqual(m_srcePoints[0].point.x,
                      m_srcePoints[AudioSamples::AUDIO_SAMPLE_LEN - 1].point.x) &&
          FloatsEqual(m_srcePoints[0].point.y,
                      m_srcePoints[AudioSamples::AUDIO_SAMPLE_LEN - 1].point.y)));

  static constexpr float COLOR_MIX_AMOUNT = 1.0F / 64.0F;
  m_srceColor = IColorMap::GetColorMix(m_srceColor, m_destColor, COLOR_MIX_AMOUNT);

  static constexpr float MIN_POW_INC = 0.03F;
  static constexpr float MAX_POW_INC = 0.10F;
  static constexpr float MIN_POWER = 1.1F;
  static constexpr float MAX_POWER = 17.5F;
  m_lineColorPower += m_lineColorPowerIncrement;
  if (m_lineColorPower < MIN_POWER)
  {
    m_lineColorPower = MIN_POWER;
    m_lineColorPowerIncrement = m_goomRand.GetRandInRange(MIN_POW_INC, MAX_POW_INC);
  }
  if (m_lineColorPower > MAX_POWER)
  {
    m_lineColorPower = MAX_POWER;
    m_lineColorPowerIncrement = -m_goomRand.GetRandInRange(MIN_POW_INC, MAX_POW_INC);
  }

  static constexpr float AMP_MIX_AMOUNT = 0.01F;
  m_linePeakAmplitude = STD20::lerp(m_linePeakAmplitude, m_newLinePeakAmplitude, AMP_MIX_AMOUNT);
}

auto LinesFx::LinesImpl::CanResetDestLine() const -> bool
{
  return m_lineLerpParam > LINE_LERP_FINISHED_VAL;
}

void LinesFx::LinesImpl::ResetDestLine(const LineType newLineType,
                                       const float newParam,
                                       const float newAmplitude,
                                       const Pixel& newColor)
{
  UpdateColorInfo();

  m_destPoints = GetFreshLine(newLineType, m_destParam);
  m_destLineType = newLineType;
  m_destParam = newParam;
  m_newLinePeakAmplitude = newAmplitude;
  m_destColor = newColor;
  m_lineLerpParam = 0.0;
  static constexpr float MIN_BRIGHTNESS = 1.5F;
  static constexpr float MAX_BRIGHTNESS = 3.0F;
  m_currentBrightness = m_goomRand.GetRandInRange(MIN_BRIGHTNESS, MAX_BRIGHTNESS);
  m_dotDrawer.ChangeDotSizes();
  m_maxNormalizedPeak = m_goomRand.GetRandInRange(MIN_MAX_NORMALIZED_PEAK, MAX_MAX_NORMALIZED_PEAK);

  m_srcePointsCopy = m_srcePoints;
}

inline auto LinesFx::LinesImpl::GetLineColorPower() const -> float
{
  return m_lineColorPower;
}

inline void LinesFx::LinesImpl::SetLineColorPower(const float val)
{
  m_lineColorPower = val;
}


inline auto LinesFx::LinesImpl::GetBlackLineColor() -> Pixel
{
  return GetSimpleColor(SimpleColors::BLACK);
}

inline auto LinesFx::LinesImpl::GetGreenLineColor() -> Pixel
{
  return GetSimpleColor(SimpleColors::VERT);
}

inline auto LinesFx::LinesImpl::GetRedLineColor() -> Pixel
{
  return GetSimpleColor(SimpleColors::RED);
}

auto LinesFx::LinesImpl::GetRandomLineColor() const -> Pixel
{
  if (constexpr float PROB_LINE_COLOR = 0.02F; m_goomRand.ProbabilityOf(PROB_LINE_COLOR))
  {
    return GetSimpleColor(static_cast<SimpleColors>(m_goomRand.GetNRand(NUM<SimpleColors>)));
  }
  return RandomColorMaps{m_goomRand}.GetRandomColor(GetRandomColorMap(), 0.0F, 1.0F);
}

inline auto LinesFx::LinesImpl::GetFinalColor(const Pixel& color) const -> Pixel
{
  if (!m_useIncreasedChroma)
  {
    return color;
  }
  return GetIncreasedChroma(color);
}

inline auto LinesFx::LinesImpl::GetFinalLineColor(const Pixel& color) const -> Pixel
{
  return GetLightenedColor(color, m_lineColorPower);
}

void LinesFx::LinesImpl::DrawLines(const AudioSamples::SampleArray& soundData,
                                   const AudioSamples::MaxMinValues& soundMinMax)
{
  static constexpr size_t LAST_POINT_INDEX = AudioSamples::AUDIO_SAMPLE_LEN - 1;

  assert(m_srceLineType != LineType::CIRCLE || m_lineLerpParam < 1.0F ||
         (FloatsEqual(m_srcePoints[0].point.x, m_srcePoints[LAST_POINT_INDEX].point.x) &&
          FloatsEqual(m_srcePoints[0].point.y, m_srcePoints[LAST_POINT_INDEX].point.y)));

  const Pixel lineColor = GetFinalLineColor(m_srceColor);

  m_audioRange = soundMinMax.maxVal - soundMinMax.minVal;
  assert(m_audioRange >= 0.0F);
  m_minAudioValue = soundMinMax.minVal;

  if (m_audioRange < SMALL_FLOAT)
  {
    // No range - flatline audio
    DrawFlatLine(lineColor);
    MoveSrceLineCloserToDest();
    return;
  }

  static constexpr uint8_t LINE_THICKNESS = 1;
  const std::vector<PointAndColor> audioPoints = GetAudioPoints(lineColor, soundData);

  Point2dInt point1 = audioPoints[0].point;
  Point2dInt point2{};

  for (size_t i = 1; i < audioPoints.size(); ++i)
  {
    const PointAndColor& nextPointData = audioPoints[i];

    point2 = nextPointData.point;
    const Pixel modColor = GetFinalColor(nextPointData.color);
    const std::vector<Pixel> colors = {lineColor, modColor};

    m_draw.Line(point1, point2, colors, LINE_THICKNESS);

    static constexpr float DOT_BRIGHTNESS = 1.5F;
    m_dotDrawer.DrawDot(point2, colors, DOT_BRIGHTNESS);

    point1 = point2;
  }

  MoveSrceLineCloserToDest();

  m_dotDrawer.ChangeDotSizes();
}

void LinesFx::LinesImpl::DrawFlatLine(const Pixel& lineColor)
{
  const LinePoint& pt0 = m_srcePoints[0];
  const LinePoint& ptN = m_srcePoints[AudioSamples::AUDIO_SAMPLE_LEN - 1];
  const std::vector<Pixel> colors = {lineColor, lineColor};

  m_draw.Line(pt0.point.ToInt(), ptN.point.ToInt(), colors, 1);
}

auto LinesFx::LinesImpl::GetAudioPoints(const Pixel& lineColor,
                                        const std::vector<float>& audioData) const
    -> std::vector<PointAndColor>
{
  const Pixel randColor = GetRandomLineColor();

  static constexpr float T_STEP = 1.0F / static_cast<float>(AudioSamples::AUDIO_SAMPLE_LEN - 1);
  static constexpr float HALFWAY_T = 0.5F;
  float currentTStep = T_STEP;
  float t = 0.0;

  std::vector<PointAndColor> audioPoints{};
  audioPoints.reserve(audioData.size());

  for (size_t i = 0; i < audioData.size(); ++i)
  {
    audioPoints.emplace_back(
        GetNextPointData(m_srcePoints[i], GetMainColor(lineColor, t), randColor, audioData[i]));

    if (t >= HALFWAY_T)
    {
      currentTStep = -T_STEP;
    }
    t += currentTStep;
  }

  if ((m_srceLineType == LineType::CIRCLE) && (m_lineLerpParam >= 1.0F))
  {
    // It's a complete circle - lerp the last few points to nicely join back to start.
    static constexpr size_t NUM_POINTS_TO_LERP = 50;
    SmoothTheCircleJoinAtEnds(audioPoints, NUM_POINTS_TO_LERP);
  }

  return audioPoints;
}

auto LinesFx::LinesImpl::GetNextPointData(const LinePoint& pt,
                                          const Pixel& mainColor,
                                          const Pixel& randColor,
                                          const float dataVal) const -> PointAndColor
{
  assert(m_goomInfo.GetSoundInfo().GetAllTimesMinVolume() <= (dataVal + SMALL_FLOAT));
  assert(m_minAudioValue <= (dataVal + SMALL_FLOAT));
  assert(dataVal <= (m_minAudioValue + m_audioRange + SMALL_FLOAT));

  const float tData = (dataVal - m_minAudioValue) / m_audioRange;
  assert(0.0F <= tData && tData <= 1.0F);

  const float cosAngle = std::cos(pt.angle);
  const float sinAngle = std::sin(pt.angle);
  const float normalizedDataVal = m_maxNormalizedPeak * tData;
  assert(normalizedDataVal >= 0.0F);
  const Point2dInt nextPointData{
      static_cast<int>(pt.point.x + (m_linePeakAmplitude * cosAngle * normalizedDataVal)),
      static_cast<int>(pt.point.y + (m_linePeakAmplitude * sinAngle * normalizedDataVal))};

  const float brightness = m_currentBrightness * tData;
  const Pixel modColor =
      GetGammaCorrection(brightness, IColorMap::GetColorMix(mainColor, randColor, tData));

  return {nextPointData, modColor};
}

inline auto LinesFx::LinesImpl::GetGammaCorrection(const float brightness, const Pixel& color) const
    -> Pixel
{
  if constexpr (1.0F == GAMMA)
  {
    return GetBrighterColor(brightness, color);
  }
  return m_gammaCorrect.GetCorrection(brightness, color);
}

inline auto LinesFx::LinesImpl::GetMainColor(const Pixel& lineColor, const float t) const -> Pixel
{
  if (m_useLineColor)
  {
    return lineColor;
  }
  return m_randomColorMapsManager.GetColorMap(m_currentColorMapID).GetColor(t);
}

} // namespace GOOM::VISUAL_FX
