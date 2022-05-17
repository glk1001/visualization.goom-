#include "shapes.h"

#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "goom_config.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "shape_parts.h"
#include "utils/math/goom_rand_base.h"

namespace GOOM::VISUAL_FX::SHAPES
{

using COLOR::RandomColorMaps;
using COLOR::RandomColorMapsManager;
using UTILS::MATH::IGoomRand;

Shape::Shape(const IGoomRand& goomRand,
             const PluginInfo& goomInfo,
             RandomColorMapsManager& colorMapsManager,
             const Params& params) noexcept
  : m_goomRand{goomRand},
    m_goomInfo{goomInfo},
    m_colorMapsManager{colorMapsManager},
    m_shapeParts{GetInitialShapeParts(params)}
{
}

auto Shape::GetInitialShapeParts(const Params& params) noexcept -> std::vector<ShapePart>
{
  std::vector<ShapePart> shapeParts{};

  for (uint32_t i = 0; i < NUM_SHAPE_PARTS; ++i)
  {
    static constexpr float T_MIN_MAX_LERP = 0.5F;
    const ShapePart::Params shapePartParams{
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
    shapeParts.emplace_back(m_goomRand, m_goomInfo, m_colorMapsManager, shapePartParams);
  }

  return shapeParts;
}

auto Shape::SetWeightedMainColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  Expects(AllColorMapsValid());
  std::for_each(begin(m_shapeParts), end(m_shapeParts),
                [&weightedMaps](ShapePart& shapePart)
                { shapePart.SetWeightedMainColorMaps(weightedMaps); });
  Ensures(AllColorMapsValid());
}

auto Shape::SetWeightedLowColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  Expects(AllColorMapsValid());
  std::for_each(begin(m_shapeParts), end(m_shapeParts),
                [&weightedMaps](ShapePart& shapePart)
                { shapePart.SetWeightedLowColorMaps(weightedMaps); });
  Ensures(AllColorMapsValid());
}

auto Shape::SetWeightedInnerColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  Expects(AllColorMapsValid());
  std::for_each(begin(m_shapeParts), end(m_shapeParts),
                [&weightedMaps](ShapePart& shapePart)
                { shapePart.SetWeightedInnerColorMaps(weightedMaps); });
  Ensures(AllColorMapsValid());
}

auto Shape::AllColorMapsValid() const noexcept -> bool
{
  for (const auto& shapePart : m_shapeParts)
  {
    const size_t numShapePaths = shapePart.GetNumShapePaths();
    for (size_t shapePathNum = 0; shapePathNum < numShapePaths; ++shapePathNum)
    {
      assert(shapePart.GetShapePath(shapePathNum).GetColorInfo().mainColorMapId.IsSet());
      assert(shapePart.GetShapePath(shapePathNum).GetColorInfo().lowColorMapId.IsSet());
      assert(shapePart.GetShapePath(shapePathNum).GetColorInfo().innerColorMapId.IsSet());
    }
  }
  return true;
}

auto Shape::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  if (static constexpr float PROB_ACCEPT_NEW_MIDPOINT = 0.8F;
      not m_goomRand.ProbabilityOf(PROB_ACCEPT_NEW_MIDPOINT))
  {
    return;
  }

  std::for_each(begin(m_shapeParts), end(m_shapeParts),
                [&zoomMidpoint](ShapePart& shapePart)
                { shapePart.SetShapePathsTargetPoint(zoomMidpoint); });
}

auto Shape::SetShapePathsMinMaxNumSteps(const uint32_t shapePathsMinNumSteps,
                                        const uint32_t shapePathsMaxNumSteps) noexcept -> void
{
  std::for_each(
      begin(m_shapeParts), end(m_shapeParts),
      [&shapePathsMinNumSteps, &shapePathsMaxNumSteps](ShapePart& shapePart)
      { shapePart.SetShapePathsMinMaxNumSteps(shapePathsMinNumSteps, shapePathsMaxNumSteps); });
}

auto Shape::Start() noexcept -> void
{
  SetFixedShapeNumSteps();

  std::for_each(begin(m_shapeParts), end(m_shapeParts),
                [](ShapePart& shapePart) { shapePart.Start(); });

  Ensures(AllColorMapsValid());
}

auto Shape::Update() noexcept -> void
{
  std::for_each(begin(m_shapeParts), end(m_shapeParts),
                [](ShapePart& shapePart) { shapePart.Update(); });
}

auto Shape::DoRandomChanges() noexcept -> void
{
  static constexpr float PROB_USE_EVEN_PART_NUMS_FOR_DIRECTION = 0.5F;
  const bool useEvenPartNumsForDirection =
      m_goomRand.ProbabilityOf(PROB_USE_EVEN_PART_NUMS_FOR_DIRECTION);

  std::for_each(begin(m_shapeParts), end(m_shapeParts),
                [&useEvenPartNumsForDirection](ShapePart& shapePart)
                {
                  shapePart.UseEvenShapePartNumsForDirection(useEvenPartNumsForDirection);
                  shapePart.DoRandomChanges();
                });
}

auto Shape::SetFixedShapeNumSteps() noexcept -> void
{
  m_fixedTMinMaxLerp = ShapePart::GetNewRandomMinMaxLerpT(m_goomRand, m_fixedTMinMaxLerp);
  const float positionT = GetFirstShapePathPositionT();

  std::for_each(begin(m_shapeParts), end(m_shapeParts),
                [this, &positionT](ShapePart& shapePart)
                {
                  shapePart.UseFixedShapePathsNumSteps(m_fixedTMinMaxLerp);
                  shapePart.ResetTs(positionT);
                });
}

auto Shape::SetRandomShapeNumSteps() noexcept -> void
{
  std::for_each(begin(m_shapeParts), end(m_shapeParts),
                [](ShapePart& shapePart) { shapePart.UseRandomShapePathsNumSteps(); });
}

auto Shape::GetTotalNumShapePaths() const noexcept -> uint32_t
{
  uint32_t total = 0;

  const size_t numShapeParts = GetNumShapeParts();
  for (size_t i = 0; i < numShapeParts; ++i)
  {
    const size_t numShapePaths = m_shapeParts.at(i).GetNumShapePaths();
    for (size_t j = 0; j < numShapePaths; ++j)
    {
      ++total;
    }
  }

  return total;
}

} // namespace GOOM::VISUAL_FX::SHAPES
