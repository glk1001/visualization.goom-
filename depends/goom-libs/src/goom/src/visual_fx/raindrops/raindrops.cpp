#undef NO_LOGGING

#include "raindrops.h"

#include "color/color_maps.h"
#include "color/color_utils.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "draw/goom_draw_to_buffer.h"
#include "goom/goom_config.h"
#include "goom/goom_logger.h"
#include "goom/goom_types.h"
#include "goom/math20.h"
#include "goom/point2d.h"
#include "utils/graphics/blend2d_to_goom.h"
#include "utils/graphics/pixel_blend.h"
#include "utils/graphics/point_utils.h"
#include "utils/math/misc.h"
#include "utils/t_values.h"
#include "visual_fx/fx_helper.h"

#include <algorithm>
#include <array>
#include <blend2d.h> // NOLINT(misc-include-cleaner): Blend2d insists on this.
#include <blend2d/context.h>
#include <blend2d/rgba.h>
#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::RAINDROPS
{

using COLOR::ColorMaps;
using COLOR::GetBrighterColor;
using COLOR::WeightedRandomColorMaps;
using DRAW::MultiplePixels;
using UTILS::IncrementedValue;
using UTILS::TValue;
using UTILS::GRAPHICS::Blend2dToGoom;
using UTILS::GRAPHICS::GetColorAlphaNoAddBlend;
using UTILS::GRAPHICS::GetMinSideLength;
using UTILS::GRAPHICS::GetPointClippedToRectangle;
using UTILS::MATH::U_HALF;

Raindrops::Raindrops(const FxHelper& fxHelper,
                     const uint32_t numRaindrops,
                     // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                     const WeightedRandomColorMaps& randomMainColorMaps,
                     const WeightedRandomColorMaps& randomLowColorMaps,
                     const Rectangle2dInt& rectangle2D,
                     const Point2dInt& targetRectangleWeightPoint) noexcept
  : m_fxHelper{&fxHelper},
    m_randomMainColorMaps{randomMainColorMaps},
    m_randomLowColorMaps{randomLowColorMaps},
    m_circleDrawer{*fxHelper.draw},
    m_lineDrawer{*fxHelper.draw},
    m_blend2dToMainBuffer{fxHelper.goomInfo->GetDimensions(), GetColorAlphaNoAddBlend},
    m_blend2dToLowBuffer{fxHelper.goomInfo->GetDimensions(), GetColorAlphaNoAddBlend},
    m_raindropParams{GetNewRaindropParams(rectangle2D)},
    m_raindropPositions{fxHelper,
                        GetAcceptableNumRaindrops(numRaindrops),
                        GetNewRaindropPositionParams(rectangle2D, targetRectangleWeightPoint)},
    m_raindrops{GetNewRaindrops(GetAcceptableNumRaindrops(numRaindrops))}
{
}

auto Raindrops::GetNewRaindropPositionParams(
    const Rectangle2dInt& rectangle2D, const Point2dInt& targetRectangleWeightPoint) const noexcept
    -> RaindropPositions::Params
{
  RaindropPositions::Params raindropPositionParams;

  raindropPositionParams.enclosingRadius =
      static_cast<float>(U_HALF * GetMinSideLength(rectangle2D));

  raindropPositionParams.numConcentricCircles = m_raindropParams.numConcentricCircles;

  raindropPositionParams.sourceRectangleWeightPoint =
      GetNewSourceRectangleWeightPoint(m_screenCentre);
  raindropPositionParams.targetRectangleWeightPoint = targetRectangleWeightPoint;

  return raindropPositionParams;
}

auto Raindrops::GetNewSourceRectangleWeightPoint(const Point2dInt& focusPoint) const noexcept
    -> Point2dInt
{
  if (focusPoint != m_screenCentre)
  {
    return {m_fxHelper->goomInfo->GetDimensions().GetIntWidth() - focusPoint.x,
            m_fxHelper->goomInfo->GetDimensions().GetIntHeight() - focusPoint.y};
  }

  const auto xSign = m_fxHelper->goomRand->ProbabilityOf(UTILS::MATH::HALF) ? -1 : +1;
  const auto ySign = m_fxHelper->goomRand->ProbabilityOf(UTILS::MATH::HALF) ? -1 : +1;
  return {m_screenCentre.x + xSign * (m_fxHelper->goomInfo->GetDimensions().GetIntWidth() / 4),
          m_screenCentre.y + ySign * (m_fxHelper->goomInfo->GetDimensions().GetIntHeight() / 4)};
}

auto Raindrops::GetNewRaindropParams(const Rectangle2dInt& rectangle2D) const noexcept
    -> RaindropParams
{
  RaindropParams raindropParams;

  const auto maxEnclosingRadius = static_cast<float>(U_HALF * GetMinSideLength(rectangle2D));

  raindropParams.numConcentricCircles = m_fxHelper->goomRand->GetRandInRange(
      MIN_NUM_CONCENTRIC_CIRCLES, MAX_NUM_CONCENTRIC_CIRCLES + 1);
  raindropParams.maxStartingRadius = RADIUS_TO_RECT_SIDE_FRAC * maxEnclosingRadius;
  raindropParams.minStartingRadius = MIN_TO_MAX_RADIUS_FRAC * raindropParams.maxStartingRadius;

  raindropParams.maxGrowthRadius = MAX_GROWTH_FACTOR * raindropParams.maxStartingRadius;

  raindropParams.rectangle2D = {
      rectangle2D.topLeft + static_cast<int32_t>(raindropParams.maxGrowthRadius),
      rectangle2D.bottomRight - static_cast<int32_t>(raindropParams.maxGrowthRadius)};

  raindropParams.sameMainColorMap = m_randomMainColorMaps.GetRandomColorMap();
  raindropParams.sameLowColorMap  = m_randomLowColorMaps.GetRandomColorMap();

  return raindropParams;
}

auto Raindrops::GetAcceptableNumRaindrops(const uint32_t numRequestedRaindrops) const noexcept
    -> uint32_t
{
  return m_raindropParams.numConcentricCircles *
         (numRequestedRaindrops / m_raindropParams.numConcentricCircles);
}

auto Raindrops::GetNewRaindrops(const uint32_t numRaindrops) const noexcept -> std::vector<Raindrop>
{
  std::vector<Raindrop> raindrops{};
  raindrops.reserve(numRaindrops);

  for (auto i = 0U; i < numRaindrops; ++i)
  {
    raindrops.emplace_back(GetNewRaindrop(i));
  }

  return raindrops;
}

auto Raindrops::GetNewRaindrop(const uint32_t dropNum) const noexcept -> Raindrop
{
  const auto dropCentrePoint     = m_raindropPositions.GetPosition(dropNum);
  const auto fracFromWeightPoint = GetFracFromWeightPoint(dropCentrePoint);

  const auto numGrowthSteps =
      m_fxHelper->goomRand->GetRandInRange(MIN_GROWTH_STEPS, MAX_GROWTH_STEPS + 1);

  const auto startingRadius = m_fxHelper->goomRand->GetRandInRange(
      m_raindropParams.minStartingRadius, m_raindropParams.maxStartingRadius);
  const auto maxGrowthRadius =
      m_fxHelper->goomRand->GetRandInRange(startingRadius, m_raindropParams.maxGrowthRadius);

  static constexpr auto STEP_TYPE = TValue::StepType::CONTINUOUS_REVERSIBLE;

  return {
      dropNum,
      static_cast<uint8_t>(m_fxHelper->goomRand->GetRandInRange(1U, MAX_LINE_THICKNESS + 1U)),
      fracFromWeightPoint,
      IncrementedValue<float>{startingRadius, maxGrowthRadius, STEP_TYPE, numGrowthSteps},
      m_randomMainColorMaps.GetRandomColorMap(),
      m_randomLowColorMaps.GetRandomColorMap(),
      TValue{{STEP_TYPE, numGrowthSteps}}
  };
}

auto Raindrops::GetFracFromWeightPoint(const Point2dInt& position) const noexcept -> float
{
  return static_cast<float>(
             Distance(position, m_raindropPositions.GetCurrentRectangleWeightPoint())) /
         m_raindropPositions.GetEnclosingRadius();
}

auto Raindrops::UpdateAnyPendingNumRaindrops() noexcept -> void
{
  if ((m_pendingNewNumRaindrops == 0) or (m_raindrops.size() == m_pendingNewNumRaindrops))
  {
    return;
  }

  m_raindropParams.numConcentricCircles = m_fxHelper->goomRand->GetRandInRange(
      MIN_NUM_CONCENTRIC_CIRCLES, MAX_NUM_CONCENTRIC_CIRCLES + 1);

  const auto acceptableNumRaindrops = GetAcceptableNumRaindrops(m_pendingNewNumRaindrops);

  m_raindropPositions.ResetPositions(m_raindropParams.numConcentricCircles, acceptableNumRaindrops);

  if (acceptableNumRaindrops < m_raindrops.size())
  {
    const auto numExcessElements =
        static_cast<int64_t>(m_raindrops.size() - acceptableNumRaindrops);
    m_raindrops.erase(end(m_raindrops) - numExcessElements, end(m_raindrops));
  }
  else
  {
    const auto colorTToUse = m_raindrops.at(m_raindrops.back().dropNum).colorT();
    const auto nextDropNum = m_raindrops.back().dropNum + 1U;
    m_raindrops.reserve(acceptableNumRaindrops);
    for (auto i = nextDropNum; i < acceptableNumRaindrops; ++i)
    {
      auto& newRaindrop = m_raindrops.emplace_back(GetNewRaindrop(i));
      newRaindrop.colorT.Reset(colorTToUse);
    }
  }

  m_pendingNewNumRaindrops = 0U;

  Ensures(m_raindrops.size() == acceptableNumRaindrops);
}

auto Raindrops::SetRectangleWeightPoint(const Point2dInt& targetRectangleWeightPoint) noexcept
    -> void
{
  // Make sure the 'rectangleWeightPoint' is inside the raindrops rectangle.
  const auto clippedTargetPoint =
      GetPointClippedToRectangle(targetRectangleWeightPoint,
                                 m_raindropParams.rectangle2D,
                                 m_fxHelper->goomInfo->GetDimensions().GetCentrePoint());

  m_raindropPositions.SetTargetRectangleWeightPoint(clippedTargetPoint);
}

auto Raindrops::SetWeightedColorMaps(const WeightedRandomColorMaps& randomMainColorMaps,
                                     const WeightedRandomColorMaps& randomLowColorMaps) noexcept
    -> void
{
  m_randomMainColorMaps = randomMainColorMaps;
  m_randomLowColorMaps  = randomLowColorMaps;

  m_raindropParams.sameMainColorMap = m_randomMainColorMaps.GetRandomColorMap();
  m_raindropParams.sameLowColorMap  = m_randomLowColorMaps.GetRandomColorMap();

  m_mainWeightPointColor = m_raindropParams.sameMainColorMap.GetColor(0.0F);
  m_lowWeightPointColor  = m_raindropParams.sameLowColorMap.GetColor(0.0F);

  std::for_each(begin(m_raindrops),
                end(m_raindrops),
                [this](auto& raindrop)
                { raindrop.mainColorMap = m_randomMainColorMaps.GetRandomColorMap(); });
}

auto Raindrops::DrawRaindrops() noexcept -> void
{
  Blend2dClearAll();

  DrawCircleAroundWeightPoint();

  std::for_each(begin(m_raindrops),
                end(m_raindrops),
                [this](auto& raindrop) { DrawRaindrop(raindrop, GetRaindropColors(raindrop)); });

  AddBlend2dImagesToGoomBuffers();
}

auto Raindrops::GetRaindropColors(const Raindrop& raindrop) const noexcept -> DRAW::MultiplePixels
{
  const auto sameColorT       = raindrop.fracFromWeightPoint;
  const auto sameMainColorMap = m_raindropParams.sameMainColorMap;
  const auto sameLowColorMap  = m_raindropParams.sameLowColorMap;

  const auto colorT         = raindrop.colorT();
  const auto mainBrightness = STD20::lerp(MAX_DROP_BRIGHTNESS, MIN_DROP_BRIGHTNESS, colorT);
  const auto lowBrightness  = LOW_BRIGHTNESS_INCREASE * mainBrightness;

  const auto mainColor = m_colorAdjustment.GetAdjustment(
      mainBrightness,
      ColorMaps::GetColorMix(
          raindrop.mainColorMap.GetColor(colorT), sameMainColorMap.GetColor(sameColorT), 0.7F));
  const auto lowColor = m_colorAdjustment.GetAdjustment(
      lowBrightness,
      ColorMaps::GetColorMix(
          raindrop.lowColorMap.GetColor(colorT), sameLowColorMap.GetColor(sameColorT), 0.7F));

  //  LogInfo(*m_fxHelper->goomLogger,
  //          "Raindrop {}, position: ({},{}), mainBrightness: {}, lowBrightness: {}",
  //          raindrop.dropNum,
  //          m_raindropPositions.GetPosition(raindrop.dropNum).x,
  //          m_raindropPositions.GetPosition(raindrop.dropNum).y,
  //          mainBrightness,
  //          lowBrightness);
  //  LogInfo(*m_fxHelper->goomLogger,
  //          "Raindrop {}, colors: ({},{},{}), ({},{},{})",
  //          raindrop.dropNum,
  //          mainColor.R(),
  //          mainColor.G(),
  //          mainColor.B(),
  //          lowColor.R(),
  //          lowColor.G(),
  //          lowColor.B());

  return {mainColor, lowColor};
}

auto Raindrops::FillCircle(BLContext& blend2dContext,
                           const Pixel& color,
                           const float brightness,
                           const Point2dInt& centre,
                           const double radius) noexcept -> void
{
  auto radialGradient = BLGradient{BLRadialGradientValues(static_cast<double>(centre.x),
                                                          static_cast<double>(centre.y),
                                                          static_cast<double>(centre.x),
                                                          static_cast<double>(centre.y),
                                                          radius)};
  struct RadialStop
  {
    double offset;
    float brightnessFactor;
  };
  static constexpr auto RADIAL_STOPS = std::array{
      RadialStop{0.0, 0.25F},
      RadialStop{0.5, 0.50F},
      RadialStop{1.0, 1.00F},
  };
  for (const auto& stop : RADIAL_STOPS)
  {
    radialGradient.addStop(stop.offset,
                           BLRgba32(Blend2dToGoom::GetBlend2dColor(
                               GetBrighterColor(stop.brightnessFactor * brightness, color))));
  }

  blend2dContext.fillCircle(
      static_cast<double>(centre.x), static_cast<double>(centre.y), radius, radialGradient);
}

auto Raindrops::Blend2dClearAll() -> void
{
  m_blend2dToMainBuffer.GetBlend2DBuffer().blend2dContext.clearAll();
  m_blend2dToLowBuffer.GetBlend2DBuffer().blend2dContext.clearAll();
}

auto Raindrops::AddBlend2dImagesToGoomBuffers() -> void
{
  m_blend2dToMainBuffer.UpdateGoomBuffer(
      dynamic_cast<DRAW::GoomDrawToTwoBuffers&>(*m_fxHelper->draw).GetBuffer1());
  m_blend2dToLowBuffer.UpdateGoomBuffer(
      dynamic_cast<DRAW::GoomDrawToTwoBuffers&>(*m_fxHelper->draw).GetBuffer2());
}

auto Raindrops::DrawCircleAroundWeightPoint() noexcept -> void
{
  const auto position = m_raindropPositions.GetCurrentRectangleWeightPoint();
  const auto radius =
      static_cast<double>(WEIGHT_POINT_RADIUS_FRAC * m_raindropPositions.GetEnclosingRadius());

  FillCircle(m_blend2dToMainBuffer.GetBlend2DBuffer().blend2dContext,
             m_mainWeightPointColor,
             WEIGHT_POINT_CIRCLE_BRIGHTNESS,
             position,
             radius);
  FillCircle(m_blend2dToLowBuffer.GetBlend2DBuffer().blend2dContext,
             m_lowWeightPointColor,
             WEIGHT_POINT_CIRCLE_BRIGHTNESS,
             position,
             radius);
}

auto Raindrops::DrawRaindrop(const Raindrop& raindrop, const MultiplePixels& colors) noexcept
    -> void
{
  const auto position = m_raindropPositions.GetPosition(raindrop.dropNum);
  const auto radius   = static_cast<double>(raindrop.growthRadius());

  FillCircle(m_blend2dToMainBuffer.GetBlend2DBuffer().blend2dContext,
             DRAW::GetMainColor(colors),
             1.0F,
             position,
             radius);
  FillCircle(m_blend2dToLowBuffer.GetBlend2DBuffer().blend2dContext,
             DRAW::GetLowColor(colors),
             1.0F,
             position,
             radius);

  m_lineDrawer.SetLineThickness(raindrop.lineThickness);
  m_lineDrawer.DrawLine(position,
                        m_raindropPositions.GetCurrentRectangleWeightPoint(),
                        {GetBrighterColor(LINE_TO_TARGET_BRIGHTNESS, DRAW::GetMainColor(colors)),
                         GetBrighterColor(LINE_TO_TARGET_BRIGHTNESS, DRAW::GetLowColor(colors))});

  const auto nextDropNum = raindrop.dropNum == m_raindrops.size() - 1 ? 0 : raindrop.dropNum + 1;
  m_lineDrawer.DrawLine(
      position,
      m_raindropPositions.GetPosition(nextDropNum),
      {GetBrighterColor(LINE_TO_NEXT_DROP_BRIGHTNESS, DRAW::GetMainColor(colors)),
       GetBrighterColor(LINE_TO_NEXT_DROP_BRIGHTNESS, DRAW::GetLowColor(colors))});
}

auto Raindrops::UpdateRaindrops() noexcept -> void
{
  if (m_raindropPositions.OkToChangeNumRaindrops())
  {
    UpdateAnyPendingNumRaindrops();
  }

  m_raindropPositions.UpdatePositions();

  std::for_each(
      begin(m_raindrops), end(m_raindrops), [](auto& raindrop) { UpdateRaindrop(raindrop); });
}

auto Raindrops::UpdateRaindrop(Raindrop& raindrop) noexcept -> void
{
  raindrop.growthRadius.Increment();
  raindrop.colorT.Increment();
}

} // namespace GOOM::VISUAL_FX::RAINDROPS
