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

#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "ifs/colorizer.h"
#include "ifs/fractal.h"
#include "ifs/low_density_blurrer.h"
#include "utils/goom_rand_base.h"
#include "utils/graphics/small_image_bitmaps.h"
//#undef NO_LOGGING
#include "color/random_colormaps.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
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
using UTILS::IGoomRand;
using UTILS::TValue;
using UTILS::Weights;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;

class IfsDancersFx::IfsDancersFxImpl
{
public:
  IfsDancersFxImpl(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept;

  void ApplyNoDraw();
  void UpdateIfs();
  void UpdateLowDensityThreshold();
  void Refresh();

  void SetWeightedColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps);
  void PostStateUpdate(bool wasActiveInPreviousState);

  void Start();
  void Finish();

private:
  static constexpr int32_t MIN_CYCLE_LENGTH = 1000;
  static constexpr int32_t MAX_CYCLE_LENGTH = 2000;
  int32_t m_cycleLength = MIN_CYCLE_LENGTH;

  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;

  Colorizer m_colorizer;

  std::unique_ptr<Fractal> m_fractal{};

  void InitFractal();

  int32_t m_cycle = 0;
  int32_t m_ifsIncr = 1; // dessiner l'ifs (0 = non: > = increment)
  int32_t m_decayIfs = 0; // disparition de l'ifs
  int32_t m_recayIfs = 0; // dedisparition de l'ifs
  void UpdateIncr();
  [[nodiscard]] auto GetIfsIncr() const -> int;
  void UpdateCycle();
  void UpdateDecay();
  void UpdateDecayAndRecay();
  void Renew();

  // TODO Move to simi
  static constexpr float T_MIX_STARTING_VALUE = 0.01F;
  TValue m_tMix{TValue::StepType::CONTINUOUS_REVERSIBLE, T_MIX_STARTING_VALUE};
  static constexpr float POINT_BRIGHTNESS = 3.0F;
  static constexpr float BITMAP_BRIGHTNESS = 2.0F;
  void ChangeColorMaps();
  void ChangeSpeed();
  void DrawNextIfsPoints();
  void DrawPoint(const IfsPoint& point, float t, float tMix) const;

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
  void SetLowDensityColors(const std::vector<IfsPoint>& points, uint32_t maxLowDensityCount) const;
  void UpdateLowDensityBlurThreshold();
  [[nodiscard]] auto GetNewBlurWidth() const -> uint32_t;

  [[nodiscard]] auto MegaChangeColorMapEvent() -> bool;
  [[nodiscard]] auto IfsRenewEvent() -> bool;
  const Weights<BlurrerColorMode> m_blurrerColorModeWeights;
};

IfsDancersFx::IfsDancersFx(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<IfsDancersFxImpl>(fxHelper, smallBitmaps)}
{
}

void IfsDancersFx::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedColorMaps(weightedMaps);
}

void IfsDancersFx::PostStateUpdate(const bool wasActiveInPreviousState)
{
  m_fxImpl->PostStateUpdate(wasActiveInPreviousState);
}

void IfsDancersFx::Start()
{
  m_fxImpl->Start();
}

void IfsDancersFx::Resume()
{
  // nothing to do
}

void IfsDancersFx::Suspend()
{
  // nothing to do
}

void IfsDancersFx::Finish()
{
  m_fxImpl->Finish();
}

auto IfsDancersFx::GetFxName() const -> std::string
{
  return "IFS FX";
}

void IfsDancersFx::ApplyNoDraw()
{
  m_fxImpl->ApplyNoDraw();
}

void IfsDancersFx::ApplyMultiple()
{
  m_fxImpl->UpdateIfs();
}

void IfsDancersFx::Refresh()
{
  m_fxImpl->Refresh();
}

// clang-format off
constexpr float BLURRER_COLOR_MODE_SMOOTH_WITH_NEIGHBOURS_WEIGHT = 1000.0F;
constexpr float BLURRER_COLOR_MODE_SMOOTH_NO_NEIGHBOURS_WEIGHT   =    1.0F;
constexpr float BLURRER_COLOR_MODE_SIMI_WITH_NEIGHBOURS_WEIGHT   =    1.0F;
constexpr float BLURRER_COLOR_MODE_SIMI_NO_NEIGHBOURS_WEIGHT     =    5.0F;
constexpr float BLURRER_COLOR_MODE_SINGLE_WITH_NEIGHBOURS_WEIGHT =    1.0F;
constexpr float BLURRER_COLOR_MODE_SINGLE_NO_NEIGHBOURS_WEIGHT   =    5.0F;
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
    m_blurrer{m_draw, BLUR_WIDTH, &m_colorizer},
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

