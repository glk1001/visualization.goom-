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

#include "ifs_dancers_fx.h"

//#undef NO_LOGGING

#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "ifs/colorizer.h"
#include "ifs/fractal.h"
#include "ifs/low_density_blurrer.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX
{

using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using IFS::BlurrerColorMode;
using IFS::Colorizer;
using IFS::Fractal;
using IFS::IfsPoint;
using IFS::LowDensityBlurrer;
using UTILS::TValue;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Weights;

class IfsDancersFx::IfsDancersFxImpl
{
public:
  IfsDancersFxImpl(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept;

  auto ApplyNoDraw() -> void;
  auto UpdateIfs() -> void;
  auto UpdateLowDensityThreshold() -> void;
  auto Refresh() -> void;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;
  auto PostStateUpdate(bool wasActiveInPreviousState) -> void;

  auto Start() -> void;
  auto Finish() -> void;

private:
  static constexpr int32_t MIN_CYCLE_LENGTH = 1000;
  static constexpr int32_t MAX_CYCLE_LENGTH = 2000;
  int32_t m_cycleLength = MIN_CYCLE_LENGTH;

  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;

  Colorizer m_colorizer;

  std::unique_ptr<Fractal> m_fractal{};

  auto InitFractal() -> void;

  int32_t m_cycle = 0;
  int32_t m_ifsIncr = 1; // dessiner l'ifs (0 = non: > = increment)
  int32_t m_decayIfs = 0; // disparition de l'ifs
  int32_t m_recayIfs = 0; // dedisparition de l'ifs
  auto UpdateIncr() -> void;
  [[nodiscard]] auto GetIfsIncr() const -> int;
  auto UpdateCycle() -> void;
  auto UpdateDecay() -> void;
  auto UpdateDecayAndRecay() -> void;
  auto Renew() -> void;

  // TODO Move to simi
  static constexpr float T_MIX_STARTING_VALUE = 0.01F;
  TValue m_tMix{TValue::StepType::CONTINUOUS_REVERSIBLE, T_MIX_STARTING_VALUE};
  static constexpr float POINT_BRIGHTNESS = 3.0F;
  static constexpr float BITMAP_BRIGHTNESS = 2.0F;
  auto ChangeColorMaps() -> void;
  auto ChangeSpeed() -> void;
  auto DrawNextIfsPoints() -> void;
  auto DrawPoint(const IfsPoint& ifsPoint, float t, float tMix) const -> void;

  static constexpr uint32_t MAX_DENSITY_COUNT = 20;
  static constexpr uint32_t MIN_DENSITY_COUNT = 5;
  uint32_t m_lowDensityCount = MIN_DENSITY_COUNT;
  LowDensityBlurrer m_blurrer;
  static constexpr uint32_t BLUR_WIDTH = 3;
  static constexpr float DEFAULT_LOW_DENSITY_BLUR_THRESHOLD = 0.99F;
  float m_lowDensityBlurThreshold = DEFAULT_LOW_DENSITY_BLUR_THRESHOLD;
  [[nodiscard]] auto BlurLowDensityColors(size_t numPoints,
                                          const std::vector<IfsPoint>& lowDensityPoints) const
      -> bool;
  auto SetLowDensityColors(const std::vector<IfsPoint>& points, uint32_t maxLowDensityCount) const
      -> void;
  auto UpdateLowDensityBlurThreshold() -> void;
  [[nodiscard]] auto GetNewBlurWidth() const -> uint32_t;

  [[nodiscard]] auto MegaChangeColorMapEvent() const -> bool;
  [[nodiscard]] auto IfsRenewEvent() const -> bool;
  const Weights<BlurrerColorMode> m_blurrerColorModeWeights;
};

IfsDancersFx::IfsDancersFx(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<IfsDancersFxImpl>(fxHelper, smallBitmaps)}
{
}

auto IfsDancersFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_fxImpl->SetWeightedColorMaps(weightedColorMaps);
}

auto IfsDancersFx::Start() noexcept -> void
{
  m_fxImpl->Start();
}

auto IfsDancersFx::Finish() noexcept -> void
{
  m_fxImpl->Finish();
}

auto IfsDancersFx::Resume() noexcept -> void
{
  // nothing to do
}

auto IfsDancersFx::Suspend() noexcept -> void
{
  // nothing to do
}

auto IfsDancersFx::GetFxName() const noexcept -> std::string
{
  return "IFS FX";
}

auto IfsDancersFx::ApplyNoDraw() noexcept -> void
{
  m_fxImpl->ApplyNoDraw();
}

auto IfsDancersFx::ApplyMultiple() noexcept -> void
{
  m_fxImpl->UpdateIfs();
}

auto IfsDancersFx::Refresh() noexcept -> void
{
  m_fxImpl->Refresh();
}

auto IfsDancersFx::PostStateUpdate(const bool wasActiveInPreviousState) noexcept -> void
{
  m_fxImpl->PostStateUpdate(wasActiveInPreviousState);
}

// clang-format off
static constexpr float BLURRER_COLOR_MODE_SMOOTH_WITH_NEIGHBOURS_WEIGHT = 1000.0F;
static constexpr float BLURRER_COLOR_MODE_SMOOTH_NO_NEIGHBOURS_WEIGHT   =    1.0F;
static constexpr float BLURRER_COLOR_MODE_SIMI_WITH_NEIGHBOURS_WEIGHT   =    1.0F;
static constexpr float BLURRER_COLOR_MODE_SIMI_NO_NEIGHBOURS_WEIGHT     =    5.0F;
static constexpr float BLURRER_COLOR_MODE_SINGLE_WITH_NEIGHBOURS_WEIGHT =    1.0F;
static constexpr float BLURRER_COLOR_MODE_SINGLE_NO_NEIGHBOURS_WEIGHT   =    5.0F;
// clang-format on

IfsDancersFx::IfsDancersFxImpl::IfsDancersFxImpl(const FxHelper& fxHelper,
                                                 const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_colorizer{m_goomRand},
    m_fractal{std::make_unique<Fractal>(m_draw.GetScreenWidth(),
                                        m_draw.GetScreenHeight(),
                                        m_goomRand,
                                        m_colorizer.GetColorMaps(),
                                        smallBitmaps)},
    m_blurrer{m_draw, m_goomRand, BLUR_WIDTH, &m_colorizer, smallBitmaps},
    // clang-format off
    m_blurrerColorModeWeights{
      m_goomRand,
      {
        {BlurrerColorMode::SMOOTH_WITH_NEIGHBOURS, BLURRER_COLOR_MODE_SMOOTH_WITH_NEIGHBOURS_WEIGHT},
        {BlurrerColorMode::SMOOTH_NO_NEIGHBOURS,   BLURRER_COLOR_MODE_SMOOTH_NO_NEIGHBOURS_WEIGHT},
        {BlurrerColorMode::SIMI_WITH_NEIGHBOURS,   BLURRER_COLOR_MODE_SIMI_WITH_NEIGHBOURS_WEIGHT},
        {BlurrerColorMode::SIMI_NO_NEIGHBOURS,     BLURRER_COLOR_MODE_SIMI_NO_NEIGHBOURS_WEIGHT},
        {BlurrerColorMode::SINGLE_WITH_NEIGHBOURS, BLURRER_COLOR_MODE_SINGLE_WITH_NEIGHBOURS_WEIGHT},
        {BlurrerColorMode::SINGLE_NO_NEIGHBOURS,   BLURRER_COLOR_MODE_SINGLE_NO_NEIGHBOURS_WEIGHT},
    }}
// clang-format on
{
}

inline auto IfsDancersFx::IfsDancersFxImpl::MegaChangeColorMapEvent() const -> bool
{
  static constexpr float PROB_MEGA_CHANGE_COLOR_MAP_EVENT = 0.5F;
  return m_goomRand.ProbabilityOf(PROB_MEGA_CHANGE_COLOR_MAP_EVENT);
}

inline auto IfsDancersFx::IfsDancersFxImpl::IfsRenewEvent() const -> bool
{
  static constexpr float PROB_IFS_RENEW_EVENT = 2.0F / 3.0F;
  return m_goomRand.ProbabilityOf(PROB_IFS_RENEW_EVENT);
}

inline auto IfsDancersFx::IfsDancersFxImpl::InitFractal() -> void
{
  m_fractal->Init();
  UpdateLowDensityThreshold();
}

inline auto IfsDancersFx::IfsDancersFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  Expects(weightedColorMaps.mainColorMaps != nullptr);
  m_colorizer.SetWeightedColorMaps(weightedColorMaps.mainColorMaps);
}

inline auto IfsDancersFx::IfsDancersFxImpl::Start() -> void
{
  InitFractal();
}

inline auto IfsDancersFx::IfsDancersFxImpl::Finish() -> void
{
  // nothing to do
}

inline auto IfsDancersFx::IfsDancersFxImpl::PostStateUpdate(const bool wasActiveInPreviousState)
    -> void
{
  if (!wasActiveInPreviousState)
  {
    InitFractal();
  }

  UpdateIncr();

  if (IfsRenewEvent())
  {
    Renew();
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::Refresh() -> void
{
  Renew();
}

inline auto IfsDancersFx::IfsDancersFxImpl::Renew() -> void
{
  ChangeColorMaps();
  m_colorizer.ChangeColorMode();

  ChangeSpeed();
}

inline auto IfsDancersFx::IfsDancersFxImpl::ChangeSpeed() -> void
{
  static constexpr float MIN_SPEED_AMP = 1.1F;
  static constexpr float MAX_SPEED_AMP = 5.1F;
  static constexpr float MAX_SPEED_WEIGHT = 10.0F;
  const float speedAmp =
      std::min(m_goomRand.GetRandInRange(MIN_SPEED_AMP, MAX_SPEED_WEIGHT), MAX_SPEED_AMP);
  const float accelFactor = 1.0F / (1.1F - m_goomInfo.GetSoundInfo().GetAcceleration());

  m_fractal->SetSpeed(std::max(1U, static_cast<uint32_t>(speedAmp * accelFactor)));
}

inline auto IfsDancersFx::IfsDancersFxImpl::ChangeColorMaps() -> void
{
  m_colorizer.ChangeColorMaps();
  m_blurrer.SetColorMode(m_blurrerColorModeWeights.GetRandomWeighted());
  static constexpr float SINGLE_COLOR_T = 0.5F;
  m_blurrer.SetSingleColor(m_colorizer.GetColorMaps().GetRandomColorMap().GetColor(SINGLE_COLOR_T));
}

inline auto IfsDancersFx::IfsDancersFxImpl::ApplyNoDraw() -> void
{
  UpdateDecayAndRecay();
  UpdateDecay();
}

auto IfsDancersFx::IfsDancersFxImpl::UpdateIfs() -> void
{
  UpdateDecayAndRecay();
  if (GetIfsIncr() <= 0)
  {
    return;
  }

  UpdateCycle();

  DrawNextIfsPoints();
}

auto IfsDancersFx::IfsDancersFxImpl::UpdateDecayAndRecay() -> void
{
  static constexpr int32_t BY_TWO = 2;

  --m_decayIfs;
  if (m_decayIfs > 0)
  {
    m_ifsIncr += BY_TWO;
  }
  if (0 == m_decayIfs)
  {
    m_ifsIncr = 0;
  }

  if (m_recayIfs)
  {
    m_ifsIncr -= BY_TWO;
    --m_recayIfs;
    if ((0 == m_recayIfs) && (m_ifsIncr <= 0))
    {
      m_ifsIncr = 1;
    }
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::UpdateIncr() -> void
{
  if (m_ifsIncr <= 0)
  {
    static constexpr int32_t NEW_RECAY_IFS = 5;
    m_recayIfs = NEW_RECAY_IFS;
    static constexpr int32_t NEW_IFS_INCR = 11;
    m_ifsIncr = NEW_IFS_INCR;
    Renew();
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::UpdateDecay() -> void
{
  if ((m_ifsIncr > 0) && (m_decayIfs <= 0))
  {
    static constexpr int32_t NEW_DECAY_IFS = 100;
    m_decayIfs = NEW_DECAY_IFS;
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::GetIfsIncr() const -> int
{
  return m_ifsIncr;
}

inline auto IfsDancersFx::IfsDancersFxImpl::UpdateCycle() -> void
{
  // TODO: trouver meilleur soluce pour increment (mettre le code de gestion de l'ifs
  //       dans ce fichier)
  //       find the best solution for increment (put the management code of the ifs in this file)
  m_tMix.Increment();

  ++m_cycle;
  if (m_cycle < m_cycleLength)
  {
    return;
  }

  m_cycle = 0;
  m_cycleLength = m_goomRand.GetRandInRange(MIN_CYCLE_LENGTH, MAX_CYCLE_LENGTH + 1);

  UpdateLowDensityBlurThreshold();

  m_fractal->Reset();

  Renew();
}

inline auto IfsDancersFx::IfsDancersFxImpl::UpdateLowDensityBlurThreshold() -> void
{
  static constexpr float PROB_HIGH_BLUR_THRESHOLD = 0.75F;
  static constexpr float HIGH_BLUR_THRESHOLD = 0.99F;
  static constexpr float LOW_BLUR_THRESHOLD = 0.40F;
  m_lowDensityBlurThreshold =
      m_goomRand.ProbabilityOf(PROB_HIGH_BLUR_THRESHOLD) ? HIGH_BLUR_THRESHOLD : LOW_BLUR_THRESHOLD;
}

auto IfsDancersFx::IfsDancersFxImpl::DrawNextIfsPoints() -> void
{
  const std::vector<IfsPoint>& points = m_fractal->GetNextIfsPoints();
  const uint32_t maxHitCount = m_fractal->GetMaxHitCount();

  m_colorizer.SetMaxHitCount(maxHitCount);

  const size_t numPoints = points.size();
  const float tStep = (1 == numPoints) ? 0.0F : ((1.0F - 0.0F) / static_cast<float>(numPoints - 1));
  float t = -tStep;

  bool doneColorChange =
      (m_colorizer.GetColorMode() != IfsDancersFx::ColorMode::MEGA_MAP_COLOR_CHANGE) &&
      (m_colorizer.GetColorMode() != IfsDancersFx::ColorMode::MEGA_MIX_COLOR_CHANGE);
  uint32_t maxLowDensityCount = 0;
  uint32_t numSelectedPoints = 0;
  std::vector<IfsPoint> lowDensityPoints{};

  for (size_t i = 0; i < numPoints; i += static_cast<size_t>(GetIfsIncr()))
  {
    t += tStep;

    if (const auto [x, y] = std::pair(points[i].GetX(), points[i].GetY());
        (x >= m_goomInfo.GetScreenInfo().width) || (y >= m_goomInfo.GetScreenInfo().height))
    {
      continue;
    }

    if ((!doneColorChange) && MegaChangeColorMapEvent())
    {
      ChangeColorMaps();
      doneColorChange = true;
    }

    ++numSelectedPoints;
    DrawPoint(points[i], t, m_tMix());

    if (points[i].GetCount() <= m_lowDensityCount)
    {
      lowDensityPoints.emplace_back(points[i]);
      if (maxLowDensityCount < points[i].GetCount())
      {
        maxLowDensityCount = points[i].GetCount();
      }
    }
  }

  if (!BlurLowDensityColors(numSelectedPoints, lowDensityPoints))
  {
    SetLowDensityColors(lowDensityPoints, maxLowDensityCount);
  }
  else
  {
    // Enough dense points to make blurring worthwhile.
    if (constexpr float PROB_FIXED_MIX_FACTOR = 0.8F;
        m_goomRand.ProbabilityOf(PROB_FIXED_MIX_FACTOR))
    {
      static constexpr float FIXED_MIX_FACTOR = 0.98F;
      m_blurrer.SetNeighbourMixFactor(FIXED_MIX_FACTOR);
    }
    else
    {
      static constexpr float MIN_MIX_FACTOR = 0.9F;
      static constexpr float MAX_MIX_FACTOR = 1.0F;
      m_blurrer.SetNeighbourMixFactor(m_goomRand.GetRandInRange(MIN_MIX_FACTOR, MAX_MIX_FACTOR));
    }
    m_blurrer.DoBlur(lowDensityPoints, maxLowDensityCount);
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::DrawPoint(const IfsPoint& ifsPoint,
                                                      const float t,
                                                      const float tMix) const -> void
{
  const Point2dInt point = {static_cast<int32_t>(ifsPoint.GetX()),
                            static_cast<int32_t>(ifsPoint.GetY())};

  const auto tX = static_cast<float>(point.x) / static_cast<float>(m_draw.GetScreenWidth());
  const auto tY = static_cast<float>(point.y) / static_cast<float>(m_draw.GetScreenHeight());

  if (const Pixel baseColor = ifsPoint.GetSimi()->GetColorMap()->GetColor(t);
      nullptr == ifsPoint.GetSimi()->GetCurrentPointBitmap())
  {
    const Pixel mixedColor =
        m_colorizer.GetMixedColor(baseColor, ifsPoint.GetCount(), POINT_BRIGHTNESS, tMix, tX, tY);
    m_draw.DrawPixels(point, {mixedColor, mixedColor});
  }
  else
  {
    const Pixel mixedColor =
        m_colorizer.GetMixedColor(baseColor, ifsPoint.GetCount(), BITMAP_BRIGHTNESS, tMix, tX, tY);
    const auto getColor = [&mixedColor]([[maybe_unused]] const size_t x,
                                        [[maybe_unused]] const size_t y,
                                        [[maybe_unused]] const Pixel& bgnd) { return mixedColor; };
    const ImageBitmap& bitmap{*ifsPoint.GetSimi()->GetCurrentPointBitmap()};
    m_draw.Bitmap(point, bitmap, {getColor, getColor});
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::BlurLowDensityColors(
    const size_t numPoints, const std::vector<IfsPoint>& lowDensityPoints) const -> bool
{
  if (0 == numPoints)
  {
    return false;
  }
  return (static_cast<float>(lowDensityPoints.size()) / static_cast<float>(numPoints)) >
         m_lowDensityBlurThreshold;
}

inline auto IfsDancersFx::IfsDancersFxImpl::SetLowDensityColors(
    const std::vector<IfsPoint>& points, const uint32_t maxLowDensityCount) const -> void
{
  const float logMaxLowDensityCount = std::log(static_cast<float>(maxLowDensityCount));

  float t = 0.0F;
  const float tStep = 1.0F / static_cast<float>(points.size());
  for (const auto& point : points)
  {
    const float logAlpha =
        point.GetCount() <= 1
            ? 1.0F
            : (std::log(static_cast<float>(point.GetCount())) / logMaxLowDensityCount);

    DrawPoint(point, t, logAlpha);

    t += tStep;
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::UpdateLowDensityThreshold() -> void
{
  m_lowDensityCount = m_goomRand.GetRandInRange(MIN_DENSITY_COUNT, MAX_DENSITY_COUNT);

  m_blurrer.SetWidth(GetNewBlurWidth());
}

inline auto IfsDancersFx::IfsDancersFxImpl::GetNewBlurWidth() const -> uint32_t
{
  static constexpr uint32_t NUM_WIDTHS = 3;
  static constexpr uint32_t WIDTH_RANGE = (MAX_DENSITY_COUNT - MIN_DENSITY_COUNT) / NUM_WIDTHS;
  static constexpr uint32_t DOUBLE_WIDTH_RANGE = 2 * WIDTH_RANGE;

  static constexpr uint32_t LARGE_BLUR_WIDTH = 7;
  static constexpr uint32_t MEDIUM_BLUR_WIDTH = 5;
  static constexpr uint32_t SMALL_BLUR_WIDTH = 3;

  uint32_t blurWidth = SMALL_BLUR_WIDTH;

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
