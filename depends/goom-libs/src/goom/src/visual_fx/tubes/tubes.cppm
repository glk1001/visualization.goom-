module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

module Goom.VisualFx.TubesFx:Tubes;

import Goom.Color.ColorAdjustment;
import Goom.Color.ColorMaps;
import Goom.Color.ColorUtils;
import Goom.Color.RandomColorMaps;
import Goom.Draw.GoomDrawBase;
import Goom.Utils.Timer;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.ParametricFunctions2d;
import Goom.Utils.Math.Paths;
import Goom.Utils.Math.TValues;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;
import :TubeData;

using GOOM::COLOR::ColorAdjustment;
using GOOM::COLOR::ColorMapPtrWrapper;
using GOOM::COLOR::ColorMaps;
using GOOM::COLOR::GetLightenedColor;
using GOOM::COLOR::WeightedRandomColorMaps;
using GOOM::DRAW::MultiplePixels;
using GOOM::UTILS::Timer;
using GOOM::UTILS::GRAPHICS::SmallImageBitmaps;
using GOOM::UTILS::MATH::ISimplePath;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::OscillatingFunction;
using GOOM::UTILS::MATH::OscillatingPath;
using GOOM::UTILS::MATH::PI;
using GOOM::UTILS::MATH::SMALL_FLOAT;
using GOOM::UTILS::MATH::Sq;
using GOOM::UTILS::MATH::StartAndEndPos;
using GOOM::UTILS::MATH::THIRD_PI;
using GOOM::UTILS::MATH::TValue;
using GOOM::UTILS::MATH::TWO_PI;
using GOOM::UTILS::MATH::U_HALF;
using GOOM::UTILS::MATH::Weights;

namespace GOOM::VISUAL_FX::TUBES
{

enum class ColorMapMixMode : UnderlyingEnumType
{
  SHAPES_ONLY,
  STRIPED_SHAPES_ONLY,
  CIRCLES_ONLY,
  SHAPES_AND_CIRCLES,
  STRIPED_SHAPES_AND_CIRCLES,
};

struct ShapeColors
{
  Pixel mainColor;
  Pixel lowColor;
  Pixel innerMainColor;
  Pixel innerLowColor;
  Pixel outerCircleMainColor;
  Pixel outerCircleLowColor;
};

class BrightnessAttenuation
{
public:
  static constexpr float DIST_SQ_CUTOFF = 0.10F;
  struct Properties
  {
    uint32_t screenWidth{};
    uint32_t screenHeight{};
    float cutoffBrightness{};
  };

  explicit BrightnessAttenuation(const Properties& properties) noexcept;
  [[nodiscard]] auto GetPositionBrightness(const Point2dInt& pos,
                                           float minBrightnessPastCutoff) const noexcept -> float;

private:
  float m_cutoffBrightness;
  uint32_t m_maxRSquared;
  [[nodiscard]] auto GetDistFromCentreFactor(const Point2dInt& pos) const noexcept -> float;
};

class Tube
{
public:
  Tube(const TubeData& data, const OscillatingFunction::Params& pathParams) noexcept;

  [[nodiscard]] auto IsActive() const noexcept -> bool;

  auto SetWeightedMainColorMaps(const COLOR::WeightedRandomColorMaps& weightedMaps) noexcept
      -> void;
  auto SetWeightedLowColorMaps(const COLOR::WeightedRandomColorMaps& weightedMaps) noexcept -> void;

  auto ResetColorMaps() noexcept -> void;

  auto SetBrightnessFactor(float val) noexcept -> void;

  auto SetMaxJitterOffset(int32_t val) noexcept -> void;

  using TransformCentreFunc = std::function<Vec2dInt(uint32_t tubeId, const Point2dInt& centre)>;
  auto SetTransformCentreFunc(const TransformCentreFunc& func) noexcept -> void;
  auto SetCentrePathT(float val) noexcept -> void;
  static const float NORMAL_CENTRE_SPEED;
  auto SetCentreSpeed(float val) noexcept -> void;
  auto IncreaseCentreSpeed() noexcept -> void;
  auto DecreaseCentreSpeed() noexcept -> void;

  auto SetAllowOscillatingCirclePaths(bool val) noexcept -> void;
  auto SetCirclePathParams(const OscillatingFunction::Params& params) noexcept -> void;
  static const float NORMAL_CIRCLE_SPEED;
  auto SetCircleSpeed(float val) noexcept -> void;
  auto IncreaseCircleSpeed() noexcept -> void;
  auto DecreaseCircleSpeed() noexcept -> void;

  auto DrawCircleOfShapes() noexcept -> void;

private:
  class TubeImpl;
  spimpl::unique_impl_ptr<TubeImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX::TUBES


namespace GOOM::VISUAL_FX::TUBES
{

static constexpr auto NUM_SHAPES_PER_TUBE = 45U;
// Strangely, 'NUM_SHAPES_PER_TUBE = 100' gives a small gap in
// circle at 90 and 270 degrees.

static constexpr auto PROB_INTERIOR_SHAPE         = 45.0F / 50.0F;
static constexpr auto MAX_INTERIOR_SHAPES_TIME    = 500U;
static constexpr auto PROB_NO_BOUNDARY_SHAPES     = 10.0F / 50.0F;
static constexpr auto MAX_NO_BOUNDARY_SHAPES_TIME = 1U;
static constexpr auto PROB_HEX_DOT_SHAPE          = 1.0F / 50.0F;
static constexpr auto MAX_HEX_DOT_SHAPES_TIME     = 100U;

static constexpr auto STRIPE_WIDTH_RANGE =
    NumberRange{NUM_SHAPES_PER_TUBE / 6U, NUM_SHAPES_PER_TUBE / 3U};
static_assert(STRIPE_WIDTH_RANGE.min > 0);

static constexpr auto MAX_INDEX_CIRCLES_IN_GROUP_RANGE = NumberRange{10U, 100U};

static constexpr auto MIN_HEX_SIZE = 3.0F;
static constexpr auto MAX_HEX_SIZE = 9.0F;

static constexpr auto MIN_CIRCLE_SPEED = 0.0005F;
static constexpr auto NML_CIRCLE_SPEED = 0.005F;
static constexpr auto MAX_CIRCLE_SPEED = 0.008F;

static constexpr auto MIN_CENTRE_SPEED = 0.0005F;
static constexpr auto NML_CENTRE_SPEED = 0.005F;
static constexpr auto MAX_CENTRE_SPEED = 0.05F;

enum class LowColorTypes : UnderlyingEnumType
{
  TRUE_LOW_COLOR,
  MAIN_COLOR,
  LIGHTENED_LOW_COLOR,
};
static constexpr auto LOW_COLOR_TYPE_TIME_RANGE = NumberRange{100U, 1000U};

static constexpr auto OUTER_CIRCLE_BRIGHTNESS = 1.0F;
static constexpr auto LIGHTER_COLOR_POWER     = 100.0F;


struct Shape
{
  uint32_t shapeNum{};
  std::unique_ptr<OscillatingPath> path;
  uint8_t lineThickness{1};
};

class ShapeColorizer
{
public:
  struct ShapeColorMaps
  {
    ColorMapPtrWrapper mainColorMap{nullptr};
    ColorMapPtrWrapper lowColorMap{nullptr};
    ColorMapPtrWrapper innerMainColorMap{nullptr};
    ColorMapPtrWrapper innerLowColorMap{nullptr};
  };

