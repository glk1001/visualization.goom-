module;

//#undef NO_LOGGING

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

module Goom.VisualFx.ShapesFx;

import Goom.Utils.Timer;
import Goom.Utils.Graphics.PointUtils;
import Goom.Utils.Math.IncrementedValues;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.Misc;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.FxUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;
import :Shapes;

namespace GOOM::VISUAL_FX
{

using FX_UTILS::RandomPixelBlender;
using SHAPES::Shape;
using UTILS::Timer;
using UTILS::GRAPHICS::GetPointClippedToRectangle;
using UTILS::MATH::IncrementedValue;
using UTILS::MATH::TValue;
using UTILS::MATH::TWO_PI;

class ShapesFx::ShapesFxImpl
{
public:
  explicit ShapesFxImpl(FxHelper& fxHelper) noexcept;

  auto Start() noexcept -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;
  [[nodiscard]] static auto GetCurrentColorMapsNames() noexcept -> std::vector<std::string>;

  auto ApplyToImageBuffers() noexcept -> void;

private:
  FxHelper* m_fxHelper;
  Point2dInt m_screenCentre       = m_fxHelper->GetDimensions().GetCentrePoint();
  PixelChannelType m_defaultAlpha = DEFAULT_VISUAL_FX_ALPHA;

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  static constexpr float MIN_RADIUS_FRACTION = 0.2F;
  static constexpr float MAX_RADIUS_FRACTION = 0.5F;
  static_assert(MIN_RADIUS_FRACTION <= MAX_RADIUS_FRACTION);

  static constexpr uint32_t MIN_NUM_SHAPE_PATH_STEPS = 20;
  static constexpr uint32_t MAX_NUM_SHAPE_PATH_STEPS = 200;
  static_assert(0 < MIN_NUM_SHAPE_PATH_STEPS);
  static_assert(MIN_NUM_SHAPE_PATH_STEPS < MAX_NUM_SHAPE_PATH_STEPS);

  [[nodiscard]] auto GetShapes() noexcept -> std::array<Shape, NUM_SHAPES>;
  std::array<Shape, NUM_SHAPES> m_shapes;
  [[nodiscard]] auto GetShapeZoomMidpoints(const Point2dInt& zoomMidpoint) const noexcept
      -> std::array<Point2dInt, NUM_SHAPES>;
  [[nodiscard]] auto GetAdjustedZoomMidpoint(const Point2dInt& zoomMidpoint) const noexcept
      -> Point2dInt;
  [[nodiscard]] auto GetRadialZoomMidpoints() const noexcept -> std::array<Point2dInt, NUM_SHAPES>;
  [[nodiscard]] auto GetRandomZoomMidpoints(const Point2dInt& zoomMidpoint) const noexcept
      -> std::array<Point2dInt, NUM_SHAPES>;

  static constexpr uint32_t TIME_BEFORE_SYNCHRONISED_CHANGE = 5000;
  Timer m_synchronisedShapeChangesTimer{m_fxHelper->GetGoomTime(), TIME_BEFORE_SYNCHRONISED_CHANGE};

  static constexpr uint32_t MIN_INCREMENTS_PER_UPDATE = 1;
  static constexpr uint32_t MAX_INCREMENTS_PER_UPDATE = 100;
  static_assert(0 < MIN_INCREMENTS_PER_UPDATE);
  static_assert(MIN_INCREMENTS_PER_UPDATE <= MAX_INCREMENTS_PER_UPDATE);
  uint32_t m_numIncrementsPerUpdate = m_fxHelper->GetGoomRand().GetRandInRange(
      MIN_INCREMENTS_PER_UPDATE, MAX_INCREMENTS_PER_UPDATE + 1);
  auto UpdateShapeEffects() noexcept -> void;
  auto UpdateShapeSpeeds() noexcept -> void;
  auto SetShapeSpeeds() noexcept -> void;
  auto UpdateShapePathMinMaxNumSteps() noexcept -> void;
  auto UpdateShapes() noexcept -> void;
  static auto UpdateShape(Shape& shape) noexcept -> void;
  [[nodiscard]] auto GetNextNumIncrements() const noexcept -> size_t;
};

ShapesFx::ShapesFx(FxHelper& fxHelper) noexcept
  : m_pimpl{spimpl::make_unique_impl<ShapesFxImpl>(fxHelper)}
{
}

auto ShapesFx::GetFxName() const noexcept -> std::string
{
  return "shapes";
}

auto ShapesFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto ShapesFx::Finish() noexcept -> void
{
  // nothing to do
}

auto ShapesFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto ShapesFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto ShapesFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto ShapesFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return ShapesFxImpl::GetCurrentColorMapsNames();
}

auto ShapesFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

ShapesFx::ShapesFxImpl::ShapesFxImpl(FxHelper& fxHelper) noexcept
  : m_fxHelper{&fxHelper}, m_pixelBlender{fxHelper.GetGoomRand()}, m_shapes{GetShapes()}
{
  UpdateShapePathMinMaxNumSteps();
}

