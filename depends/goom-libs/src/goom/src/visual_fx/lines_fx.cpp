/*
 *  Goom
 *  Copyright (c) 2000-2003 iOS-software. All rights reserved.
 *
 *  - converted to C++14 2021-02-01 (glk)
 */

#include "lines_fx.h"

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "fx_helpers.h"
#include "fx_utils/dot_drawer.h"
#include "goom/logging_control.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
//#undef NO_LOGGING
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "sound_info.h"
#include "utils/goom_rand_base.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/mathutils.h"
#include "v2d.h"

#undef NDEBUG
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
#else
namespace GOOM::VISUAL_FX
{
#endif

using COLOR::GammaCorrection;
using COLOR::GetAllSlimMaps;
using COLOR::GetBrighterColor;
using COLOR::GetLightenedColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using UTILS::floats_equal;
using UTILS::IGoomRand;
using UTILS::m_half_pi;
using UTILS::m_pi;
using UTILS::m_two_pi;
using UTILS::SMALL_FLOAT;
using UTILS::SmallImageBitmaps;

constexpr float MIN_MAX_NORMALIZED_PEAK = 100.0F;
constexpr float MAX_MAX_NORMALIZED_PEAK = 300.0F;

class LinesFx::LinesImpl
{
public:
  // construit un effet de line (une ligne horitontale pour commencer)
  // builds a line effect (a horizontal line to start with)
  LinesImpl(const FxHelpers& fxHelpers,
            const SmallImageBitmaps& smallBitmaps,
            LineType srceLineType,
            float srceParam,
            const Pixel& srceColor,
            LineType destLineType,
            float destParam,
            const Pixel& destColor);

  void SetWeightedColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps);

  void Start();

  [[nodiscard]] auto GetRandomLineColor() const -> Pixel;

  [[nodiscard]] auto GetPower() const -> float;
  void SetPower(float val);

  [[nodiscard]] auto CanResetDestLine() const -> bool;

  void ResetDestLine(LineType newLineType,
                     float newParam,
                     float newAmplitude,
                     const Pixel& newColor);

  void DrawLines(const AudioSamples::SampleArray& soundData,
                 const AudioSamples::MaxMinValues& soundMinMax);

  void Finish();

  [[nodiscard]] static auto GetBlackLineColor() -> Pixel;
  [[nodiscard]] static auto GetGreenLineColor() -> Pixel;
  [[nodiscard]] static auto GetRedLineColor() -> Pixel;

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  IGoomRand& m_goomRand;
  std::shared_ptr<RandomColorMaps> m_colorMaps;
  std::reference_wrapper<const IColorMap> m_currentColorMap;
  static constexpr float GAMMA = 1.0F / 1.0F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.1F;
  GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
  [[nodiscard]] auto GetGammaCorrection(float brightness, const Pixel& color) const -> Pixel;
  float m_currentBrightness = 1.0F;

  struct LinePoint
  {
    float x;
    float y;
    float angle;
  };
  std::vector<LinePoint> m_srcePoints{};
  std::vector<LinePoint> m_srcePointsCopy{};
  LineType m_srcLineType;
  const float m_srceParam;
  std::vector<LinePoint> m_destPoints{};
  LineType m_destLineType = LineType::CIRCLE;
  float m_destParam;
  static constexpr float LINE_LERP_FINISHED_VAL = 1.1F;
  static constexpr float LINE_LERP_INC = 1.0F / static_cast<float>(MIN_LINE_DURATION - 1);
  float m_lineLerpFactor = 0.0F;
  bool m_useLineColor = true;
  void GenerateLinePoints(LineType lineType, float lineParam, std::vector<LinePoint>& line);
  [[nodiscard]] auto GetRandomColorMap() const -> const IColorMap&;

  // pour l'instant je stocke la couleur a terme, on stockera le mode couleur et l'on animera
  Pixel m_srceColor{};
  Pixel m_destColor{};

  FX_UTILS::DotDrawer m_dotDrawer;

  float m_power = 0.0F;
  float m_powerIncrement = 0.0F;
  // This factor gives height to the audio samples lines. This value seems pleasing.
  float m_maxNormalizedPeak = MIN_MAX_NORMALIZED_PEAK;