inline auto IfsDancersFx::IfsDancersFxImpl::MegaChangeColorMapEvent() -> bool
{
  constexpr float PROB_MEGA_CHANGE_COLOR_MAP_EVENT = 0.5F;
  return m_goomRand.ProbabilityOf(PROB_MEGA_CHANGE_COLOR_MAP_EVENT);
}

inline auto IfsDancersFx::IfsDancersFxImpl::IfsRenewEvent() -> bool
{
  constexpr float PROB_IFS_RENEW_EVENT = 2.0F / 3.0F;
  return m_goomRand.ProbabilityOf(PROB_IFS_RENEW_EVENT);
}

void IfsDancersFx::IfsDancersFxImpl::InitFractal()
{
  m_fractal->Init();
  UpdateLowDensityThreshold();
}

inline void IfsDancersFx::IfsDancersFxImpl::SetWeightedColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_colorizer.SetWeightedColorMaps(weightedMaps);
}

inline void IfsDancersFx::IfsDancersFxImpl::Start()
{
  InitFractal();
}

inline void IfsDancersFx::IfsDancersFxImpl::Finish()
{
  // nothing to do
}

inline void IfsDancersFx::IfsDancersFxImpl::PostStateUpdate(const bool wasActiveInPreviousState)
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

inline void IfsDancersFx::IfsDancersFxImpl::Refresh()
{
  Renew();
}

inline void IfsDancersFx::IfsDancersFxImpl::Renew()
{
  ChangeColorMaps();
  m_colorizer.ChangeColorMode();

  ChangeSpeed();
}

inline void IfsDancersFx::IfsDancersFxImpl::ChangeSpeed()
{
  constexpr float MIN_SPEED_AMP = 1.1F;
  constexpr float MAX_SPEED_AMP = 5.1F;
  constexpr float MAX_SPEED_WEIGHT = 10.0F;
  const float speedAmp =
      std::min(m_goomRand.GetRandInRange(MIN_SPEED_AMP, MAX_SPEED_WEIGHT), MAX_SPEED_AMP);
  const float accelFactor = 1.0F / (1.1F - m_goomInfo.GetSoundInfo().GetAcceleration());

  m_fractal->SetSpeed(std::max(1U, static_cast<uint32_t>(speedAmp * accelFactor)));
}

inline void IfsDancersFx::IfsDancersFxImpl::ChangeColorMaps()
{
  m_colorizer.ChangeColorMaps();
  m_blurrer.SetColorMode(m_blurrerColorModeWeights.GetRandomWeighted());
  constexpr float SINGLE_COLOR_T = 0.5F;
  m_blurrer.SetSingleColor(m_colorizer.GetColorMaps().GetRandomColorMap().GetColor(SINGLE_COLOR_T));
}

inline void IfsDancersFx::IfsDancersFxImpl::ApplyNoDraw()
{
  UpdateDecayAndRecay();
  UpdateDecay();
}

void IfsDancersFx::IfsDancersFxImpl::UpdateIfs()
{
  UpdateDecayAndRecay();
  if (GetIfsIncr() <= 0)
  {
    return;
  }

  UpdateCycle();

  DrawNextIfsPoints();
}

