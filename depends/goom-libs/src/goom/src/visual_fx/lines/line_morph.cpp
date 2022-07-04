/*
 *  Goom
 *  Copyright (c) 2000-2003 iOS-software. All rights reserved.
 *
 *  - converted to C++17 2021-02-01 (glk)
 */

#include "line_morph.h"

//#undef NO_LOGGING

#include "color/color_maps.h"
#include "color/color_utils.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "goom/logging.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "sound_info.h"
#include "utils/enumutils.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::LINES
{

using COLOR::GetLightenedColor;
using COLOR::GetSimpleColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using COLOR::SimpleColors;
using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using FX_UTILS::DotSizes;
using FX_UTILS::GetCircularLinePoints;
using FX_UTILS::GetHorizontalLinePoints;
using FX_UTILS::GetVerticalLinePoints;
using FX_UTILS::LinePoint;
using FX_UTILS::SmoothTheCircleJoinAtEnds;
using UTILS::Logging;
using UTILS::NUM;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::FloatsEqual;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::SMALL_FLOAT;

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

LineMorph::LineMorph(IGoomDraw& draw,
                     const PluginInfo& goomInfo,
                     const IGoomRand& goomRand,
                     const SmallImageBitmaps& smallBitmaps,
                     const LineParams& srceLineParams,
                     const LineParams& destLineParams) noexcept
  : m_draw{draw},
    m_goomInfo{goomInfo},
    m_goomRand{goomRand},
    m_srcePoints(AudioSamples::AUDIO_SAMPLE_LEN),
    m_srcePointsCopy(AudioSamples::AUDIO_SAMPLE_LEN),
    m_srceLineParams{srceLineParams},
    m_destPoints(AudioSamples::AUDIO_SAMPLE_LEN),
    m_destLineParams{destLineParams},
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

auto LineMorph::GetRandomColorMap() const noexcept -> const IColorMap&
{
  Expects(m_colorMaps != nullptr);
  return m_colorMaps->GetRandomColorMap(m_colorMaps->GetRandomGroup());
}

auto LineMorph::Start() noexcept -> void
{
  UpdateColorInfo();

  m_srcePoints = GetFreshLine(m_srceLineParams.lineType, m_srceLineParams.param);
  m_srcePointsCopy = m_srcePoints;

  ResetDestLine(m_destLineParams);
}

auto LineMorph::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return {m_colorMaps->GetColorMapsName()};
}

auto LineMorph::SetWeightedColorMaps(
    const std::shared_ptr<const RandomColorMaps> weightedMaps) noexcept -> void
{
  m_colorMaps = weightedMaps;
  m_colorMapsManager.UpdateColorMapInfo(m_currentColorMapID,
                                        {m_colorMaps, RandomColorMaps::ALL_COLOR_MAP_TYPES});
}

inline auto LineMorph::UpdateColorInfo() noexcept -> void
{
  m_colorMapsManager.ChangeAllColorMapsNow();

  static constexpr float PROB_USE_LINE_COLOR = 0.5F;
  m_useLineColor = m_goomRand.ProbabilityOf(PROB_USE_LINE_COLOR);
}

inline auto LineMorph::GetFreshLine(const LineType lineType, const float lineParam) const noexcept
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
      FailFast();
      return {};
  }
}

auto LineMorph::MoveSrceLineCloserToDest() noexcept -> void
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
    m_srceLineParams.lineType = m_destLineParams.lineType;
    static constexpr float MIN_BRIGHTNESS = 2.5F;
    static constexpr float MAX_BRIGHTNESS = 4.0F;
    m_currentBrightness = m_goomRand.GetRandInRange(MIN_BRIGHTNESS, MAX_BRIGHTNESS);
  }

  Ensures((m_srceLineParams.lineType != LineType::CIRCLE) or (m_lineLerpParam < 1.0F) or
          (FloatsEqual(m_srcePoints[0].point.x,
                       m_srcePoints[AudioSamples::AUDIO_SAMPLE_LEN - 1].point.x) and
           FloatsEqual(m_srcePoints[0].point.y,
                       m_srcePoints[AudioSamples::AUDIO_SAMPLE_LEN - 1].point.y)));

  static constexpr float COLOR_MIX_AMOUNT = 1.0F / 64.0F;
  m_srceLineParams.color =
      IColorMap::GetColorMix(m_srceLineParams.color, m_destLineParams.color, COLOR_MIX_AMOUNT);

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
  m_srceLineParams.amplitude =
      STD20::lerp(m_srceLineParams.amplitude, m_destLineParams.amplitude, AMP_MIX_AMOUNT);
}

auto LineMorph::ResetDestLine(const LineParams& newParams) noexcept -> void
{
  UpdateColorInfo();

  m_destPoints = GetFreshLine(newParams.lineType, m_destLineParams.param);
  m_destLineParams = newParams;

  m_lineLerpParam = 0.0;

  static constexpr float MIN_BRIGHTNESS = 1.5F;
  static constexpr float MAX_BRIGHTNESS = 3.0F;
  m_currentBrightness = m_goomRand.GetRandInRange(MIN_BRIGHTNESS, MAX_BRIGHTNESS);

  m_dotDrawer.ChangeDotSizes();

  m_maxNormalizedPeak = m_goomRand.GetRandInRange(MIN_MAX_NORMALIZED_PEAK, MAX_MAX_NORMALIZED_PEAK);

  m_srcePointsCopy = m_srcePoints;
}