  ShapeColorizer(uint32_t numShapes, const TubeData& data, uint32_t numCircles) noexcept;

  [[nodiscard]] auto GetBrightnessFactor() const noexcept -> float;
  auto SetBrightnessFactor(float val) noexcept -> void;

  auto SetWeightedMainColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void;
  auto SetWeightedLowColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void;

  auto ResetColorMaps() noexcept -> void;
  auto RotateShapeColorMaps() noexcept -> void;
  [[nodiscard]] auto GetColors(LowColorTypes lowColorType,
                               uint32_t circleNum,
                               const Shape& shape,
                               const Point2dInt& shapeCentrePos) const noexcept -> ShapeColors;
  auto UpdateAllTValues() noexcept -> void;

private:
  TubeData m_data;

  static constexpr auto GAMMA = 1.9F;
  ColorAdjustment m_colorAdjust{
      {.gamma = GAMMA, .alterChromaFactor = ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };

  std::vector<ShapeColorMaps> m_shapeColorMaps;
  std::vector<ShapeColors> m_oldShapeColors;
  static constexpr uint32_t NUM_SHAPE_COLOR_STEPS = 1000;
  TValue m_shapeColorsT{
      {.stepType = TValue::StepType::CONTINUOUS_REPEATABLE, .numSteps = NUM_SHAPE_COLOR_STEPS}
  };

  std::vector<ShapeColorMaps> m_circleColorMaps;
  std::vector<ShapeColors> m_oldCircleColors;
  TValue m_circleColorsT{
      {.stepType = TValue::StepType::CONTINUOUS_REPEATABLE,
       .numSteps = MAX_INDEX_CIRCLES_IN_GROUP_RANGE.max + 1}
  };

  ColorMapPtrWrapper m_outerCircleMainColorMap;
  ColorMapPtrWrapper m_outerCircleLowColorMap;
  static constexpr uint32_t NUM_OUTER_CIRCLE_COLOR_STEPS = 100;
  TValue m_outerCircleT{
      {.stepType = TValue::StepType::CONTINUOUS_REVERSIBLE,
       .numSteps = NUM_OUTER_CIRCLE_COLOR_STEPS}
  };

  ColorMapMixMode m_colorMapMixMode             = ColorMapMixMode::CIRCLES_ONLY;
  static constexpr uint32_t NUM_MIX_COLOR_STEPS = 1000;
  TValue m_mixT{
      {.stepType = TValue::StepType::CONTINUOUS_REVERSIBLE, .numSteps = NUM_MIX_COLOR_STEPS}
  };
  Weights<ColorMapMixMode> m_colorMapMixModes;

  static constexpr uint32_t NUM_STEPS_FROM_OLD = 50;
  TValue m_oldT{
      {.stepType = TValue::StepType::SINGLE_CYCLE, .numSteps = NUM_STEPS_FROM_OLD}
  };

  auto InitColorMaps() noexcept -> void;
  auto ResetColorMixMode() noexcept -> void;
  auto ResetColorMapsLists() noexcept -> void;
  auto ResetColorMapsList(std::vector<ShapeColorMaps>* colorMaps,
                          std::vector<ShapeColors>* oldColors,
                          TValue* t) noexcept -> void;
  auto ResetColorMaps(ShapeColorMaps* colorMaps) const noexcept -> void;
  auto CopyColors(const ShapeColorMaps& colorMaps,
                  const TValue& t,
                  ShapeColors* oldColors) const noexcept -> void;

  uint32_t m_stripeWidth = STRIPE_WIDTH_RANGE.min;
  [[nodiscard]] auto GetShapeNumToUse(uint32_t shapeNum) const noexcept -> uint32_t;
  [[nodiscard]] auto GetBrightness(const Shape& shape,
                                   const Point2dInt& shapeCentrePos) const noexcept -> float;

  [[nodiscard]] auto GetShapeColors(uint32_t shapeNum,
                                    float brightness) const noexcept -> ShapeColors;
  [[nodiscard]] auto GetCircleColors(uint32_t circleNum,
                                     float brightness) const noexcept -> ShapeColors;
  [[nodiscard]] auto GetColors(const ShapeColorMaps& shapeColorMaps,
                               const TValue& t,
                               const ShapeColors& oldShapeColors,
                               float brightness) const noexcept -> ShapeColors;
  [[nodiscard]] static auto GetLowColor(LowColorTypes colorType,
                                        const ShapeColors& colors) noexcept -> Pixel;
  [[nodiscard]] static auto GetLowMixedColor(LowColorTypes colorType,
                                             const ShapeColors& colors1,
                                             const ShapeColors& colors2,
                                             float mixT) noexcept -> Pixel;
  [[nodiscard]] static auto GetInnerLowColor(LowColorTypes colorType,
                                             const ShapeColors& colors) noexcept -> Pixel;
  [[nodiscard]] static auto GetInnerLowMixedColor(LowColorTypes colorType,
                                                  const ShapeColors& colors1,
                                                  const ShapeColors& colors2,
                                                  float mixT) noexcept -> Pixel;
  static constexpr float CUTOFF_BRIGHTNESS = 0.005F;
  BrightnessAttenuation m_brightnessAttenuation;
  [[nodiscard]] auto GetFinalColor(const Pixel& oldColor,
                                   const Pixel& color) const noexcept -> Pixel;
  [[nodiscard]] auto GetShapesOnlyColors(const LowColorTypes& lowColorType,
                                         const Shape& shape,
                                         float brightness) const noexcept -> ShapeColors;
  [[nodiscard]] auto GetCirclesOnlyColors(const LowColorTypes& lowColorType,
                                          uint32_t circleNum,
                                          float brightness) const noexcept -> ShapeColors;
  [[nodiscard]] auto GetShapesAndCirclesColors(const LowColorTypes& lowColorType,
                                               uint32_t circleNum,
                                               const Shape& shape,
                                               float brightness) const noexcept -> ShapeColors;
};

static constexpr auto TRUE_LOW_COLOR_WEIGHT      = 30.0F;
static constexpr auto MAIN_COLOR_WEIGHT          = 10.0F;
static constexpr auto LIGHTENED_LOW_COLOR_WEIGHT = 10.0F;

class TubeParametricPath : public ISimplePath
{
public:
  explicit TubeParametricPath(std::unique_ptr<TValue> positionT) noexcept;

