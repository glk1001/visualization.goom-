module;

/*
 * ifs.c --- modified iterated functions system for goom.
 */

/*-
 * Copyright (c) 1997 by Massimino Pascal <Pascal.Massimon@ens.fr>
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 * If this mode is weird, and you have an old MetroX server, it is buggy.
 * There is a free SuSE-enhanced MetroX X server that is fine.
 *
 * When shown ifs, Diana Rose (4 years old) said, "It looks like dancing."
 *
 * Revision History:
 * 01-Feb-2021: converted to C++17, lots of color changes (glk)
 * 13-Dec-2003: Added some goom specific stuffs (to make ifs a VisualFX).
 * 11-Apr-2002: jeko@ios-software.com: Make ifs.c system-independent. (ifs.h added)
 * 01-Nov-2000: Allocation checks
 * 10-May-1997: jwz@jwz.org: turned into a standalone program.
 *              Made it render into an off-screen bitmap and then copy
 *              that onto the screen, to reduce flicker.
 */

//#undef NO_LOGGING

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

module Goom.VisualFx.IfsDancersFx;

import Goom.Color.RandomColorMaps;
import Goom.Draw.ShapeDrawers.BitmapDrawer;
import Goom.Draw.ShaperDrawers.PixelDrawer;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRandBase;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.FxUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;
import :Colorizer;
import :Fractal;
import :LowDensityBlurrer;