void IfsDancersFx::IfsDancersFxImpl::UpdateDecayAndRecay()
{
  constexpr int32_t BY_TWO = 2;

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

inline void IfsDancersFx::IfsDancersFxImpl::UpdateIncr()
{
  if (m_ifsIncr <= 0)
  {
    constexpr int32_t NEW_RECAY_IFS = 5;
    m_recayIfs = NEW_RECAY_IFS;
    constexpr int32_t NEW_IFS_INCR = 11;
    m_ifsIncr = NEW_IFS_INCR;
    Renew();
  }
}

inline void IfsDancersFx::IfsDancersFxImpl::UpdateDecay()
{
  if ((m_ifsIncr > 0) && (m_decayIfs <= 0))
  {
    constexpr int32_t NEW_DECAY_IFS = 100;
    m_decayIfs = NEW_DECAY_IFS;
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::GetIfsIncr() const -> int
{
  return m_ifsIncr;
}

inline void IfsDancersFx::IfsDancersFxImpl::UpdateCycle()
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

inline void IfsDancersFx::IfsDancersFxImpl::UpdateLowDensityBlurThreshold()
{
  constexpr float PROB_HIGH_BLUR_THRESHOLD = 0.75F;
  constexpr float HIGH_BLUR_THRESHOLD = 0.99F;
  constexpr float LOW_BLUR_THRESHOLD = 0.40F;
  m_lowDensityBlurThreshold =
      m_goomRand.ProbabilityOf(PROB_HIGH_BLUR_THRESHOLD) ? HIGH_BLUR_THRESHOLD : LOW_BLUR_THRESHOLD;
}

void IfsDancersFx::IfsDancersFxImpl::DrawNextIfsPoints()
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

    const uint32_t x = points[i].GetX();
    const uint32_t y = points[i].GetY();
    if ((x >= m_goomInfo.GetScreenInfo().width) || (y >= m_goomInfo.GetScreenInfo().height))
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
    constexpr float PROB_NON_RANDOM_MIX_FACTOR = 4.0F / 5.0F;
    if (m_goomRand.ProbabilityOf(PROB_NON_RANDOM_MIX_FACTOR))
    {
      constexpr float FIXED_MIX_FACTOR = 0.98F;
      m_blurrer.SetNeighbourMixFactor(FIXED_MIX_FACTOR);
    }
    else
    {
      constexpr float MIN_MIX_FACTOR = 0.9F;
      constexpr float MAX_MIX_FACTOR = 1.0F;
      m_blurrer.SetNeighbourMixFactor(m_goomRand.GetRandInRange(MIN_MIX_FACTOR, MAX_MIX_FACTOR));
    }
    m_blurrer.DoBlur(lowDensityPoints, maxLowDensityCount);
  }
}

inline void IfsDancersFx::IfsDancersFxImpl::DrawPoint(const IfsPoint& point,
                                                      const float t,
                                                      const float tMix) const
{
  const auto pointX = static_cast<int32_t>(point.GetX());
  const auto pointY = static_cast<int32_t>(point.GetY());

  const auto tX = static_cast<float>(pointX) / static_cast<float>(m_draw.GetScreenWidth());
  const auto tY = static_cast<float>(pointY) / static_cast<float>(m_draw.GetScreenHeight());

  const Pixel baseColor = point.GetSimi()->GetColorMap()->GetColor(t);

  if (nullptr == point.GetSimi()->GetCurrentPointBitmap())
  {
    const Pixel mixedColor =
        m_colorizer.GetMixedColor(baseColor, point.GetCount(), POINT_BRIGHTNESS, tMix, tX, tY);
    m_draw.DrawPixels(pointX, pointY, {mixedColor, mixedColor});
  }
  else
  {
    const Pixel mixedColor =
        m_colorizer.GetMixedColor(baseColor, point.GetCount(), BITMAP_BRIGHTNESS, tMix, tX, tY);
    const auto getColor = [&mixedColor]([[maybe_unused]] const size_t x,
                                        [[maybe_unused]] const size_t y,
                                        [[maybe_unused]] const Pixel& bgnd) { return mixedColor; };
    const ImageBitmap& bitmap{*point.GetSimi()->GetCurrentPointBitmap()};
    m_draw.Bitmap(pointX, pointY, bitmap, {getColor, getColor});
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

inline void IfsDancersFx::IfsDancersFxImpl::SetLowDensityColors(
    const std::vector<IfsPoint>& points, const uint32_t maxLowDensityCount) const
{
  const float logMaxLowDensityCount = std::log(static_cast<float>(maxLowDensityCount));

  float t = 0.0F;
  const float tStep = 1.0F / static_cast<float>(points.size());
  for (const auto& point : points)
  {
    const float logAlpha = point.GetCount() <= 1 ? 1.0F
                                                 : (std::log(static_cast<float>(point.GetCount())) /
                                                       logMaxLowDensityCount);

    DrawPoint(point, t, logAlpha);

    t += tStep;
  }
}

inline void IfsDancersFx::IfsDancersFxImpl::UpdateLowDensityThreshold()
{
  m_lowDensityCount = m_goomRand.GetRandInRange(MIN_DENSITY_COUNT, MAX_DENSITY_COUNT);

  m_blurrer.SetWidth(GetNewBlurWidth());
}

inline auto IfsDancersFx::IfsDancersFxImpl::GetNewBlurWidth() const -> uint32_t
{
  constexpr uint32_t NUM_WIDTHS = 3;
  constexpr uint32_t WIDTH_RANGE = (MAX_DENSITY_COUNT - MIN_DENSITY_COUNT) / NUM_WIDTHS;
  constexpr uint32_t DOUBLE_WIDTH_RANGE = 2 * WIDTH_RANGE;

  constexpr uint32_t LARGE_BLUR_WIDTH = 7;
  constexpr uint32_t MEDIUM_BLUR_WIDTH = 5;
  constexpr uint32_t SMALL_BLUR_WIDTH = 3;

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
