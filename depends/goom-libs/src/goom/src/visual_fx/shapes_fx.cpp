#include "shapes_fx.h"

#undef NO_LOGGING

#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "goom_config.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "shapes/shape_drawers.h"
#include "shapes/shapes.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <array>
#include <memory>

namespace GOOM::VISUAL_FX
{

using COLOR::RandomColorMaps;
using COLOR::RandomColorMapsManager;
using DRAW::IGoomDraw;
using SHAPES::Shape;
using SHAPES::ShapeDrawer;
using UTILS::Logging;
using UTILS::Timer;
using UTILS::TValue;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::TWO_PI;
using UTILS::MATH::U_HALF;

class ShapesFx::ShapesFxImpl
{
public:
  explicit ShapesFxImpl(const FxHelper& fxHelper) noexcept;

  auto SetWeightedMainColorMaps(size_t shapeNum,
                                std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;
  auto SetWeightedLowColorMaps(size_t shapeNum,
                               std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;
  auto SetWeightedInnerColorMaps(size_t shapeNum,
                                 std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;

  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto Start() noexcept -> void;

  auto ApplyMultiple() noexcept -> void;

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  RandomColorMapsManager m_colorMapsManager{};
  ShapeDrawer m_shapeDrawer{m_draw, m_goomRand, m_colorMapsManager};

  const Point2dInt m_screenMidPoint{U_HALF * m_goomInfo.GetScreenInfo().width,
                                    U_HALF* m_goomInfo.GetScreenInfo().height};

  static constexpr float MIN_RADIUS_FRACTION = 0.2F;
  static constexpr float MAX_RADIUS_FRACTION = 0.5F;
  static_assert(MIN_RADIUS_FRACTION <= MAX_RADIUS_FRACTION);

  static constexpr float MIN_SHAPE_PATH_SPEED_PER_UPDATE = 0.005F;
  static constexpr float MAX_SHAPE_PATH_SPEED_PER_UPDATE = 0.01F;
  static_assert(0.0F < MIN_SHAPE_PATH_SPEED_PER_UPDATE);
  static_assert(MIN_SHAPE_PATH_SPEED_PER_UPDATE <= MAX_SHAPE_PATH_SPEED_PER_UPDATE);
  static_assert(MAX_SHAPE_PATH_SPEED_PER_UPDATE < 1.0F);

  [[nodiscard]] auto GetShapes() noexcept -> std::array<Shape, NUM_SHAPES>;
  std::array<Shape, NUM_SHAPES> m_shapes{GetShapes()};
  [[nodiscard]] auto GetShapeZoomMidpoints(const Point2dInt& zoomMidpoint) const noexcept
      -> std::array<Point2dInt, NUM_SHAPES>;
  [[nodiscard]] auto GetAdjustedZoomMidpoint(const Point2dInt& zoomMidpoint) const noexcept
      -> Point2dInt;
  [[nodiscard]] auto GetRadialZoomMidpoints() const noexcept -> std::array<Point2dInt, NUM_SHAPES>;
  [[nodiscard]] auto GetRandomZoomMidpoints(const Point2dInt& zoomMidpoint) const noexcept
      -> std::array<Point2dInt, NUM_SHAPES>;

  static constexpr uint32_t TIME_BEFORE_SYNCHRONISED_CHANGE = 5000;
  Timer m_synchronisedShapeChangesTimer{TIME_BEFORE_SYNCHRONISED_CHANGE};

  auto UpdateShapeEffects() noexcept -> void;
  auto UpdateShapeSpeeds() noexcept -> void;
  auto SetShapeSpeeds() noexcept -> void;
  auto UpdateShapeMinMaxSpeeds() noexcept -> void;

  static constexpr uint32_t MIN_INCREMENTS_PER_UPDATE = 1;
  static constexpr uint32_t MAX_INCREMENTS_PER_UPDATE = 10;
  static_assert(0 < MIN_INCREMENTS_PER_UPDATE);
  static_assert(MIN_INCREMENTS_PER_UPDATE <= MAX_INCREMENTS_PER_UPDATE);
  uint32_t m_numIncrementsPerUpdate =
      m_goomRand.GetRandInRange(MIN_INCREMENTS_PER_UPDATE, MAX_INCREMENTS_PER_UPDATE + 1);
  auto UpdateShapes() noexcept -> void;
  auto UpdateShape(Shape& shape) noexcept -> void;
};

ShapesFx::ShapesFx(const FxHelper& fxHelper) noexcept
  : m_fxImpl{spimpl::make_unique_impl<ShapesFxImpl>(fxHelper)}
{
}

auto ShapesFx::GetFxName() const noexcept -> std::string
{
  return "shapes";
}

auto ShapesFx::SetWeightedMainColorMaps(
    const size_t shapeNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void
{
  m_fxImpl->SetWeightedMainColorMaps(shapeNum, weightedMaps);
}

auto ShapesFx::SetWeightedLowColorMaps(const size_t shapeNum,
                                       const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  m_fxImpl->SetWeightedLowColorMaps(shapeNum, weightedMaps);
}

auto ShapesFx::SetWeightedInnerColorMaps(
    const size_t shapeNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void
{
  m_fxImpl->SetWeightedInnerColorMaps(shapeNum, weightedMaps);
}

auto ShapesFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_fxImpl->SetZoomMidpoint(zoomMidpoint);
}

auto ShapesFx::Start() noexcept -> void
{
  m_fxImpl->Start();
}

auto ShapesFx::Finish() noexcept -> void
{
  // nothing to do
}

auto ShapesFx::ApplyMultiple() noexcept -> void
{
  m_fxImpl->ApplyMultiple();
}

ShapesFx::ShapesFxImpl::ShapesFxImpl(const FxHelper& fxHelper) noexcept
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()}
{
  UpdateShapeMinMaxSpeeds();
}

auto ShapesFx::ShapesFxImpl::GetShapes() noexcept -> std::array<Shape, NUM_SHAPES>
{
  const std::array<Point2dInt, NUM_SHAPES> initialShapeZoomMidpoints =
      GetShapeZoomMidpoints(m_screenMidPoint);

  static constexpr int32_t SHAPE0_MIN_DOT_RADIUS = 10;
  static constexpr int32_t SHAPE0_MAX_DOT_RADIUS = 20;
  static constexpr uint32_t SHAPE0_MAX_NUM_PATHS = 6;

  return {
      {
       Shape{m_goomRand,
       m_goomInfo,
       m_colorMapsManager,
       {MIN_RADIUS_FRACTION, MAX_RADIUS_FRACTION, SHAPE0_MIN_DOT_RADIUS,
       SHAPE0_MAX_DOT_RADIUS, SHAPE0_MAX_NUM_PATHS, initialShapeZoomMidpoints.at(0),
       MIN_SHAPE_PATH_SPEED_PER_UPDATE, MAX_SHAPE_PATH_SPEED_PER_UPDATE}},
       /**
       Shape{m_goomRand,
       m_goomInfo,
       m_colorMapsManager,
       {0.5F * MIN_RADIUS_FRACTION, 0.5F * MAX_RADIUS_FRACTION, 1, 3, 4,
       initialShapeZoomMidpoints.at(1)}},
       Shape{m_goomRand,
       m_goomInfo,
       m_colorMapsManager,
       {0.2F * MIN_RADIUS_FRACTION, 0.2F * MAX_RADIUS_FRACTION, 1, 3, 4,
       initialShapeZoomMidpoints.at(2)}},
        **/
      }
  };
}

inline auto ShapesFx::ShapesFxImpl::SetWeightedMainColorMaps(
    const size_t shapeNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void
{
  m_shapes.at(shapeNum).SetWeightedMainColorMaps(weightedMaps);
}

inline auto ShapesFx::ShapesFxImpl::SetWeightedLowColorMaps(
    const size_t shapeNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void
{
  m_shapes.at(shapeNum).SetWeightedLowColorMaps(weightedMaps);
}

inline auto ShapesFx::ShapesFxImpl::SetWeightedInnerColorMaps(
    const size_t shapeNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void
{
  m_shapes.at(shapeNum).SetWeightedInnerColorMaps(weightedMaps);
}

inline auto ShapesFx::ShapesFxImpl::UpdateShapeEffects() noexcept -> void
{
  if (static constexpr float PROB_UPDATE_NUM_INCREMENTS = 0.1F;
      m_goomRand.ProbabilityOf(PROB_UPDATE_NUM_INCREMENTS))
  {
    m_numIncrementsPerUpdate =
        m_goomRand.GetRandInRange(MIN_INCREMENTS_PER_UPDATE, MAX_INCREMENTS_PER_UPDATE + 1);
  }

  static constexpr float PROB_VARY_DOT_RADIUS = 0.1F;
  m_shapeDrawer.SetVaryDotRadius(m_goomRand.ProbabilityOf(PROB_VARY_DOT_RADIUS));

  static constexpr float PROJ_DOT_JITTER = 0.3F;
  m_shapeDrawer.SetDoDotJitter(m_goomRand.ProbabilityOf(PROJ_DOT_JITTER));
}

inline auto ShapesFx::ShapesFxImpl::UpdateShapeMinMaxSpeeds() noexcept -> void
{
  const auto pathSpeedFactor = 1.0F / static_cast<float>(m_numIncrementsPerUpdate);
  const float newMinShapePathSpeed = pathSpeedFactor * MIN_SHAPE_PATH_SPEED_PER_UPDATE;
  const float newMaxShapePathSpeed = pathSpeedFactor * MAX_SHAPE_PATH_SPEED_PER_UPDATE;
  std::for_each(begin(m_shapes), end(m_shapes),
                [&newMinShapePathSpeed, &newMaxShapePathSpeed](Shape& shape)
                { shape.SetMinMaxShapePathSpeeds(newMinShapePathSpeed, newMaxShapePathSpeed); });
}

inline auto ShapesFx::ShapesFxImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  const Point2dInt adjustedZoomMidpoint = GetAdjustedZoomMidpoint(zoomMidpoint);
  const std::array<Point2dInt, NUM_SHAPES> shapeZoomMidpoints =
      GetShapeZoomMidpoints(adjustedZoomMidpoint);

  for (size_t i = 0; i < NUM_SHAPES; ++i)
  {
    m_shapes.at(i).SetZoomMidpoint(shapeZoomMidpoints.at(i));
  }

  UpdateShapeEffects();
}

auto ShapesFx::ShapesFxImpl::GetAdjustedZoomMidpoint(const Point2dInt& zoomMidpoint) const noexcept
    -> Point2dInt
{
  const auto xMax = static_cast<int32_t>(m_goomInfo.GetScreenInfo().width - 1);
  const auto yMax = static_cast<int32_t>(m_goomInfo.GetScreenInfo().height - 1);
  const int32_t xCutoff = xMax / 5;
  const int32_t yCutoff = yMax / 5;

  return {
      std::clamp(zoomMidpoint.x, xCutoff, xMax - xCutoff),
      std::clamp(zoomMidpoint.y, yCutoff, yMax - yCutoff),
  };
}

auto ShapesFx::ShapesFxImpl::GetShapeZoomMidpoints(const Point2dInt& zoomMidpoint) const noexcept
    -> std::array<Point2dInt, NUM_SHAPES>
{
  if (m_screenMidPoint == zoomMidpoint)
  {
    return GetRadialZoomMidpoints();
  }

  return GetRandomZoomMidpoints(zoomMidpoint);
}

auto ShapesFx::ShapesFxImpl::GetRadialZoomMidpoints() const noexcept
    -> std::array<Point2dInt, NUM_SHAPES>
{
  std::array<Point2dInt, NUM_SHAPES> shapeZoomMidpoints{};

  shapeZoomMidpoints.at(0) = m_screenMidPoint;

  const auto radius = static_cast<float>(m_screenMidPoint.y) / 3.0F;
  TValue angleStep{TValue::StepType::SINGLE_CYCLE, NUM_SHAPES - 1};

  for (size_t i = 1; i < NUM_SHAPES; ++i)
  {
    const float angle = STD20::lerp(0.0F, TWO_PI, angleStep());
    const Vec2dFlt radialOffset{radius * std::cos(angle), radius * std::sin(angle)};

    shapeZoomMidpoints.at(i) = m_screenMidPoint + radialOffset.ToInt();

    angleStep.Increment();
  }

  return shapeZoomMidpoints;
}

auto ShapesFx::ShapesFxImpl::GetRandomZoomMidpoints(const Point2dInt& zoomMidpoint) const noexcept
    -> std::array<Point2dInt, NUM_SHAPES>
{
  std::array<Point2dInt, NUM_SHAPES> shapeZoomMidpoints{};

  shapeZoomMidpoints.at(0) = zoomMidpoint;

  static constexpr int32_t MARGIN = 20;
  const int32_t width = static_cast<int32_t>(m_goomInfo.GetScreenInfo().width) - MARGIN;
  const int32_t height = static_cast<int32_t>(m_goomInfo.GetScreenInfo().height) - MARGIN;

  for (size_t i = 1; i < NUM_SHAPES; ++i)
  {
    shapeZoomMidpoints.at(i) = {m_goomRand.GetRandInRange(MARGIN, width),
                                m_goomRand.GetRandInRange(MARGIN, height)};
  }

  return shapeZoomMidpoints;
}

inline auto ShapesFx::ShapesFxImpl::Start() noexcept -> void
{
  UpdateShapeEffects();

  std::for_each(begin(m_shapes), end(m_shapes),
                [](Shape& shape)
                {
                  shape.SetFixedShapeSpeeds();
                  shape.Start();
                });
}

inline auto ShapesFx::ShapesFxImpl::ApplyMultiple() noexcept -> void
{
  UpdateShapeSpeeds();
  UpdateShapes();
}

inline auto ShapesFx::ShapesFxImpl::UpdateShapeSpeeds() noexcept -> void
{
  m_synchronisedShapeChangesTimer.Increment();
  if (m_synchronisedShapeChangesTimer.Finished())
  {
    SetShapeSpeeds();
    m_synchronisedShapeChangesTimer.ResetToZero();
  }
}

inline auto ShapesFx::ShapesFxImpl::SetShapeSpeeds() noexcept -> void
{
  std::for_each(begin(m_shapes), end(m_shapes), [](Shape& shape) { shape.SetShapeSpeeds(); });
}

inline auto ShapesFx::ShapesFxImpl::UpdateShapes() noexcept -> void
{
  for (size_t i = 0; i < m_numIncrementsPerUpdate; ++i)
  {
    std::for_each(begin(m_shapes), end(m_shapes), [this](Shape& shape) { UpdateShape(shape); });
  }
}

inline auto ShapesFx::ShapesFxImpl::UpdateShape(Shape& shape) noexcept -> void
{
  m_shapeDrawer.DrawShapeParts(shape);
  shape.Update();
  shape.DoRandomChanges();
}

} // namespace GOOM::VISUAL_FX