namespace GOOM::VISUAL_FX
{

using COLOR::WeightedRandomColorMaps;
using DRAW::SHAPE_DRAWERS::BitmapDrawer;
using DRAW::SHAPE_DRAWERS::PixelDrawer;
using FX_UTILS::RandomPixelBlender;
using IFS::BlurrerColorMode;
using IFS::Colorizer;
using IFS::Fractal;
using IFS::IfsPoint;
using IFS::LowDensityBlurrer;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::TValue;
using UTILS::MATH::Weights;

class IfsDancersFx::IfsDancersFxImpl
{
public:
  IfsDancersFxImpl(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept;

  auto Start() noexcept -> void;

  auto Refresh() noexcept -> void;
  auto Suspend() noexcept -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;

  auto ApplyToImageBuffers() noexcept -> void;
  auto UpdateLowDensityThreshold() noexcept -> void;

private:
  static constexpr int32_t MIN_CYCLE_LENGTH = 1000;
  static constexpr int32_t MAX_CYCLE_LENGTH = 2000;
  int32_t m_cycleLength                     = MIN_CYCLE_LENGTH;

  FxHelper* m_fxHelper;
  BitmapDrawer m_bitmapDrawer;
  PixelDrawer m_pixelDrawer;
  PixelChannelType m_defaultAlpha = DEFAULT_VISUAL_FX_ALPHA;

  Colorizer m_colorizer;

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  std::unique_ptr<Fractal> m_fractal;

  auto InitFractal() noexcept -> void;

  int32_t m_cycle    = 0;
  int32_t m_ifsIncr  = 1; // dessiner l'ifs (0 = non: > = increment)
  int32_t m_decayIfs = 0; // disparition de l'ifs
  int32_t m_recayIfs = 0; // dedisparition de l'ifs
  auto UpdateIncr() noexcept -> void;
  auto UpdateCycle() noexcept -> void;
  auto UpdateDecayAndRecay() noexcept -> void;
  auto Renew() noexcept -> void;

  // TODO(glk) Move to simi
  static constexpr auto T_MIX_STARTING_VALUE = 0.01F;
  TValue m_tMix{
      {T_MIX_STARTING_VALUE, TValue::StepType::CONTINUOUS_REVERSIBLE}
  };
  static constexpr auto POINT_BRIGHTNESS  = 8.0F;
  static constexpr auto BITMAP_BRIGHTNESS = 6.0F;
  auto InitColorMaps() noexcept -> void;
  auto ChangeColorMaps() noexcept -> void;
  auto ChangeSpeed() noexcept -> void;
  auto DrawNextIfsPoints() noexcept -> void;
  auto DrawPoint(float t, const IfsPoint& ifsPoint, float tMix) noexcept -> void;

  static constexpr auto PROB_DRAW_LOW_DENSITY_POINTS = 0.1F;
  bool m_drawLowDensityPoints                         = false;
  static constexpr uint32_t MAX_DENSITY_COUNT         = 20;
  static constexpr uint32_t MIN_DENSITY_COUNT         = 5;
  uint32_t m_lowDensityCount                          = MIN_DENSITY_COUNT;
  LowDensityBlurrer m_blurrer;
  static constexpr auto BLUR_WIDTH                         = 3U;
  static constexpr auto DEFAULT_LOW_DENSITY_BLUR_THRESHOLD = 0.99F;
  float m_lowDensityBlurThreshold                          = DEFAULT_LOW_DENSITY_BLUR_THRESHOLD;
  auto DrawLowDensityPoints(size_t numPointsAlreadyDrawn,
                            const std::vector<IfsPoint>& points) noexcept -> void;
  [[nodiscard]] auto BlurTheLowDensityPoints(
      size_t numPointsAlreadyDrawn,
      const std::vector<IfsPoint>& lowDensityPoints) const noexcept -> bool;
  auto DrawLowDensityPointsWithoutBlur(const std::vector<IfsPoint>& lowDensityPoints,
                                       uint32_t maxLowDensityCount) noexcept -> void;
  auto DrawLowDensityPointsWithBlur(std::vector<IfsPoint>& lowDensityPoints,
                                    uint32_t maxLowDensityCount) noexcept -> void;
  auto UpdateLowDensityBlurThreshold() noexcept -> void;
  [[nodiscard]] auto GetNewBlurWidth() const noexcept -> uint32_t;

  [[nodiscard]] auto MegaChangeColorMapEvent() const noexcept -> bool;
  [[nodiscard]] auto IfsRenewEvent() const noexcept -> bool;
  Weights<BlurrerColorMode> m_blurrerColorModeWeights;
};

IfsDancersFx::IfsDancersFx(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_pimpl{spimpl::make_unique_impl<IfsDancersFxImpl>(fxHelper, smallBitmaps)}
{
}

auto IfsDancersFx::GetFxName() const noexcept -> std::string
{
  return "IFS FX";
}

auto IfsDancersFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto IfsDancersFx::Finish() noexcept -> void
{
  // nothing to do
}

auto IfsDancersFx::Resume() noexcept -> void
{
  // nothing to do
}

auto IfsDancersFx::Suspend() noexcept -> void
{
  // nothing to do
}

auto IfsDancersFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto IfsDancersFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto IfsDancersFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto IfsDancersFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

auto IfsDancersFx::Refresh() noexcept -> void
{
  m_pimpl->Refresh();
}

static constexpr auto BLURRER_COLOR_MODE_SMOOTH_WITH_NEIGHBOURS_WGT = 999.0F;
static constexpr auto BLURRER_COLOR_MODE_SMOOTH_NO_NEIGHBOURS_WGT   = 001.0F;
static constexpr auto BLURRER_COLOR_MODE_SIMI_WITH_NEIGHBOURS_WGT   = 001.0F;
static constexpr auto BLURRER_COLOR_MODE_SIMI_NO_NEIGHBOURS_WGT     = 005.0F;
static constexpr auto BLURRER_COLOR_MODE_SINGLE_WITH_NEIGHBOURS_WGT = 001.0F;
static constexpr auto BLURRER_COLOR_MODE_SINGLE_NO_NEIGHBOURS_WGT   = 005.0F;

IfsDancersFx::IfsDancersFxImpl::IfsDancersFxImpl(FxHelper& fxHelper,
                                                 const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxHelper{&fxHelper},
    m_bitmapDrawer{fxHelper.GetDraw()},
    m_pixelDrawer{fxHelper.GetDraw()},
    m_colorizer{fxHelper.GetGoomRand(), m_defaultAlpha},
    m_pixelBlender{fxHelper.GetGoomRand()},
    m_fractal{std::make_unique<Fractal>(fxHelper.GetDimensions(),
                                        fxHelper.GetGoomRand(),
                                        smallBitmaps)},
    m_blurrer{fxHelper.GetDraw(), fxHelper.GetGoomRand(), BLUR_WIDTH, m_colorizer, smallBitmaps},
    m_blurrerColorModeWeights{
      fxHelper.GetGoomRand(),
      {
        {BlurrerColorMode::SMOOTH_WITH_NEIGHBOURS, BLURRER_COLOR_MODE_SMOOTH_WITH_NEIGHBOURS_WGT},
        {BlurrerColorMode::SMOOTH_NO_NEIGHBOURS,   BLURRER_COLOR_MODE_SMOOTH_NO_NEIGHBOURS_WGT},
        {BlurrerColorMode::SIMI_WITH_NEIGHBOURS,   BLURRER_COLOR_MODE_SIMI_WITH_NEIGHBOURS_WGT},
        {BlurrerColorMode::SIMI_NO_NEIGHBOURS,     BLURRER_COLOR_MODE_SIMI_NO_NEIGHBOURS_WGT},
        {BlurrerColorMode::SINGLE_WITH_NEIGHBOURS, BLURRER_COLOR_MODE_SINGLE_WITH_NEIGHBOURS_WGT},
        {BlurrerColorMode::SINGLE_NO_NEIGHBOURS,   BLURRER_COLOR_MODE_SINGLE_NO_NEIGHBOURS_WGT},
    }}
{
}

inline auto IfsDancersFx::IfsDancersFxImpl::MegaChangeColorMapEvent() const noexcept -> bool
{
  static constexpr auto PROB_MEGA_CHANGE_COLOR_MAP_EVENT = 0.5F;
  return m_fxHelper->GetGoomRand().ProbabilityOf(PROB_MEGA_CHANGE_COLOR_MAP_EVENT);
}

inline auto IfsDancersFx::IfsDancersFxImpl::IfsRenewEvent() const noexcept -> bool
{
  static constexpr auto PROB_IFS_RENEW_EVENT = 2.0F / 3.0F;
  return m_fxHelper->GetGoomRand().ProbabilityOf(PROB_IFS_RENEW_EVENT);
}

inline auto IfsDancersFx::IfsDancersFxImpl::InitFractal() noexcept -> void
{
  m_fractal->Init();
  UpdateLowDensityThreshold();
}

inline auto IfsDancersFx::IfsDancersFxImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto IfsDancersFx::IfsDancersFxImpl::GetCurrentColorMapsNames() const noexcept
    -> std::vector<std::string>
{
  return {m_colorizer.GetWeightedColorMaps().GetColorMapsName()};
}

inline auto IfsDancersFx::IfsDancersFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  const auto newWeightedMainColorMaps =
      WeightedRandomColorMaps{weightedColorMaps.mainColorMaps, m_defaultAlpha};

  m_fractal->SetWeightedColorMaps(newWeightedMainColorMaps);
  m_colorizer.SetWeightedColorMaps(newWeightedMainColorMaps);
}

inline auto IfsDancersFx::IfsDancersFxImpl::Start() noexcept -> void
{
  InitFractal();
  InitColorMaps();
}

inline auto IfsDancersFx::IfsDancersFxImpl::Suspend() noexcept -> void
{
  InitFractal();

  UpdateIncr();

  if (IfsRenewEvent())
  {
    Renew();
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::Refresh() noexcept -> void
{
  Renew();
}

inline auto IfsDancersFx::IfsDancersFxImpl::Renew() noexcept -> void
{
  ChangeColorMaps();
  m_colorizer.ChangeColorMode();

  ChangeSpeed();
}

inline auto IfsDancersFx::IfsDancersFxImpl::ChangeSpeed() noexcept -> void
{
  static constexpr auto MIN_SPEED_AMP    = 1.1F;
  static constexpr auto MAX_SPEED_AMP    = 5.1F;
  static constexpr auto MAX_SPEED_WEIGHT = 10.0F;
  const auto speedAmp                    = std::min(
      m_fxHelper->GetGoomRand().GetRandInRange(MIN_SPEED_AMP, MAX_SPEED_WEIGHT), MAX_SPEED_AMP);
  const auto accelFactor =
      1.0F / (1.1F - m_fxHelper->GetSoundEvents().GetSoundInfo().GetAcceleration());

  m_fractal->SetSpeed(std::max(1U, static_cast<uint32_t>(speedAmp * accelFactor)));
}

auto IfsDancersFx::IfsDancersFxImpl::InitColorMaps() noexcept -> void
{
  m_colorizer.InitColorMaps();
}

auto IfsDancersFx::IfsDancersFxImpl::ChangeColorMaps() noexcept -> void
{
  m_colorizer.ChangeColorMaps();

  m_drawLowDensityPoints = m_fxHelper->GetGoomRand().ProbabilityOf(PROB_DRAW_LOW_DENSITY_POINTS);

  if (m_drawLowDensityPoints)
  {
    m_blurrer.SetColorMode(m_blurrerColorModeWeights.GetRandomWeighted());
    static constexpr auto SINGLE_COLOR_T = 0.5F;
    m_blurrer.SetSingleColor(
        m_colorizer.GetColorMaps().GetRandomColorMap().GetColor(SINGLE_COLOR_T));
  }
}

auto IfsDancersFx::IfsDancersFxImpl::ApplyToImageBuffers() noexcept -> void
{
  UpdatePixelBlender();

  UpdateDecayAndRecay();
  if (m_ifsIncr <= 0)
  {
    return;
  }

  UpdateCycle();

  DrawNextIfsPoints();
}

inline auto IfsDancersFx::IfsDancersFxImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->GetDraw().SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

auto IfsDancersFx::IfsDancersFxImpl::UpdateDecayAndRecay() noexcept -> void
{
  static constexpr auto BY_TWO = 2;

  --m_decayIfs;
  if (m_decayIfs > 0)
  {
    m_ifsIncr += BY_TWO;
  }
  if (0 == m_decayIfs)
  {
    m_ifsIncr = 0;
  }

  if (m_recayIfs != 0)
  {
    m_ifsIncr -= BY_TWO;
    --m_recayIfs;
    if ((0 == m_recayIfs) && (m_ifsIncr <= 0))
    {
      m_ifsIncr = 1;
    }
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::UpdateIncr() noexcept -> void
{
  if (m_ifsIncr <= 0)
  {
    static constexpr auto NEW_RECAY_IFS = 5;
    m_recayIfs                          = NEW_RECAY_IFS;
    static constexpr auto NEW_IFS_INCR  = 11;
    m_ifsIncr                           = NEW_IFS_INCR;
    Renew();
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::UpdateCycle() noexcept -> void
{
  // TODO(glk): trouver meilleur soluce pour increment (mettre le code de gestion de l'ifs
  //            dans ce fichier)
  //            find the best solution for increment (put the management code of the ifs
  //            in this file)
  m_tMix.Increment();

  ++m_cycle;
  if (m_cycle < m_cycleLength)
  {
    return;
  }

  m_cycle       = 0;
  m_cycleLength = m_fxHelper->GetGoomRand().GetRandInRange(MIN_CYCLE_LENGTH, MAX_CYCLE_LENGTH + 1);

  UpdateLowDensityBlurThreshold();

  m_fractal->Reset();

  Renew();
}

inline auto IfsDancersFx::IfsDancersFxImpl::UpdateLowDensityBlurThreshold() noexcept -> void
{
  static constexpr auto PROB_HIGH_BLUR_THRESHOLD = 0.75F;
  static constexpr auto HIGH_BLUR_THRESHOLD      = 0.99F;
  static constexpr auto LOW_BLUR_THRESHOLD       = 0.40F;
  m_lowDensityBlurThreshold = m_fxHelper->GetGoomRand().ProbabilityOf(PROB_HIGH_BLUR_THRESHOLD)
                                  ? HIGH_BLUR_THRESHOLD
                                  : LOW_BLUR_THRESHOLD;
}

auto IfsDancersFx::IfsDancersFxImpl::DrawNextIfsPoints() noexcept -> void
{
  m_colorizer.SetMaxHitCount(m_fractal->GetMaxHitCount());

  const auto& points   = m_fractal->GetNextIfsPoints();
  const auto numPoints = points.size();
  const auto tStep = (1 == numPoints) ? 0.0F : ((1.0F - 0.0F) / static_cast<float>(numPoints - 1));
  auto t           = -tStep;

  auto doneColorChange = (m_colorizer.GetColorMode() != ColorMode::MEGA_MAP_COLOR_CHANGE) &&
                         (m_colorizer.GetColorMode() != ColorMode::MEGA_MIX_COLOR_CHANGE);
  auto numPointsDrawn = 0U;

  for (auto i = 0U; i < numPoints; i += static_cast<uint32_t>(m_ifsIncr))
  {
    const auto& point = points[i];

    t += tStep;

    if ((not doneColorChange) and MegaChangeColorMapEvent())
    {
      ChangeColorMaps();
      doneColorChange = true;
    }

    DrawPoint(t, point, m_tMix());

    ++numPointsDrawn;
  }

  DrawLowDensityPoints(numPointsDrawn, points);
}

auto IfsDancersFx::IfsDancersFxImpl::DrawPoint(const float t,
                                               const IfsPoint& ifsPoint,
                                               const float tMix) noexcept -> void
{
  const auto point =
      Point2dInt{static_cast<int32_t>(ifsPoint.GetX()), static_cast<int32_t>(ifsPoint.GetY())};

  const auto tX = static_cast<float>(point.x) / m_fxHelper->GetDimensions().GetFltWidth();
  const auto tY = static_cast<float>(point.y) / m_fxHelper->GetDimensions().GetFltHeight();

  if (const auto baseColor = ifsPoint.GetSimi()->GetColorMap().GetColor(t);
      nullptr == ifsPoint.GetSimi()->GetCurrentPointBitmap())
  {
    const auto mixedColor =
        m_colorizer.GetMixedColor(baseColor, ifsPoint.GetCount(), {POINT_BRIGHTNESS, tMix, tX, tY});
    m_pixelDrawer.DrawPixels(point, {mixedColor, mixedColor});
  }
  else
  {
    const auto mixedColor = m_colorizer.GetMixedColor(
        baseColor, ifsPoint.GetCount(), {BITMAP_BRIGHTNESS, tMix, tX, tY});
    const auto getColor = [&mixedColor]([[maybe_unused]] const Point2dInt& bitmapPoint,
                                        [[maybe_unused]] const Pixel& bgnd) { return mixedColor; };
    const auto& bitmap{*ifsPoint.GetSimi()->GetCurrentPointBitmap()};
    m_bitmapDrawer.Bitmap(point, bitmap, {getColor, getColor});
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::DrawLowDensityPoints(
    const size_t numPointsAlreadyDrawn, const std::vector<IfsPoint>& points) noexcept -> void
{
  if (not m_drawLowDensityPoints)
  {
    return;
  }

  const auto numPoints    = points.size();
  auto maxLowDensityCount = 0U;
  auto lowDensityPoints   = std::vector<IfsPoint>{};

  for (auto i = 0U; i < numPoints; i += static_cast<uint32_t>(m_ifsIncr))
  {
    const auto& point = points[i];

    if (point.GetCount() > m_lowDensityCount)
    {
      continue;
    }

    lowDensityPoints.emplace_back(point);
    maxLowDensityCount = std::max(maxLowDensityCount, point.GetCount());
  }

  if (BlurTheLowDensityPoints(numPointsAlreadyDrawn, lowDensityPoints))
  {
    DrawLowDensityPointsWithBlur(lowDensityPoints, maxLowDensityCount);
  }
  else
  {
    DrawLowDensityPointsWithoutBlur(lowDensityPoints, maxLowDensityCount);
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::BlurTheLowDensityPoints(
    const size_t numPointsAlreadyDrawn,
    const std::vector<IfsPoint>& lowDensityPoints) const noexcept -> bool
{
  if (0 == numPointsAlreadyDrawn)
  {
    return false;
  }
  return (static_cast<float>(lowDensityPoints.size()) / static_cast<float>(numPointsAlreadyDrawn)) >
         m_lowDensityBlurThreshold;
}

inline auto IfsDancersFx::IfsDancersFxImpl::DrawLowDensityPointsWithoutBlur(
    const std::vector<IfsPoint>& lowDensityPoints,
    const uint32_t maxLowDensityCount) noexcept -> void
{
  const auto logMaxLowDensityCount = std::log(static_cast<float>(maxLowDensityCount));

  auto t           = 0.0F;
  const auto tStep = 1.0F / static_cast<float>(lowDensityPoints.size());
  for (const auto& point : lowDensityPoints)
  {
    const auto logAlpha =
        point.GetCount() <= 1
            ? 1.0F
            : (std::log(static_cast<float>(point.GetCount())) / logMaxLowDensityCount);

    DrawPoint(t, point, logAlpha);

    t += tStep;
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::DrawLowDensityPointsWithBlur(
    std::vector<IfsPoint>& lowDensityPoints, const uint32_t maxLowDensityCount) noexcept -> void
{
  if (static constexpr auto PROB_FIXED_MIX_FACTOR = 0.8F;
      m_fxHelper->GetGoomRand().ProbabilityOf(PROB_FIXED_MIX_FACTOR))
  {
    static constexpr auto FIXED_MIX_FACTOR = 0.98F;
    m_blurrer.SetNeighbourMixFactor(FIXED_MIX_FACTOR);
  }
  else
  {
    static constexpr auto MIN_MIX_FACTOR = 0.9F;
    static constexpr auto MAX_MIX_FACTOR = 1.0F;
    m_blurrer.SetNeighbourMixFactor(
        m_fxHelper->GetGoomRand().GetRandInRange(MIN_MIX_FACTOR, MAX_MIX_FACTOR));
  }

  m_blurrer.DoBlur(lowDensityPoints, maxLowDensityCount);
}

inline auto IfsDancersFx::IfsDancersFxImpl::UpdateLowDensityThreshold() noexcept -> void
{
  m_lowDensityCount =
      m_fxHelper->GetGoomRand().GetRandInRange(MIN_DENSITY_COUNT, MAX_DENSITY_COUNT);

  m_blurrer.SetWidth(GetNewBlurWidth());
}

inline auto IfsDancersFx::IfsDancersFxImpl::GetNewBlurWidth() const noexcept -> uint32_t
{
  static constexpr auto NUM_WIDTHS         = 3U;
  static constexpr auto WIDTH_RANGE        = (MAX_DENSITY_COUNT - MIN_DENSITY_COUNT) / NUM_WIDTHS;
  static constexpr auto DOUBLE_WIDTH_RANGE = 2 * WIDTH_RANGE;

  static constexpr auto LARGE_BLUR_WIDTH  = 7U;
  static constexpr auto MEDIUM_BLUR_WIDTH = 5U;
  static constexpr auto SMALL_BLUR_WIDTH  = 3U;

  auto blurWidth = SMALL_BLUR_WIDTH;

  if (m_lowDensityCount <= (MIN_DENSITY_COUNT + WIDTH_RANGE))
  {
    blurWidth = LARGE_BLUR_WIDTH;
  }
  else if (m_lowDensityCount <= (MIN_DENSITY_COUNT + DOUBLE_WIDTH_RANGE))
  {
    blurWidth = MEDIUM_BLUR_WIDTH;
  }

  return blurWidth;
}

} // namespace GOOM::VISUAL_FX