auto ShapesFx::ShapesFxImpl::GetShapes() noexcept -> std::array<Shape, NUM_SHAPES>
{
  const auto initialShapeZoomMidpoints = GetShapeZoomMidpoints(m_screenCentre);

  static constexpr auto SHAPE0_MIN_DOT_RADIUS = 10;
  static constexpr auto SHAPE0_MAX_DOT_RADIUS = 20;
  static constexpr auto SHAPE0_MAX_NUM_PATHS  = 6U;

  return {
      {
       Shape{*m_fxHelper,
       {MIN_RADIUS_FRACTION,
       MAX_RADIUS_FRACTION,
       SHAPE0_MIN_DOT_RADIUS,
       SHAPE0_MAX_DOT_RADIUS,
       SHAPE0_MAX_NUM_PATHS,
       initialShapeZoomMidpoints.at(0),
       MIN_NUM_SHAPE_PATH_STEPS,
       MAX_NUM_SHAPE_PATH_STEPS},
       m_defaultAlpha},
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

inline auto ShapesFx::ShapesFxImpl::GetCurrentColorMapsNames() noexcept -> std::vector<std::string>
{
  // TODO(glk) -- fix this
  return {};
}

inline auto ShapesFx::ShapesFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  const auto newWeightedColorMaps =
      GetWeightedColorMapsWithNewAlpha(weightedColorMaps, m_defaultAlpha);

  const auto shapeNum = newWeightedColorMaps.id;
  m_shapes.at(shapeNum).SetWeightedMainColorMaps(newWeightedColorMaps.mainColorMaps);
  m_shapes.at(shapeNum).SetWeightedLowColorMaps(newWeightedColorMaps.lowColorMaps);
  m_shapes.at(shapeNum).SetWeightedInnerColorMaps(newWeightedColorMaps.extraColorMaps);
}

inline auto ShapesFx::ShapesFxImpl::UpdateShapeEffects() noexcept -> void
{
  if (static constexpr auto PROB_UPDATE_NUM_INCREMENTS = 0.1F;
      m_fxHelper->GetGoomRand().ProbabilityOf(PROB_UPDATE_NUM_INCREMENTS))
  {
    m_numIncrementsPerUpdate = m_fxHelper->GetGoomRand().GetRandInRange(
        MIN_INCREMENTS_PER_UPDATE, MAX_INCREMENTS_PER_UPDATE + 1);
  }

  static constexpr auto PROB_VARY_DOT_RADIUS = 0.01F;
  const auto varyDotRadius = m_fxHelper->GetGoomRand().ProbabilityOf(PROB_VARY_DOT_RADIUS);
  std::for_each(begin(m_shapes),
                end(m_shapes),
                [&varyDotRadius](Shape& shape) { shape.SetVaryDotRadius(varyDotRadius); });
}

inline auto ShapesFx::ShapesFxImpl::UpdateShapePathMinMaxNumSteps() noexcept -> void
{
  const auto newMinMaxNumShapePathSteps =
      MinMaxValues<uint32_t>{m_numIncrementsPerUpdate * MIN_NUM_SHAPE_PATH_STEPS,
                             m_numIncrementsPerUpdate * MAX_NUM_SHAPE_PATH_STEPS};
  std::for_each(begin(m_shapes),
                end(m_shapes),
                [&newMinMaxNumShapePathSteps](Shape& shape)
                { shape.SetShapePathsMinMaxNumSteps(newMinMaxNumShapePathSteps); });
}

inline auto ShapesFx::ShapesFxImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto ShapesFx::ShapesFxImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  const auto adjustedZoomMidpoint = GetAdjustedZoomMidpoint(zoomMidpoint);
  const auto shapeZoomMidpoints   = GetShapeZoomMidpoints(adjustedZoomMidpoint);

  for (auto i = 0U; i < NUM_SHAPES; ++i)
  {
    m_shapes.at(i).SetZoomMidpoint(shapeZoomMidpoints.at(i));
  }

  UpdateShapeEffects();
}

auto ShapesFx::ShapesFxImpl::GetAdjustedZoomMidpoint(const Point2dInt& zoomMidpoint) const noexcept
    -> Point2dInt
{
  const auto xMax = m_fxHelper->GetDimensions().GetIntWidth() - 1;
  const auto yMax = m_fxHelper->GetDimensions().GetIntHeight() - 1;

  const auto minZoomMidpoint   = Point2dInt{xMax / 5, yMax / 5};
  const auto maxZoomMidpoint   = Point2dInt{xMax - minZoomMidpoint.x, yMax - minZoomMidpoint.y};
  const auto zoomClipRectangle = Rectangle2dInt{minZoomMidpoint, maxZoomMidpoint};

  return GetPointClippedToRectangle(
      zoomMidpoint, zoomClipRectangle, m_fxHelper->GetDimensions().GetCentrePoint());
}

auto ShapesFx::ShapesFxImpl::GetShapeZoomMidpoints(const Point2dInt& zoomMidpoint) const noexcept
    -> std::array<Point2dInt, NUM_SHAPES>
{
  if (m_screenCentre == zoomMidpoint)
  {
    return GetRadialZoomMidpoints();
  }

  return GetRandomZoomMidpoints(zoomMidpoint);
}

auto ShapesFx::ShapesFxImpl::GetRadialZoomMidpoints() const noexcept
    -> std::array<Point2dInt, NUM_SHAPES>
{
  auto shapeZoomMidpoints = std::array<Point2dInt, NUM_SHAPES>{};

  shapeZoomMidpoints.at(0) = m_screenCentre;

  if constexpr (NUM_SHAPES > 1U)
  {
    const auto radius = static_cast<float>(m_screenCentre.y) / 3.0F;
    auto angle =
        IncrementedValue<float>{0.0F, TWO_PI, TValue::StepType::SINGLE_CYCLE, NUM_SHAPES - 1};

    for (auto i = 1U; i < NUM_SHAPES; ++i)
    {
      const Vec2dFlt radialOffset{radius * std::cos(angle()), radius * std::sin(angle())};

      shapeZoomMidpoints.at(i) = m_screenCentre + ToVec2dInt(radialOffset);

      angle.Increment();
    }
  }

  return shapeZoomMidpoints;
}

auto ShapesFx::ShapesFxImpl::GetRandomZoomMidpoints(const Point2dInt& zoomMidpoint) const noexcept
    -> std::array<Point2dInt, NUM_SHAPES>
{
  auto shapeZoomMidpoints = std::array<Point2dInt, NUM_SHAPES>{};

  shapeZoomMidpoints.at(0) = zoomMidpoint;

  static constexpr auto MARGIN = 20;
  const auto width             = m_fxHelper->GetDimensions().GetIntWidth() - MARGIN;
  const auto height            = m_fxHelper->GetDimensions().GetIntHeight() - MARGIN;

  for (auto i = 1U; i < NUM_SHAPES; ++i)
  {
    shapeZoomMidpoints.at(i) = {m_fxHelper->GetGoomRand().GetRandInRange(MARGIN, width),
                                m_fxHelper->GetGoomRand().GetRandInRange(MARGIN, height)};
  }

  return shapeZoomMidpoints;
}

inline auto ShapesFx::ShapesFxImpl::Start() noexcept -> void
{
  UpdateShapeEffects();

  std::for_each(begin(m_shapes),
                end(m_shapes),
                [](Shape& shape)
                {
                  shape.SetFixedShapeNumSteps();
                  shape.Start();
                });
}

inline auto ShapesFx::ShapesFxImpl::ApplyToImageBuffers() noexcept -> void
{
  m_fxHelper->GetBlend2dContexts().blend2dBuffersWereUsed = true;

  UpdatePixelBlender();
  UpdateShapeSpeeds();
  UpdateShapes();
}

inline auto ShapesFx::ShapesFxImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->GetDraw().SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

inline auto ShapesFx::ShapesFxImpl::UpdateShapeSpeeds() noexcept -> void
{
  if (m_synchronisedShapeChangesTimer.Finished())
  {
    SetShapeSpeeds();
    m_synchronisedShapeChangesTimer.ResetToZero();
  }
}

inline auto ShapesFx::ShapesFxImpl::SetShapeSpeeds() noexcept -> void
{
  std::for_each(begin(m_shapes), end(m_shapes), [](Shape& shape) { shape.SetShapeNumSteps(); });
}

inline auto ShapesFx::ShapesFxImpl::UpdateShapes() noexcept -> void
{
  const auto numIncrements = GetNextNumIncrements();

  for (auto i = 0U; i < numIncrements; ++i)
  {
    std::for_each(begin(m_shapes), end(m_shapes), [](Shape& shape) { UpdateShape(shape); });
  }
}

inline auto ShapesFx::ShapesFxImpl::GetNextNumIncrements() const noexcept -> size_t
{
  static constexpr auto T_CUTOFF = 0.75F;
  auto tDistanceFromBoundary =
      m_shapes.at(0).GetShapePart(0).GetFirstShapePathTDistanceFromClosestBoundary();

  if (tDistanceFromBoundary > T_CUTOFF)
  {
    return m_numIncrementsPerUpdate;
  }

  tDistanceFromBoundary /= T_CUTOFF;

  return static_cast<size_t>(std::lerp(1U, m_numIncrementsPerUpdate, tDistanceFromBoundary));
}

inline auto ShapesFx::ShapesFxImpl::UpdateShape(Shape& shape) noexcept -> void
{
  shape.Draw();
  shape.Update();
  shape.DoRandomChanges();
}

} // namespace GOOM::VISUAL_FX