  float m_newAmplitude = 1.0F;
  float m_amplitude = 1.0F;
  float m_audioRange = 0.0F;
  float m_minAudioValue = 0.0F;

  struct PointAndColor
  {
    V2dInt point;
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
  void MoveSrceLineCloserToDest();
  static void SmoothCircleJoin(std::vector<PointAndColor>& audioPoints);
};

LinesFx::LinesFx(const FxHelpers& fxHelpers,
                 const SmallImageBitmaps& smallBitmaps,
                 const LineType srceLineType,
                 const float srceParam,
                 const Pixel& srceColor,
                 const LineType destLineType,
                 const float destParam,
                 const Pixel& destColor) noexcept
  : m_fxImpl{spimpl::make_unique_impl<LinesImpl>(fxHelpers,
                                                 smallBitmaps,
                                                 srceLineType,
                                                 srceParam,
                                                 srceColor,
                                                 destLineType,
                                                 destParam,
                                                 destColor)}
{
}

void LinesFx::SetWeightedColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps)
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

auto LinesFx::GetPower() const -> float
{
  return m_fxImpl->GetPower();
}

void LinesFx::SetPower(const float val)
{
  m_fxImpl->SetPower(val);
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

LinesFx::LinesImpl::LinesImpl(const FxHelpers& fxHelpers,
                              const SmallImageBitmaps& smallBitmaps,
                              const LineType srceLineType,
                              const float srceParam,
                              const Pixel& srceColor,
                              const LineType destLineType,
                              const float destParam,
                              const Pixel& destColor)
  : m_draw{fxHelpers.GetDraw()},
    m_goomInfo{fxHelpers.GetGoomInfo()},
    m_goomRand{fxHelpers.GetGoomRand()},
    m_colorMaps{GetAllSlimMaps(m_goomRand)},
    m_currentColorMap{GetRandomColorMap()},
    m_srcePoints(AudioSamples::AUDIO_SAMPLE_LEN),
    m_srcePointsCopy(AudioSamples::AUDIO_SAMPLE_LEN),
    m_srcLineType{srceLineType},
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
                {1, 100},
                {3, 50},
                {5, 20},
                {7, 10},
            }
        },
        // normal dot sizes
        {
            m_goomRand,
            {
                { 1, 50},
                { 3, 20},
                { 5, 20},
                { 7, 10},
                { 9, 10},
                {11, 10},
                {13, 10},
                {15, 10},
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
  GenerateLinePoints(m_srcLineType, m_srceParam, m_srcePoints);
  m_srcePointsCopy = m_srcePoints;
  ResetDestLine(m_destLineType, m_destParam, 1.0F, m_destColor);
}

void LinesFx::LinesImpl::SetWeightedColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_colorMaps = weightedMaps;
}

void LinesFx::LinesImpl::Finish()
{
  // nothing to do
}