  [[nodiscard]] auto GetClone() const noexcept -> std::unique_ptr<IPath> override;
  [[nodiscard]] auto GetNextPoint() const noexcept -> Point2dInt override;

private:
  static constexpr float DEFAULT_B   = 350.0F;
  float m_b                          = DEFAULT_B;
  static constexpr float DEFAULT_K_X = 3.0F;
  float m_kX                         = DEFAULT_K_X;
  static constexpr float DEFAULT_K_Y = 3.0F;
  float m_kY                         = DEFAULT_K_Y;
};

TubeParametricPath::TubeParametricPath(std::unique_ptr<TValue> positionT) noexcept
  : ISimplePath{std::move(positionT)}
{
}

auto TubeParametricPath::GetClone() const noexcept -> std::unique_ptr<IPath>
{
  return std::make_unique<TubeParametricPath>(std::make_unique<TValue>(GetPositionT()));
}

auto TubeParametricPath::GetNextPoint() const noexcept -> Point2dInt
{
  const auto point = Point2dInt{
      .x = static_cast<int32_t>(
          std::round((m_b * std::cos(m_kX * GetCurrentT())) * std::cos(GetCurrentT()))),
      .y = static_cast<int32_t>(
          std::round((m_b * std::cos(m_kY * GetCurrentT())) * std::sin(GetCurrentT()))),
  };

  return point;
}

class Tube::TubeImpl
{
public:
  TubeImpl(const TubeData& data, const OscillatingFunction::Params& pathParams) noexcept;

  [[nodiscard]] auto GetTubeId() const noexcept -> uint32_t;
  [[nodiscard]] auto IsActive() const noexcept -> bool;

  auto SetWeightedMainColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void;
  auto SetWeightedLowColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void;

  auto ResetColorMaps() noexcept -> void;
  auto RotateShapeColorMaps() noexcept -> void;

  [[nodiscard]] auto GetBrightnessFactor() const noexcept -> float;
  auto SetBrightnessFactor(float val) noexcept -> void;

  [[nodiscard]] auto GetMaxJitterOffset() const noexcept -> int32_t;
  auto SetMaxJitterOffset(int32_t val) noexcept -> void;

  auto SetTransformCentreFunc(const TransformCentreFunc& func) noexcept -> void;
  [[nodiscard]] auto GetCentrePathT() const noexcept -> float;
  auto SetCentrePathT(float val) noexcept -> void;
  [[nodiscard]] auto GetCentreSpeed() const noexcept -> float;
  auto SetCentreSpeed(float val) noexcept -> void;
  auto IncreaseCentreSpeed() noexcept -> void;
  auto DecreaseCentreSpeed() noexcept -> void;

  auto SetAllowOscillatingCirclePaths(bool val) noexcept -> void;
  auto SetCirclePathParams(const OscillatingFunction::Params& params) noexcept -> void;
  [[nodiscard]] auto GetCircleSpeed() const noexcept -> float;
  auto SetCircleSpeed(float val) noexcept -> void;
  auto IncreaseCircleSpeed() noexcept -> void;
  auto DecreaseCircleSpeed() noexcept -> void;

  auto DrawShapes() noexcept -> void;
  auto UpdateTValues() noexcept -> void;
  auto UpdateTimers() noexcept -> void;

private:
  TubeData m_data;
  std::unique_ptr<ShapeColorizer> m_colorizer{std::make_unique<ShapeColorizer>(
      NUM_SHAPES_PER_TUBE, m_data, MAX_INDEX_CIRCLES_IN_GROUP_RANGE.max + 1)};
  bool m_active                                = true;
  static constexpr float PATH_STEP             = NML_CIRCLE_SPEED;
  static constexpr uint32_t SHAPE_T_DELAY_TIME = 10;
  static constexpr float T_AT_CENTRE           = 0.5F;
  int32_t m_maxJitterOffset                    = 0;
  std::unique_ptr<TubeParametricPath> m_centrePath{
      std::make_unique<TubeParametricPath>(std::make_unique<TValue>(TValue::StepSizeProperties{
          .stepSize = PATH_STEP, .stepType = TValue::StepType::CONTINUOUS_REVERSIBLE}))};
  TransformCentreFunc m_getTransformedCentre;
  std::vector<Shape> m_shapes;
  [[nodiscard]] static auto GetInitialShapes(const TubeData& data,
                                             const OscillatingFunction::Params& pathParams) noexcept
      -> std::vector<Shape>;

  Timer m_circleGroupTimer{*m_data.goomTime,
                           m_data.goomRand->GetRandInRange<MAX_INDEX_CIRCLES_IN_GROUP_RANGE>()};
  Timer m_interiorShapeTimer{*m_data.goomTime, MAX_INTERIOR_SHAPES_TIME};
  Timer m_noBoundaryShapeTimer{*m_data.goomTime, MAX_NO_BOUNDARY_SHAPES_TIME};
  Timer m_hexDotShapeTimer{*m_data.goomTime, MAX_HEX_DOT_SHAPES_TIME, true};
  float m_hexLen = MIN_HEX_SIZE;
  [[nodiscard]] auto GetHexLen() const noexcept -> float;
  uint32_t m_interiorShapeSize{GetInteriorShapeSize(m_hexLen)};
  [[nodiscard]] auto GetInteriorShapeSize(float hexLen) const noexcept -> uint32_t;

  Timer m_lowColorTypeTimer{*m_data.goomTime, LOW_COLOR_TYPE_TIME_RANGE.max};
  LowColorTypes m_currentLowColorType = LowColorTypes::TRUE_LOW_COLOR;
  Weights<LowColorTypes> m_lowColorTypes;

