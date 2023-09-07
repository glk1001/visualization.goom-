//#undef NO_LOGGING

#include "shapes.h"

#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/goom_types.h"
#include "goom/math20.h"
#include "goom/point2d.h"
#include "goom_plugin_info.h"
#include "shape_parts.h"
#include "utils/math/goom_rand_base.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::SHAPES
{

using COLOR::WeightedRandomColorMaps;
using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using UTILS::MATH::IGoomRand;

Shape::Shape(IGoomDraw& draw,
             const IGoomRand& goomRand,
             const PluginInfo& goomInfo,
             const Params& params,
             const PixelChannelType defaultAlpha) noexcept
  : m_goomRand{&goomRand},
    m_shapeParts{GetInitialShapeParts(draw, goomRand, goomInfo, params, defaultAlpha)}
{
}

auto Shape::GetInitialShapeParts(DRAW::IGoomDraw& draw,
                                 const IGoomRand& goomRand,
                                 const PluginInfo& goomInfo,
                                 const Params& params,
                                 const PixelChannelType defaultAlpha) noexcept
    -> std::vector<ShapePart>
{
  auto shapeParts = std::vector<ShapePart>{};

  for (auto i = 0U; i < NUM_SHAPE_PARTS; ++i)
  {
    static constexpr auto T_MIN_MAX_LERP = 0.5F;
    const auto shapePartParams           = ShapePart::Params{
        i,
        NUM_SHAPE_PARTS,
        params.minRadiusFraction,
        params.maxRadiusFraction,
        params.minShapeDotRadius,
        params.maxShapeDotRadius,
        params.maxNumShapePaths,
        T_MIN_MAX_LERP,
        params.zoomMidpoint,
        params.minNumShapePathSteps,
        params.maxNumShapePathSteps,
    };
    shapeParts.emplace_back(draw, goomRand, goomInfo, shapePartParams, defaultAlpha);
  }

  return shapeParts;
}

auto Shape::SetWeightedMainColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void
{
  m_meetingPointMainColorMapPtr =
      weightedMaps.GetRandomColorMapSharedPtr(WeightedRandomColorMaps::GetAllColorMapsTypes());

  std::for_each(begin(m_shapeParts),
                end(m_shapeParts),
                [&weightedMaps](ShapePart& shapePart)
                { shapePart.SetWeightedMainColorMaps(weightedMaps); });
}

auto Shape::SetWeightedLowColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void
{
  m_meetingPointLowColorMapPtr =
      weightedMaps.GetRandomColorMapSharedPtr(WeightedRandomColorMaps::GetAllColorMapsTypes());

  std::for_each(begin(m_shapeParts),
                end(m_shapeParts),
                [&weightedMaps](ShapePart& shapePart)
                { shapePart.SetWeightedLowColorMaps(weightedMaps); });
}

auto Shape::SetWeightedInnerColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void
{
  std::for_each(begin(m_shapeParts),
                end(m_shapeParts),
                [&weightedMaps](ShapePart& shapePart)
                { shapePart.SetWeightedInnerColorMaps(weightedMaps); });
}

auto Shape::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  if (static constexpr auto PROB_ACCEPT_NEW_MIDPOINT = 0.8F;
      not m_goomRand->ProbabilityOf(PROB_ACCEPT_NEW_MIDPOINT))
  {
    return;
  }

  std::for_each(begin(m_shapeParts),
                end(m_shapeParts),
                [&zoomMidpoint](ShapePart& shapePart)
                { shapePart.SetShapePathsTargetPoint(zoomMidpoint); });
}

auto Shape::SetShapePathsMinMaxNumSteps(
    const MinMaxValues<uint32_t>& minMaxShapePathsNumSteps) noexcept -> void
{
  std::for_each(begin(m_shapeParts),
                end(m_shapeParts),
                [&minMaxShapePathsNumSteps](ShapePart& shapePart)
                { shapePart.SetShapePathsMinMaxNumSteps(minMaxShapePathsNumSteps); });
}