void LinesFx::LinesImpl::GenerateLinePoints(const LineType lineType,
                                            const float lineParam,
                                            std::vector<LinePoint>& line)
{
  m_currentColorMap = GetRandomColorMap();
  m_useLineColor = m_goomRand.ProbabilityOfMInN(10, 20);

  switch (lineType)
  {
    case LineType::H_LINE:
    {
      const float xStep = static_cast<float>(m_goomInfo.GetScreenInfo().width - 1) /
                          static_cast<float>(AudioSamples::AUDIO_SAMPLE_LEN - 1);
      float x = 0;
      for (auto& pt : line)
      {
        pt.angle = m_half_pi;
        pt.x = x;
        pt.y = lineParam;

        x += xStep;
      }
      return;
    }
    case LineType::V_LINE:
    {
      const float yStep = static_cast<float>(m_goomInfo.GetScreenInfo().height - 1) /
                          static_cast<float>(AudioSamples::AUDIO_SAMPLE_LEN - 1);
      float y = 0;
      for (auto& pt : line)
      {
        pt.angle = 0;
        pt.x = lineParam;
        pt.y = y;

        y += yStep;
      }
      return;
    }
    case LineType::CIRCLE:
    {
      const float cx = 0.5F * static_cast<float>(m_goomInfo.GetScreenInfo().width);
      const float cy = 0.5F * static_cast<float>(m_goomInfo.GetScreenInfo().height);
      // Make sure the circle joins at each end so use symmetry about x-axis.
      //static_assert(0 == (AudioSamples::AUDIO_SAMPLE_LEN % 2), "AUDIO_SAMPLE_LEN must divide by 2");
      assert(0 == (AudioSamples::AUDIO_SAMPLE_LEN % 2));
      const float angleStep =
          m_pi / ((0.5F * static_cast<float>(AudioSamples::AUDIO_SAMPLE_LEN)) - 1.0F);
      float angle = 0;
      for (size_t i = 0; i < (AudioSamples::AUDIO_SAMPLE_LEN / 2); ++i)
      {
        line[i].angle = angle;
        line[i].x = cx + (lineParam * std::cos(angle));
        line[i].y = cy + (lineParam * std::sin(angle));
        angle += angleStep;
      }
      size_t j = AudioSamples::AUDIO_SAMPLE_LEN - 1;
      for (size_t i = 0; i < (AudioSamples::AUDIO_SAMPLE_LEN / 2); ++i)
      {
        line[j].angle = m_two_pi - line[i].angle;
        line[j].x = line[i].x;
        line[j].y = (2.0F * cy) - line[i].y;
        --j;
      }

      assert(floats_equal(line[0].x, line[line.size() - 1].x));
      assert(floats_equal(line[0].y, line[line.size() - 1].y));

      return;
    }
    default:
      throw std::logic_error("Unknown LineType enum.");
  }
}

void LinesFx::LinesImpl::MoveSrceLineCloserToDest()
{
  m_lineLerpFactor += LINE_LERP_INC;
  const float t = std::min(1.0F, m_lineLerpFactor);
  for (uint32_t i = 0; i < AudioSamples::AUDIO_SAMPLE_LEN; ++i)
  {
    m_srcePoints[i].x = stdnew::lerp(m_srcePointsCopy[i].x, m_destPoints[i].x, t);
    m_srcePoints[i].y = stdnew::lerp(m_srcePointsCopy[i].y, m_destPoints[i].y, t);
    m_srcePoints[i].angle = stdnew::lerp(m_srcePointsCopy[i].angle, m_destPoints[i].angle, t);
  }
  if (m_lineLerpFactor >= 1.0F)
  {
    m_srcLineType = m_destLineType;
    m_currentBrightness = m_goomRand.GetRandInRange(2.0F, 3.5F);
  }

  assert(m_srcLineType != LineType::CIRCLE || m_lineLerpFactor < 1.0F ||
         (floats_equal(m_srcePoints[0].x, m_srcePoints[AudioSamples::AUDIO_SAMPLE_LEN - 1].x) &&
          floats_equal(m_srcePoints[0].y, m_srcePoints[AudioSamples::AUDIO_SAMPLE_LEN - 1].y)));

  constexpr float COLOR_MIX_AMOUNT = 1.0F / 64.0F;
  m_srceColor = IColorMap::GetColorMix(m_srceColor, m_destColor, COLOR_MIX_AMOUNT);

  constexpr float MIN_POW_INC = 0.03F;
  constexpr float MAX_POW_INC = 0.10F;
  constexpr float MIN_POWER = 1.1F;
  constexpr float MAX_POWER = 17.5F;
  m_power += m_powerIncrement;
  if (m_power < MIN_POWER)
  {
    m_power = MIN_POWER;
    m_powerIncrement = m_goomRand.GetRandInRange(MIN_POW_INC, MAX_POW_INC);
  }
  if (m_power > MAX_POWER)
  {
    m_power = MAX_POWER;
    m_powerIncrement = -m_goomRand.GetRandInRange(MIN_POW_INC, MAX_POW_INC);
  }

  constexpr float AMP_MIX_AMOUNT = 0.01F;
  m_amplitude = stdnew::lerp(m_amplitude, m_newAmplitude, AMP_MIX_AMOUNT);
}

auto LinesFx::LinesImpl::CanResetDestLine() const -> bool
{
  return m_lineLerpFactor > LINE_LERP_FINISHED_VAL;
}