  auto DrawShape(const Shape& shape, const Vec2dInt& centreOffset) const noexcept -> void;
  auto DrawInteriorShape(const Point2dInt& shapeCentrePos,
                         const ShapeColors& allColors) const noexcept -> void;
  auto DrawHexOutline(const Point2dInt& hexCentre,
                      const ShapeColors& allColors,
                      uint8_t lineThickness) const noexcept -> void;
  auto DrawOuterCircle(const Point2dInt& shapeCentrePos,
                       const ShapeColors& allColors) const noexcept -> void;
};

const float Tube::NORMAL_CENTRE_SPEED = NML_CENTRE_SPEED;
const float Tube::NORMAL_CIRCLE_SPEED = NML_CIRCLE_SPEED;

Tube::Tube(const TubeData& data, const OscillatingFunction::Params& pathParams) noexcept
  : m_pimpl{spimpl::make_unique_impl<Tube::TubeImpl>(data, pathParams)}
{
}

auto Tube::SetWeightedMainColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void
{
  m_pimpl->SetWeightedMainColorMaps(weightedMaps);
}

auto Tube::SetWeightedLowColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void
{
  m_pimpl->SetWeightedLowColorMaps(weightedMaps);
}

auto Tube::ResetColorMaps() noexcept -> void
{
  m_pimpl->ResetColorMaps();
}

auto Tube::IsActive() const noexcept -> bool
{
  return m_pimpl->IsActive();
}

auto Tube::SetTransformCentreFunc(const TransformCentreFunc& func) noexcept -> void
{
  m_pimpl->SetTransformCentreFunc(func);
}

auto Tube::SetCentrePathT(const float val) noexcept -> void
{
  m_pimpl->SetCentrePathT(val);
}

auto Tube::SetCentreSpeed(const float val) noexcept -> void
{
  m_pimpl->SetCentreSpeed(val);
}

auto Tube::IncreaseCentreSpeed() noexcept -> void
{
  m_pimpl->IncreaseCentreSpeed();
}

auto Tube::DecreaseCentreSpeed() noexcept -> void
{
  m_pimpl->DecreaseCentreSpeed();
}

auto Tube::SetAllowOscillatingCirclePaths(const bool val) noexcept -> void
{
  m_pimpl->SetAllowOscillatingCirclePaths(val);
}

auto Tube::SetCirclePathParams(const OscillatingFunction::Params& params) noexcept -> void
{
  m_pimpl->SetCirclePathParams(params);
}

auto Tube::SetCircleSpeed(const float val) noexcept -> void
{
  m_pimpl->SetCircleSpeed(val);
}

auto Tube::IncreaseCircleSpeed() noexcept -> void
{
  m_pimpl->IncreaseCircleSpeed();
}

auto Tube::DecreaseCircleSpeed() noexcept -> void
{
  m_pimpl->DecreaseCircleSpeed();
}

auto Tube::DrawCircleOfShapes() noexcept -> void
{
  m_pimpl->DrawShapes();
}

auto Tube::SetBrightnessFactor(const float val) noexcept -> void
{
  m_pimpl->SetBrightnessFactor(val);
}

auto Tube::SetMaxJitterOffset(const int32_t val) noexcept -> void
{
  m_pimpl->SetMaxJitterOffset(val);
}

Tube::TubeImpl::TubeImpl(
    const TubeData& data, const OscillatingFunction::Params& pathParams) noexcept
  : m_data{data},
    m_shapes{GetInitialShapes(m_data, pathParams)},
    m_lowColorTypes{
        *m_data.goomRand,
        {
            {.key=LowColorTypes::TRUE_LOW_COLOR,      .weight=TRUE_LOW_COLOR_WEIGHT},
            {.key=LowColorTypes::MAIN_COLOR,          .weight=MAIN_COLOR_WEIGHT},
            {.key=LowColorTypes::LIGHTENED_LOW_COLOR, .weight=LIGHTENED_LOW_COLOR_WEIGHT},
        }
    }
{
  Ensures(not m_shapes.empty());
}

auto Tube::TubeImpl::GetInitialShapes(const TubeData& data,
                                      const OscillatingFunction::Params& pathParams) noexcept
    -> std::vector<Shape>
{
  const auto middlePos = Point2dInt{.x = static_cast<int32_t>(U_HALF * data.screenWidth),
                                    .y = static_cast<int32_t>(U_HALF * data.screenHeight)};
  const auto radius = (0.5F * static_cast<float>(std::min(data.screenWidth, data.screenHeight))) -
                      data.radiusEdgeOffset;
  static constexpr auto ANGLE_STEP = TWO_PI / static_cast<float>(NUM_SHAPES_PER_TUBE);

  static_assert(NUM_SHAPES_PER_TUBE > 0);
  auto shapes = std::vector<Shape>(NUM_SHAPES_PER_TUBE);

  auto angle    = 0.0F;
  auto shapeNum = 0U;
  for (auto& shape : shapes)
  {
    const auto cosAngle = std::cos(angle);
    const auto sinAngle = std::sin(angle);
    const auto xFrom    = radius * cosAngle;
    const auto yFrom    = radius * sinAngle;
    const auto fromPos  = middlePos + Vec2dInt{.x = static_cast<int32_t>(std::round(xFrom)),
                                               .y = static_cast<int32_t>(std::round(yFrom))};
    const auto xTo      = radius * std::cos(PI + angle);
    const auto yTo      = radius * std::sin(PI + angle);
    const auto toPos    = middlePos + Vec2dInt{.x = static_cast<int32_t>(std::round(xTo)),
                                               .y = static_cast<int32_t>(std::round(yTo))};

    shape.shapeNum                   = shapeNum;
    static const auto s_DELAY_POINTS = std::vector<TValue::DelayPoint>{
        {       .t0 = 0.0F, .delayTime = SHAPE_T_DELAY_TIME},
        {.t0 = T_AT_CENTRE, .delayTime = SHAPE_T_DELAY_TIME},
        {       .t0 = 1.0F, .delayTime = SHAPE_T_DELAY_TIME}
    };
    auto shapeT = std::make_unique<TValue>(
        TValue::StepSizeProperties{.stepSize = PATH_STEP,
                                   .stepType = TValue::StepType::CONTINUOUS_REVERSIBLE},
        s_DELAY_POINTS);
    shape.path = std::make_unique<OscillatingPath>(
        std::move(shapeT),
        StartAndEndPos{.startPos = ToPoint2dFlt(fromPos), .endPos = ToPoint2dFlt(toPos)},
        pathParams);

    angle += ANGLE_STEP;
    ++shapeNum;
  }

  return shapes;
}

auto Tube::TubeImpl::SetWeightedMainColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept
    -> void
{
  m_colorizer->SetWeightedMainColorMaps(weightedMaps);
}

auto Tube::TubeImpl::SetWeightedLowColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept
    -> void
{
  m_colorizer->SetWeightedLowColorMaps(weightedMaps);
}

auto Tube::TubeImpl::ResetColorMaps() noexcept -> void
{
  m_colorizer->ResetColorMaps();
  m_circleGroupTimer.SetTimeLimitAndResetToZero(
      m_data.goomRand->GetRandInRange<MAX_INDEX_CIRCLES_IN_GROUP_RANGE>());
}

inline auto Tube::TubeImpl::RotateShapeColorMaps() noexcept -> void
{
  m_colorizer->RotateShapeColorMaps();
}

inline auto Tube::TubeImpl::GetTubeId() const noexcept -> uint32_t
{
  return m_data.tubeId;
}

inline auto Tube::TubeImpl::IsActive() const noexcept -> bool
{
  return m_active;
}

inline auto Tube::TubeImpl::GetBrightnessFactor() const noexcept -> float
{
  return m_colorizer->GetBrightnessFactor();
}

inline auto Tube::TubeImpl::SetBrightnessFactor(const float val) noexcept -> void
{
  m_colorizer->SetBrightnessFactor(val);
}

inline auto Tube::TubeImpl::GetMaxJitterOffset() const noexcept -> int32_t
{
  return m_maxJitterOffset;
}

inline auto Tube::TubeImpl::SetMaxJitterOffset(const int32_t val) noexcept -> void
{
  m_maxJitterOffset = val;
}

inline auto Tube::TubeImpl::SetTransformCentreFunc(const TransformCentreFunc& func) noexcept -> void
{
  m_getTransformedCentre = func;
}

inline auto Tube::TubeImpl::GetCentrePathT() const noexcept -> float
{
  return m_centrePath->GetCurrentT();
}

inline auto Tube::TubeImpl::SetCentrePathT(const float val) noexcept -> void
{
  m_centrePath->Reset(val);
}

inline auto Tube::TubeImpl::GetCentreSpeed() const noexcept -> float
{
  return m_centrePath->GetStepSize();
}

inline auto Tube::TubeImpl::SetCentreSpeed(const float val) noexcept -> void
{
  m_centrePath->SetStepSize(val);
}

inline auto Tube::TubeImpl::IncreaseCentreSpeed() noexcept -> void
{
  static constexpr auto FACTOR_RANGE = NumberRange{1.01F, 10.0F};
  const auto factor                  = m_data.goomRand->GetRandInRange<FACTOR_RANGE>();
  const auto newSpeed = std::min(MAX_CENTRE_SPEED, m_centrePath->GetStepSize() * factor);
  m_centrePath->SetStepSize(newSpeed);
}

inline auto Tube::TubeImpl::DecreaseCentreSpeed() noexcept -> void
{
  static constexpr auto FACTOR_RANGE = NumberRange{0.1F, 0.99F};
  const auto factor                  = m_data.goomRand->GetRandInRange<FACTOR_RANGE>();
  const auto newSpeed = std::min(MIN_CENTRE_SPEED, m_centrePath->GetStepSize() * factor);
  m_centrePath->SetStepSize(newSpeed);
}

inline auto Tube::TubeImpl::SetAllowOscillatingCirclePaths(const bool val) noexcept -> void
{
  for (auto& shape : m_shapes)
  {
    shape.path->GetParametricFunction().SetAllowOscillatingPath(val);
  }
}

inline auto Tube::TubeImpl::SetCirclePathParams(const OscillatingFunction::Params& params) noexcept
    -> void
{
  for (auto& shape : m_shapes)
  {
    shape.path->GetParametricFunction().SetParams(params);
  }
}

inline auto Tube::TubeImpl::GetCircleSpeed() const noexcept -> float
{
  return m_shapes.at(0).path->GetStepSize();
}

inline auto Tube::TubeImpl::SetCircleSpeed(const float val) noexcept -> void
{
  std::ranges::for_each(m_shapes, [&val](Shape& shape) { shape.path->SetStepSize(val); });
}

inline auto Tube::TubeImpl::IncreaseCircleSpeed() noexcept -> void
{
  static constexpr auto INCREASE_SPEED_FACTOR_RANGE = NumberRange{1.01F, 10.0F};
  const auto factor = m_data.goomRand->GetRandInRange<INCREASE_SPEED_FACTOR_RANGE>();

  const auto newSpeed = std::min(MAX_CIRCLE_SPEED, GetCircleSpeed() * factor);
  SetCircleSpeed(newSpeed);
}

inline auto Tube::TubeImpl::DecreaseCircleSpeed() noexcept -> void
{
  static constexpr auto DECREASE_SPEED_FACTOR_RANGE = NumberRange{0.1F, 0.99F};
  const auto factor = m_data.goomRand->GetRandInRange<DECREASE_SPEED_FACTOR_RANGE>();

  const auto newSpeed = std::max(MIN_CIRCLE_SPEED, GetCircleSpeed() * factor);
  SetCircleSpeed(newSpeed);
}

auto Tube::TubeImpl::DrawShapes() noexcept -> void
{
  m_hexLen            = GetHexLen();
  m_interiorShapeSize = GetInteriorShapeSize(m_hexLen);

  const auto centreOffset = m_getTransformedCentre(m_data.tubeId, m_centrePath->GetNextPoint());
  for (const auto& shape : m_shapes)
  {
    DrawShape(shape, centreOffset);
  }

  UpdateTValues();
  UpdateTimers();
}

inline auto Tube::TubeImpl::GetHexLen() const noexcept -> float
{
  const auto hexSizeT = std::fabs(m_shapes[0].path->GetCurrentT() - T_AT_CENTRE) / T_AT_CENTRE;
  return std::lerp(MIN_HEX_SIZE, MAX_HEX_SIZE, hexSizeT);
}

inline auto Tube::TubeImpl::UpdateTValues() noexcept -> void
{
  std::ranges::for_each(m_shapes, [](Shape& shape) { shape.path->IncrementT(); });
  m_centrePath->IncrementT();
  m_colorizer->UpdateAllTValues();
}

inline auto Tube::TubeImpl::UpdateTimers() noexcept -> void
{
  if (m_circleGroupTimer.Finished())
  {
    m_circleGroupTimer.ResetToZero();
  }

  if (m_interiorShapeTimer.Finished() and m_data.goomRand->ProbabilityOf<PROB_INTERIOR_SHAPE>())
  {
    m_interiorShapeTimer.ResetToZero();
  }

  if (m_noBoundaryShapeTimer.Finished() and
      m_data.goomRand->ProbabilityOf<PROB_NO_BOUNDARY_SHAPES>())
  {
    m_noBoundaryShapeTimer.ResetToZero();
  }

  if (m_hexDotShapeTimer.Finished() and m_data.goomRand->ProbabilityOf<PROB_HEX_DOT_SHAPE>())
  {
    m_hexDotShapeTimer.ResetToZero();
  }

  if (m_lowColorTypeTimer.Finished())
  {
    m_currentLowColorType = m_lowColorTypes.GetRandomWeighted();
    m_lowColorTypeTimer.SetTimeLimitAndResetToZero(
        m_data.goomRand->GetRandInRange<LOW_COLOR_TYPE_TIME_RANGE>());
  }
}

inline auto Tube::TubeImpl::GetInteriorShapeSize(const float hexLen) const noexcept -> uint32_t
{
  static constexpr auto SIZE_FACTOR_RANGE = NumberRange{0.5F, 1.3F};
  return static_cast<uint32_t>(
      std::round(m_data.goomRand->GetRandInRange<SIZE_FACTOR_RANGE>() * hexLen));
}

auto Tube::TubeImpl::DrawShape(const Shape& shape,
                               const Vec2dInt& centreOffset) const noexcept -> void
{
  const auto jitterXOffset  = m_data.goomRand->GetRandInRange(NumberRange{0, m_maxJitterOffset});
  const auto jitterYOffset  = jitterXOffset;
  const auto jitterOffset   = Vec2dInt{.x = jitterXOffset, .y = jitterYOffset};
  const auto shapeCentrePos = shape.path->GetNextPoint() + jitterOffset + centreOffset;

  const auto allColors =
      m_colorizer->GetColors(m_currentLowColorType,
                             static_cast<uint32_t>(m_circleGroupTimer.GetTimeElapsed()),
                             shape,
                             shapeCentrePos);

  if (m_noBoundaryShapeTimer.Finished())
  {
    DrawHexOutline(shapeCentrePos, allColors, shape.lineThickness);
  }

  static constexpr auto MIN_HEX_LEN_FOR_INTERIOR = 2.0F;
  if ((not m_interiorShapeTimer.Finished()) and
      (m_hexLen > (MIN_HEX_LEN_FOR_INTERIOR + SMALL_FLOAT)))
  {
    DrawInteriorShape(shapeCentrePos, allColors);
    DrawOuterCircle(shapeCentrePos, allColors);
  }
}

auto Tube::TubeImpl::DrawHexOutline(const Point2dInt& hexCentre,
                                    const ShapeColors& allColors,
                                    const uint8_t lineThickness) const noexcept -> void
{
  static constexpr auto NUM_HEX_SIDES = 6U;
  static constexpr auto ANGLE_STEP    = THIRD_PI;
  static constexpr auto START_ANGLE   = 2.0F * ANGLE_STEP;
  const auto lineColors =
      MultiplePixels{.color1 = allColors.mainColor, .color2 = allColors.lowColor};
  const auto outerCircleColors = MultiplePixels{.color1 = allColors.outerCircleMainColor,
                                                .color2 = allColors.outerCircleLowColor};
  const auto drawHexDot        = !m_hexDotShapeTimer.Finished();

  // Start hex shape to right of centre position.
  auto point1 =
      Point2dInt{.x = static_cast<int32_t>(std::round(static_cast<float>(hexCentre.x) + m_hexLen)),
                 .y = hexCentre.y};
  auto angle = START_ANGLE;

  for (auto i = 0U; i < NUM_HEX_SIDES; ++i)
  {
    const auto point2 =
        Point2dInt{.x = point1.x + static_cast<int32_t>(std::round(m_hexLen * std::cos(angle))),
                   .y = point1.y + static_cast<int32_t>(std::round(m_hexLen * std::sin(angle)))};

    m_data.drawFuncs.drawLine(point1, point2, lineColors, lineThickness);
    if (drawHexDot)
    {
      static constexpr uint32_t HEX_DOT_SIZE = 3;
      m_data.drawFuncs.drawSmallImage(
          point2, SmallImageBitmaps::ImageNames::SPHERE, HEX_DOT_SIZE, outerCircleColors);
    }

    angle += ANGLE_STEP;
    point1 = point2;
  }
}

inline auto Tube::TubeImpl::DrawInteriorShape(const Point2dInt& shapeCentrePos,
                                              const ShapeColors& allColors) const noexcept -> void
{
  const auto colors =
      MultiplePixels{.color1 = allColors.innerMainColor, .color2 = allColors.innerLowColor};
  m_data.drawFuncs.drawSmallImage(
      shapeCentrePos, SmallImageBitmaps::ImageNames::SPHERE, m_interiorShapeSize, colors);
}

inline auto Tube::TubeImpl::DrawOuterCircle(const Point2dInt& shapeCentrePos,
                                            const ShapeColors& allColors) const noexcept -> void
{
  static constexpr auto OUTER_CIRCLE_RADIUS_FACTOR = 1.5F;
  const auto outerCircleRadius =
      static_cast<int32_t>(std::round(OUTER_CIRCLE_RADIUS_FACTOR * m_hexLen));
  static constexpr auto OUTER_CIRCLE_LINE_THICKNESS = 1U;
  const auto outerCircleColors = MultiplePixels{.color1 = allColors.outerCircleMainColor,
                                                .color2 = allColors.outerCircleLowColor};
  m_data.drawFuncs.drawCircle(
      shapeCentrePos, outerCircleRadius, outerCircleColors, OUTER_CIRCLE_LINE_THICKNESS);
}

static constexpr auto SHAPES_ONLY_WEIGHT                = 20.0F;
static constexpr auto STRIPED_SHAPES_ONLY_WEIGHT        = 10.0F;
static constexpr auto CIRCLES_ONLY_WEIGHT               = 20.0F;
static constexpr auto SHAPES_AND_CIRCLES_WEIGHT         = 05.0F;
static constexpr auto STRIPED_SHAPES_AND_CIRCLES_WEIGHT = 15.0F;

ShapeColorizer::ShapeColorizer(const uint32_t numShapes,
                               const TubeData& data,
                               const uint32_t numCircles) noexcept
  : m_data{data},
    m_shapeColorMaps(numShapes),
    m_oldShapeColors(numShapes),
    m_circleColorMaps(numCircles),
    m_oldCircleColors(numCircles),
    m_outerCircleMainColorMap{m_data.mainColorMaps.GetRandomColorMap()},
    m_outerCircleLowColorMap{m_data.lowColorMaps.GetRandomColorMap()},
    m_colorMapMixModes{
        *m_data.goomRand,
        {
            {.key=ColorMapMixMode::SHAPES_ONLY,                .weight=SHAPES_ONLY_WEIGHT},
            {.key=ColorMapMixMode::STRIPED_SHAPES_ONLY,        .weight=STRIPED_SHAPES_ONLY_WEIGHT},
            {.key=ColorMapMixMode::CIRCLES_ONLY,               .weight=CIRCLES_ONLY_WEIGHT},
            {.key=ColorMapMixMode::SHAPES_AND_CIRCLES,         .weight=SHAPES_AND_CIRCLES_WEIGHT},
            {.key=ColorMapMixMode::STRIPED_SHAPES_AND_CIRCLES, .weight=STRIPED_SHAPES_AND_CIRCLES_WEIGHT},
        }
    },
    m_brightnessAttenuation{{.screenWidth=m_data.screenWidth, .screenHeight=m_data.screenHeight, .cutoffBrightness=CUTOFF_BRIGHTNESS}}
{
  Expects(numShapes > 0);
  Expects(numCircles > 0);
  Expects(numCircles <= (MAX_INDEX_CIRCLES_IN_GROUP_RANGE.max + 1));
  InitColorMaps();
  ResetColorMaps();
}

inline auto ShapeColorizer::GetBrightnessFactor() const noexcept -> float
{
  return m_data.brightnessFactor;
}

inline auto ShapeColorizer::SetBrightnessFactor(const float val) noexcept -> void
{
  m_data.brightnessFactor = val;
}

auto ShapeColorizer::InitColorMaps() noexcept -> void
{
  for (auto& cm : m_shapeColorMaps)
  {
    ResetColorMaps(&cm);
  }
  for (auto& cm : m_circleColorMaps)
  {
    ResetColorMaps(&cm);
  }
}

auto ShapeColorizer::SetWeightedMainColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept
    -> void
{
  m_data.mainColorMaps = weightedMaps;
}

auto ShapeColorizer::SetWeightedLowColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept
    -> void
{
  m_data.lowColorMaps = weightedMaps;
}

auto ShapeColorizer::ResetColorMaps() noexcept -> void
{
  ResetColorMixMode();
  ResetColorMapsLists();

  m_stripeWidth = m_data.goomRand->GetRandInRange<STRIPE_WIDTH_RANGE>();
}

inline auto ShapeColorizer::ResetColorMapsLists() noexcept -> void
{
  ResetColorMapsList(&m_shapeColorMaps, &m_oldShapeColors, &m_shapeColorsT);
  ResetColorMapsList(&m_circleColorMaps, &m_oldCircleColors, &m_circleColorsT);
}

auto ShapeColorizer::RotateShapeColorMaps() noexcept -> void
{
  std::ranges::rotate(m_shapeColorMaps, begin(m_shapeColorMaps) + 1);
  std::ranges::rotate(m_oldShapeColors, begin(m_oldShapeColors) + 1);
}

auto ShapeColorizer::ResetColorMapsList(std::vector<ShapeColorMaps>* const colorMaps,
                                        std::vector<ShapeColors>* const oldColors,
                                        TValue* const t) noexcept -> void
{
  const auto numColorMaps = colorMaps->size();
  Expects(numColorMaps == oldColors->size());

  m_outerCircleMainColorMap = m_data.mainColorMaps.GetRandomColorMap();
  m_outerCircleLowColorMap  = m_data.lowColorMaps.GetRandomColorMap();

  for (auto i = 0U; i < numColorMaps; ++i)
  {
    CopyColors((*colorMaps)[i], *t, &(*oldColors)[i]);
    ResetColorMaps(&(*colorMaps)[i]);
  }
  t->Reset(0.0);
}

auto ShapeColorizer::ResetColorMaps(ShapeColorMaps* const colorMaps) const noexcept -> void
{
  colorMaps->mainColorMap      = m_data.mainColorMaps.GetRandomColorMap();
  colorMaps->lowColorMap       = m_data.mainColorMaps.GetRandomColorMap();
  colorMaps->innerMainColorMap = m_data.lowColorMaps.GetRandomColorMap();
  colorMaps->innerLowColorMap  = m_data.lowColorMaps.GetRandomColorMap();
}

auto ShapeColorizer::CopyColors(const ShapeColorMaps& colorMaps,
                                const TValue& t,
                                ShapeColors* const oldColors) const noexcept -> void
{
  oldColors->mainColor            = colorMaps.mainColorMap.GetColor(t());
  oldColors->lowColor             = colorMaps.lowColorMap.GetColor(t());
  oldColors->innerMainColor       = colorMaps.innerMainColorMap.GetColor(t());
  oldColors->innerLowColor        = colorMaps.innerLowColorMap.GetColor(t());
  oldColors->outerCircleMainColor = m_outerCircleMainColorMap.GetColor(t());
  oldColors->outerCircleLowColor  = m_outerCircleLowColorMap.GetColor(t());
}

inline auto ShapeColorizer::ResetColorMixMode() noexcept -> void
{
  m_colorMapMixMode = m_colorMapMixModes.GetRandomWeighted();
}

auto ShapeColorizer::UpdateAllTValues() noexcept -> void
{
  m_shapeColorsT.Increment();
  m_circleColorsT.Increment();
  m_outerCircleT.Increment();
  m_oldT.Increment();
  m_mixT.Increment();
}

auto ShapeColorizer::GetBrightness(const Shape& shape,
                                   const Point2dInt& shapeCentrePos) const noexcept -> float
{
  static constexpr auto MIN_BRIGHTNESS = 1.0F;
  const auto brightness =
      std::min(5.0F,
               m_data.brightnessFactor *
                   m_brightnessAttenuation.GetPositionBrightness(shapeCentrePos, MIN_BRIGHTNESS));

  static constexpr auto SMALL_T = 0.15F;
  if (static constexpr float HALFWAY_T = 0.5F;
      std::fabs(shape.path->GetCurrentT() - HALFWAY_T) < SMALL_T)
  {
    static constexpr auto SMALL_T_BRIGHTNESS = 0.50F;
    return SMALL_T_BRIGHTNESS * brightness;
  }
  return brightness;
}

auto ShapeColorizer::GetColors(const LowColorTypes lowColorType,
                               const uint32_t circleNum,
                               const Shape& shape,
                               const Point2dInt& shapeCentrePos) const noexcept -> ShapeColors
{
  const auto brightness = GetBrightness(shape, shapeCentrePos);

  switch (m_colorMapMixMode)
  {
    case ColorMapMixMode::STRIPED_SHAPES_ONLY:
    case ColorMapMixMode::SHAPES_ONLY:
      return GetShapesOnlyColors(lowColorType, shape, brightness);
    case ColorMapMixMode::CIRCLES_ONLY:
      return GetCirclesOnlyColors(lowColorType, circleNum, brightness);
    case ColorMapMixMode::STRIPED_SHAPES_AND_CIRCLES:
    case ColorMapMixMode::SHAPES_AND_CIRCLES:
      return GetShapesAndCirclesColors(lowColorType, circleNum, shape, brightness);
  }
}

inline auto ShapeColorizer::GetShapesOnlyColors(const LowColorTypes& lowColorType,
                                                const Shape& shape,
                                                const float brightness) const noexcept
    -> ShapeColors
{
  const auto colors = GetShapeColors(GetShapeNumToUse(shape.shapeNum), brightness);
  return {
      .mainColor            = colors.mainColor,
      .lowColor             = GetLowColor(lowColorType, colors),
      .innerMainColor       = colors.innerMainColor,
      .innerLowColor        = GetInnerLowColor(lowColorType, colors),
      .outerCircleMainColor = colors.outerCircleMainColor,
      .outerCircleLowColor  = colors.outerCircleLowColor,
  };
}

inline auto ShapeColorizer::GetCirclesOnlyColors(const LowColorTypes& lowColorType,
                                                 const uint32_t circleNum,
                                                 const float brightness) const noexcept
    -> ShapeColors
{
  const auto colors = GetCircleColors(circleNum, brightness);
  return {
      .mainColor            = colors.mainColor,
      .lowColor             = GetLowColor(lowColorType, colors),
      .innerMainColor       = colors.innerMainColor,
      .innerLowColor        = GetInnerLowColor(lowColorType, colors),
      .outerCircleMainColor = colors.outerCircleMainColor,
      .outerCircleLowColor  = colors.outerCircleLowColor,
  };
}

inline auto ShapeColorizer::GetShapesAndCirclesColors(const LowColorTypes& lowColorType,
                                                      const uint32_t circleNum,
                                                      const Shape& shape,
                                                      const float brightness) const noexcept
    -> ShapeColors
{
  const auto shapeColors  = GetShapeColors(GetShapeNumToUse(shape.shapeNum), brightness);
  const auto circleColors = GetCircleColors(circleNum, brightness);

  return {
      .mainColor = ColorMaps::GetColorMix(shapeColors.mainColor, circleColors.mainColor, m_mixT()),
      .lowColor  = GetLowMixedColor(lowColorType, shapeColors, circleColors, m_mixT()),
      .innerMainColor =
          ColorMaps::GetColorMix(shapeColors.innerMainColor, circleColors.innerMainColor, m_mixT()),
      .innerLowColor = GetInnerLowMixedColor(lowColorType, shapeColors, circleColors, m_mixT()),
      .outerCircleMainColor = ColorMaps::GetColorMix(
          shapeColors.outerCircleMainColor, circleColors.outerCircleMainColor, m_mixT()),
      .outerCircleLowColor = ColorMaps::GetColorMix(
          shapeColors.outerCircleLowColor, circleColors.outerCircleLowColor, m_mixT()),
  };
}

inline auto ShapeColorizer::GetLowColor(const LowColorTypes colorType,
                                        const ShapeColors& colors) noexcept -> Pixel
{
  if (colorType == LowColorTypes::LIGHTENED_LOW_COLOR)
  {
    return GetLightenedColor(colors.mainColor, LIGHTER_COLOR_POWER);
  }
  if (colorType == LowColorTypes::MAIN_COLOR)
  {
    return colors.mainColor;
  }
  return colors.lowColor;
}

inline auto ShapeColorizer::GetInnerLowColor(const LowColorTypes colorType,
                                             const ShapeColors& colors) noexcept -> Pixel
{
  if (colorType == LowColorTypes::LIGHTENED_LOW_COLOR)
  {
    return GetLightenedColor(colors.innerLowColor, LIGHTER_COLOR_POWER);
  }
  if (colorType == LowColorTypes::MAIN_COLOR)
  {
    return colors.innerMainColor;
  }
  return colors.innerLowColor;
}

inline auto ShapeColorizer::GetLowMixedColor(const LowColorTypes colorType,
                                             const ShapeColors& colors1,
                                             const ShapeColors& colors2,
                                             const float mixT) noexcept -> Pixel
{
  if (colorType == LowColorTypes::LIGHTENED_LOW_COLOR)
  {
    const auto mixedColor = ColorMaps::GetColorMix(colors1.lowColor, colors2.lowColor, mixT);
    return GetLightenedColor(mixedColor, LIGHTER_COLOR_POWER);
  }
  if (colorType == LowColorTypes::MAIN_COLOR)
  {
    const auto mixedColor = ColorMaps::GetColorMix(colors1.mainColor, colors2.mainColor, mixT);
    return mixedColor;
  }
  const auto mixedColor = ColorMaps::GetColorMix(colors1.lowColor, colors2.lowColor, mixT);
  return mixedColor;
}

inline auto ShapeColorizer::GetInnerLowMixedColor(const LowColorTypes colorType,
                                                  const ShapeColors& colors1,
                                                  const ShapeColors& colors2,
                                                  const float mixT) noexcept -> Pixel
{
  if (colorType == LowColorTypes::LIGHTENED_LOW_COLOR)
  {
    const auto mixedColor =
        ColorMaps::GetColorMix(colors1.innerLowColor, colors2.innerLowColor, mixT);
    return GetLightenedColor(mixedColor, LIGHTER_COLOR_POWER);
  }
  if (colorType == LowColorTypes::MAIN_COLOR)
  {
    const auto mixedColor =
        ColorMaps::GetColorMix(colors1.innerMainColor, colors2.innerMainColor, mixT);
    return mixedColor;
  }
  const auto mixedColor =
      ColorMaps::GetColorMix(colors1.innerLowColor, colors2.innerLowColor, mixT);
  return mixedColor;
}

inline auto ShapeColorizer::GetShapeNumToUse(const uint32_t shapeNum) const noexcept -> uint32_t
{
  return (m_colorMapMixMode == ColorMapMixMode::STRIPED_SHAPES_ONLY) or
                 (m_colorMapMixMode == ColorMapMixMode::STRIPED_SHAPES_AND_CIRCLES)
             ? (shapeNum / m_stripeWidth)
             : shapeNum;
}

inline auto ShapeColorizer::GetShapeColors(const uint32_t shapeNum,
                                           const float brightness) const noexcept -> ShapeColors
{
  return GetColors(
      m_shapeColorMaps.at(shapeNum), m_shapeColorsT, m_oldShapeColors.at(shapeNum), brightness);
}

inline auto ShapeColorizer::GetCircleColors(const uint32_t circleNum,
                                            const float brightness) const noexcept -> ShapeColors
{
  return GetColors(m_circleColorMaps.at(circleNum),
                   m_circleColorsT,
                   m_oldCircleColors.at(circleNum),
                   brightness);
}

auto ShapeColorizer::GetColors(const ShapeColorMaps& shapeColorMaps,
                               const TValue& t,
                               const ShapeColors& oldShapeColors,
                               const float brightness) const noexcept -> ShapeColors
{
  const auto mainColor =
      GetFinalColor(oldShapeColors.mainColor, shapeColorMaps.mainColorMap.GetColor(t()));
  const auto lowColor =
      GetFinalColor(oldShapeColors.lowColor, shapeColorMaps.lowColorMap.GetColor(t()));
  const auto innerMainColor =
      GetFinalColor(oldShapeColors.innerMainColor, shapeColorMaps.innerMainColorMap.GetColor(t()));
  const auto innerLowColor =
      GetFinalColor(oldShapeColors.innerLowColor, shapeColorMaps.innerLowColorMap.GetColor(t()));
  const auto outerCircleMainColor = GetFinalColor(
      oldShapeColors.outerCircleMainColor, m_outerCircleMainColorMap.GetColor(m_outerCircleT()));
  const auto outerCircleLowColor = GetFinalColor(
      oldShapeColors.outerCircleLowColor, m_outerCircleLowColorMap.GetColor(m_outerCircleT()));

  return {
      .mainColor      = m_colorAdjust.GetAdjustment(brightness, mainColor),
      .lowColor       = m_colorAdjust.GetAdjustment(brightness, lowColor),
      .innerMainColor = m_colorAdjust.GetAdjustment(brightness, innerMainColor),
      .innerLowColor  = m_colorAdjust.GetAdjustment(brightness, innerLowColor),
      .outerCircleMainColor =
          m_colorAdjust.GetAdjustment(OUTER_CIRCLE_BRIGHTNESS * brightness, outerCircleMainColor),
      .outerCircleLowColor =
          m_colorAdjust.GetAdjustment(OUTER_CIRCLE_BRIGHTNESS * brightness, outerCircleLowColor),
  };
}

inline auto ShapeColorizer::GetFinalColor(const Pixel& oldColor,
                                          const Pixel& color) const noexcept -> Pixel
{
  return ColorMaps::GetColorMix(oldColor, color, m_oldT());
}

BrightnessAttenuation::BrightnessAttenuation(const Properties& properties) noexcept
  : m_cutoffBrightness{properties.cutoffBrightness},
    m_maxRSquared{2 * Sq(U_HALF * std::min(properties.screenWidth, properties.screenHeight))}
{
}

auto BrightnessAttenuation::GetPositionBrightness(
    const Point2dInt& pos, const float minBrightnessPastCutoff) const noexcept -> float
{
  const auto distFromCentre = GetDistFromCentreFactor(pos);
  return distFromCentre < DIST_SQ_CUTOFF ? m_cutoffBrightness
                                         : (minBrightnessPastCutoff + distFromCentre);
}

inline auto BrightnessAttenuation::GetDistFromCentreFactor(const Point2dInt& pos) const noexcept
    -> float
{
  return static_cast<float>(Sq(pos.x) + Sq(pos.y)) / static_cast<float>(m_maxRSquared);
}

} // namespace GOOM::VISUAL_FX::TUBES