auto Shape::Start() noexcept -> void
{
  SetFixedShapeNumSteps();

  std::for_each(
      begin(m_shapeParts), end(m_shapeParts), [](ShapePart& shapePart) { shapePart.Start(); });
}

auto Shape::Draw() noexcept -> void
{
  const auto shapePartParams = ShapePart::DrawParams{
      GetBrightnessAttenuation(),
      FirstShapePathAtMeetingPoint(),
      m_varyDotRadius,
      GetCurrentMeetingPointColors(),
  };
  std::for_each(begin(m_shapeParts),
                end(m_shapeParts),
                [&shapePartParams](ShapePart& shapePart) { shapePart.Draw(shapePartParams); });

  if (FirstShapePathAtMeetingPoint())
  {
    m_meetingPointColorsT.Increment();
  }
}

inline auto Shape::GetCurrentMeetingPointColors() const noexcept -> MultiplePixels
{
  return {m_meetingPointMainColorMapPtr->GetColor(m_meetingPointColorsT()),
          m_meetingPointLowColorMapPtr->GetColor(m_meetingPointColorsT())};
}

inline auto Shape::GetBrightnessAttenuation() const noexcept -> float
{
  if (not FirstShapePathsCloseToMeeting())
  {
    return 1.0F;
  }

  const auto distanceFromOne =
      1.0F - GetShapePart(0).GetFirstShapePathTDistanceFromClosestBoundary();

  const auto minBrightness       = 2.0F / static_cast<float>(GetTotalNumShapePaths());
  static constexpr auto EXPONENT = 25.0F;
  return STD20::lerp(1.0F, minBrightness, std::pow(distanceFromOne, EXPONENT));
}

auto Shape::Update() noexcept -> void
{
  std::for_each(
      begin(m_shapeParts), end(m_shapeParts), [](ShapePart& shapePart) { shapePart.Update(); });
}

auto Shape::DoRandomChanges() noexcept -> void
{
  static constexpr auto PROB_USE_EVEN_PART_NUMS_FOR_DIRECTION = 0.5F;
  const auto useEvenPartNumsForDirection =
      m_goomRand->ProbabilityOf(PROB_USE_EVEN_PART_NUMS_FOR_DIRECTION);

  std::for_each(begin(m_shapeParts),
                end(m_shapeParts),
                [&useEvenPartNumsForDirection](ShapePart& shapePart)
                {
                  shapePart.UseEvenShapePartNumsForDirection(useEvenPartNumsForDirection);
                  shapePart.DoRandomChanges();
                });
}

auto Shape::SetFixedShapeNumSteps() noexcept -> void
{
  m_fixedTMinMaxLerp   = ShapePart::GetNewRandomMinMaxLerpT(*m_goomRand, m_fixedTMinMaxLerp);
  const auto positionT = GetFirstShapePathPositionT();

  std::for_each(begin(m_shapeParts),
                end(m_shapeParts),
                [this, &positionT](ShapePart& shapePart)
                {
                  shapePart.UseFixedShapePathsNumSteps(m_fixedTMinMaxLerp);
                  shapePart.ResetTs(positionT);
                });
}

auto Shape::SetRandomShapeNumSteps() noexcept -> void
{
  std::for_each(begin(m_shapeParts),
                end(m_shapeParts),
                [](ShapePart& shapePart) { shapePart.UseRandomShapePathsNumSteps(); });
}

auto Shape::GetTotalNumShapePaths() const noexcept -> uint32_t
{
  auto total = 0U;

  const auto numShapeParts = GetNumShapeParts();
  for (auto i = 0U; i < numShapeParts; ++i)
  {
    const auto numShapePaths = m_shapeParts.at(i).GetNumShapePaths();
    for (auto j = 0U; j < numShapePaths; ++j)
    {
      ++total;
    }
  }

  return total;
}

} // namespace GOOM::VISUAL_FX::SHAPES