void LinesFx::LinesImpl::ResetDestLine(const LineType newLineType,
                                       const float newParam,
                                       const float newAmplitude,
                                       const Pixel& newColor)
{
  GenerateLinePoints(newLineType, m_destParam, m_destPoints);

  m_destLineType = newLineType;
  m_destParam = newParam;
  m_newAmplitude = newAmplitude;
  m_destColor = newColor;
  m_lineLerpFactor = 0.0;
  m_currentBrightness = m_goomRand.GetRandInRange(1.0F, 2.5F);
  m_dotDrawer.ChangeDotSizes();
  m_maxNormalizedPeak = m_goomRand.GetRandInRange(MIN_MAX_NORMALIZED_PEAK, MAX_MAX_NORMALIZED_PEAK);

  m_srcePointsCopy = m_srcePoints;
}

inline auto LinesFx::LinesImpl::GetPower() const -> float
{
  return m_power;
}

inline void LinesFx::LinesImpl::SetPower(const float val)
{
  m_power = val;
}

// les modes couleur possible (si tu mets un autre c'est noir)
constexpr uint32_t GML_BLEUBLANC = 0;
constexpr uint32_t GML_RED = 1;
constexpr uint32_t GML_ORANGE_V = 2;
constexpr uint32_t GML_ORANGE_J = 3;
constexpr uint32_t GML_VERT = 4;
constexpr uint32_t GML_BLEU = 5;
constexpr uint32_t GML_BLACK = 6;

inline auto GetColor(const int mode) -> Pixel
{
  switch (mode)
  {
    case GML_RED:
      return Pixel{230, 120, 18, MAX_ALPHA};
    case GML_ORANGE_J:
      return Pixel{120, 252, 18, MAX_ALPHA};
    case GML_ORANGE_V:
      return Pixel{160, 236, 40, MAX_ALPHA};
    case GML_BLEUBLANC:
      return Pixel{40, 220, 140, MAX_ALPHA};
    case GML_VERT:
      return Pixel{200, 80, 18, MAX_ALPHA};
    case GML_BLEU:
      return Pixel{250, 30, 80, MAX_ALPHA};
    case GML_BLACK:
      return Pixel{16, 16, 16, MAX_ALPHA};
    default:
      throw std::logic_error("Unknown line color.");
  }
}

inline auto LinesFx::LinesImpl::GetBlackLineColor() -> Pixel
{
  return GetColor(GML_BLACK);
}

inline auto LinesFx::LinesImpl::GetGreenLineColor() -> Pixel
{
  return GetColor(GML_VERT);
}

inline auto LinesFx::LinesImpl::GetRedLineColor() -> Pixel
{
  return GetColor(GML_RED);
}

auto LinesFx::LinesImpl::GetRandomLineColor() const -> Pixel
{
  if (m_goomRand.ProbabilityOfMInN(1, 50))
  {
    return GetColor(static_cast<int>(m_goomRand.GetNRand(6)));
  }
  return RandomColorMaps{m_goomRand}.GetRandomColor(GetRandomColorMap(), 0.0F, 1.0F);
}