auto LineMorph::GetRandomLineColor() const noexcept -> Pixel
{
  if (constexpr float PROB_LINE_COLOR = 0.02F; m_goomRand.ProbabilityOf(PROB_LINE_COLOR))
  {
    return GetSimpleColor(static_cast<SimpleColors>(m_goomRand.GetNRand(NUM<SimpleColors>)));
  }
  return RandomColorMaps::GetRandomColor(m_goomRand, GetRandomColorMap(), 0.0F, 1.0F);
}

inline auto LineMorph::GetFinalLineColor(const Pixel& color) const noexcept -> Pixel
{
  return GetLightenedColor(color, m_lineColorPower);
}

auto LineMorph::DrawLines(const AudioSamples::SampleArray& soundData,
                          const AudioSamples::MinMaxValues& soundMinMax) noexcept -> void
{
  static constexpr size_t LAST_POINT_INDEX = AudioSamples::AUDIO_SAMPLE_LEN - 1;
  UNUSED_FOR_NDEBUG(LAST_POINT_INDEX);

  Expects((m_srceLineParams.lineType != LineType::CIRCLE) or (m_lineLerpParam < 1.0F) or
          (FloatsEqual(m_srcePoints[0].point.x, m_srcePoints[LAST_POINT_INDEX].point.x) and
           FloatsEqual(m_srcePoints[0].point.y, m_srcePoints[LAST_POINT_INDEX].point.y)));

  const Pixel lineColor = GetFinalLineColor(m_srceLineParams.color);

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
    const MultiplePixels colors = {lineColor, nextPointData.color};

    m_draw.Line(point1, point2, colors, LINE_THICKNESS);

    static constexpr float DOT_BRIGHTNESS = 1.5F;
    m_dotDrawer.DrawDot(point2, colors, DOT_BRIGHTNESS);

    point1 = point2;
  }

  MoveSrceLineCloserToDest();

  m_dotDrawer.ChangeDotSizes();
}

auto LineMorph::DrawFlatLine(const Pixel& lineColor) noexcept -> void
{
  const LinePoint& pt0 = m_srcePoints[0];
  const LinePoint& ptN = m_srcePoints[AudioSamples::AUDIO_SAMPLE_LEN - 1];
  const MultiplePixels colors = {lineColor, lineColor};

  m_draw.Line(pt0.point.ToInt(), ptN.point.ToInt(), colors, 1);
}

auto LineMorph::GetAudioPoints(const Pixel& lineColor,
                               const AudioSamples::SampleArray& audioData) const noexcept
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

  if ((m_srceLineParams.lineType == LineType::CIRCLE) and (m_lineLerpParam >= 1.0F))
  {
    // This is a complete circle -- lerp the last few points to nicely join back to start.
    static constexpr size_t NUM_POINTS_TO_LERP = 50;
    SmoothTheCircleJoinAtEnds(audioPoints, NUM_POINTS_TO_LERP);
  }

  return audioPoints;
}

auto LineMorph::GetNextPointData(const LinePoint& linePoint,
                                 const Pixel& mainColor,
                                 const Pixel& randColor,
                                 const float dataVal) const noexcept -> PointAndColor
{
  Expects(m_goomInfo.GetSoundEvents().GetSoundInfo().GetAllTimesMinVolume() <=
          (dataVal + SMALL_FLOAT));
  Expects(m_minAudioValue <= (dataVal + SMALL_FLOAT));
  Expects(dataVal <= ((m_minAudioValue + m_audioRange) + SMALL_FLOAT));

  const float tData = (dataVal - m_minAudioValue) / m_audioRange;
  assert((0.0F <= tData) && (tData <= 1.0F));

  const float cosAngle = std::cos(linePoint.angle);
  const float sinAngle = std::sin(linePoint.angle);
  const float normalizedDataVal = m_maxNormalizedPeak * tData;
  assert(normalizedDataVal >= 0.0F);
  // TODO - Is 'm_srceLineParams.amplitude' the right abstraction level?
  const Point2dInt nextPointData{
      static_cast<int>(linePoint.point.x +
                       (m_srceLineParams.amplitude * cosAngle * normalizedDataVal)),
      static_cast<int>(linePoint.point.y +
                       (m_srceLineParams.amplitude * sinAngle * normalizedDataVal))};

  const float brightness = m_currentBrightness * tData;
  const Pixel modColor =
      m_colorAdjust.GetAdjustment(brightness, IColorMap::GetColorMix(mainColor, randColor, tData));

  return {nextPointData, modColor};
}

inline auto LineMorph::GetMainColor(const Pixel& lineColor, const float t) const noexcept -> Pixel
{
  if (m_useLineColor)
  {
    return lineColor;
  }
  return m_colorMapsManager.GetColorMap(m_currentColorMapID).GetColor(t);
}

} // namespace GOOM::VISUAL_FX::LINES