void LinesFx::LinesImpl::DrawLines(const AudioSamples::SampleArray& soundData,
                                   const AudioSamples::MaxMinValues& soundMinMax)
{
  //constexpr size_t LAST_POINT_INDEX = AudioSamples::AUDIO_SAMPLE_LEN - 1;
  const size_t LAST_POINT_INDEX = AudioSamples::AUDIO_SAMPLE_LEN - 1;

  assert(m_srcLineType != LineType::CIRCLE || m_lineLerpFactor < 1.0F ||
         (floats_equal(m_srcePoints[0].x, m_srcePoints[LAST_POINT_INDEX].x) &&
          floats_equal(m_srcePoints[0].y, m_srcePoints[LAST_POINT_INDEX].y)));

  const LinePoint& pt0 = m_srcePoints[0];
  const LinePoint& ptN = m_srcePoints[LAST_POINT_INDEX];
  const Pixel lineColor = GetLightenedColor(m_srceColor, m_power);

  m_audioRange = soundMinMax.maxVal - soundMinMax.minVal;
  assert(m_audioRange >= 0.0F);
  m_minAudioValue = soundMinMax.minVal;

  if (m_audioRange < SMALL_FLOAT)
  {
    // No range - flatline audio
    const std::vector<Pixel> colors = {lineColor, lineColor};
    m_draw.Line(static_cast<int>(pt0.x), static_cast<int>(pt0.y), static_cast<int>(ptN.x),
                static_cast<int>(pt0.y), colors, 1);
    MoveSrceLineCloserToDest();
    return;
  }

  constexpr uint8_t THICKNESS = 1;
  const std::vector<PointAndColor> audioPoints = GetAudioPoints(lineColor, soundData);

  V2dInt point1 = audioPoints[0].point;
  V2dInt point2{};

  for (size_t i = 1; i < audioPoints.size(); ++i)
  {
    const PointAndColor& nextPointData = audioPoints[i];

    point2 = nextPointData.point;
    const Pixel modColor = nextPointData.color;
    const std::vector<Pixel> colors = {lineColor, modColor};

    m_draw.Line(point1.x, point1.y, point2.x, point2.y, colors, THICKNESS);
    m_dotDrawer.DrawDots(point2, colors, 1.0F);

    point1 = point2;
  }

  MoveSrceLineCloserToDest();

  m_dotDrawer.ChangeDotSizes();
}

auto LinesFx::LinesImpl::GetAudioPoints(const Pixel& lineColor,
                                        const std::vector<float>& audioData) const
    -> std::vector<PointAndColor>
{
  const Pixel randColor = GetRandomLineColor();

  //constexpr float T_STEP = 1.0F / static_cast<float>(AudioSamples::AUDIO_SAMPLE_LEN - 1);
  const float T_STEP = 1.0F / static_cast<float>(AudioSamples::AUDIO_SAMPLE_LEN - 1);
  constexpr float HALFWAY_T = 0.5F;
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

  if ((m_srcLineType == LineType::CIRCLE) && (m_lineLerpFactor >= 1.0F))
  {
    // It's a complete circle - lerp the last few points to nicely join back to start.
    SmoothCircleJoin(audioPoints);
  }

  return audioPoints;
}

void LinesFx::LinesImpl::SmoothCircleJoin(std::vector<PointAndColor>& audioPoints)
{
  constexpr size_t NUM_POINTS_TO_LERP = 50;
  assert(NUM_POINTS_TO_LERP < audioPoints.size());

  constexpr float T_STEP = 1.0F / static_cast<float>(NUM_POINTS_TO_LERP);

  const size_t lastPointIndex = audioPoints.size() - 1;
  const V2dInt endDiff = audioPoints[0].point - audioPoints[lastPointIndex].point;
  if ((0 == endDiff.x) && (0 == endDiff.y))
  {
    return;
  }

  V2dInt diff = endDiff;
  float t = 1.0F - T_STEP;
  for (size_t i = lastPointIndex; i > (audioPoints.size() - NUM_POINTS_TO_LERP); --i)
  {
    audioPoints[i].point += diff;

    diff = {static_cast<int32_t>(std::round(t * static_cast<float>(endDiff.x))),
            static_cast<int32_t>(std::round(t * static_cast<float>(endDiff.y)))};

    t -= T_STEP;
  }

  assert(audioPoints[0].point.x == audioPoints[audioPoints.size() - 1].point.x);
  assert(audioPoints[0].point.y == audioPoints[audioPoints.size() - 1].point.y);
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
  const V2dInt nextPointData{static_cast<int>(pt.x + (m_amplitude * cosAngle * normalizedDataVal)),
                             static_cast<int>(pt.y + (m_amplitude * sinAngle * normalizedDataVal))};

  const float brightness = m_currentBrightness * tData;
  const Pixel modColor =
      GetGammaCorrection(brightness, IColorMap::GetColorMix(mainColor, randColor, tData));

  return {nextPointData, modColor};
}

inline auto LinesFx::LinesImpl::GetGammaCorrection(const float brightness, const Pixel& color) const
    -> Pixel
{
  // if constexpr (GAMMA == 1.0F)
  if (1.0F == GAMMA)
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
  return m_currentColorMap.get().GetColor(t);
}

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
