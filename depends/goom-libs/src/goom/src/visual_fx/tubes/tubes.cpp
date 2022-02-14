#include "tubes.h"

//#undef NO_LOGGING

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "utils/goom_rand_base.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "color/random_colormaps.h"
#include "goom/logging.h"
#include "point2d.h"
#include "utils/mathutils.h"
#include "utils/paths.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <array>
#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace GOOM::VISUAL_FX::TUBES
{

using COLOR::GammaCorrection;
using COLOR::GetBrighterColor;
using COLOR::GetIncreasedChroma;
using COLOR::GetLightenedColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using UTILS::GetHalf;
using UTILS::IGoomRand;
using UTILS::LinearTimePath;
using UTILS::Logging;
using UTILS::m_pi;
using UTILS::m_third_pi;
using UTILS::m_two_pi;
using UTILS::OscillatingPath;
using UTILS::PathParams;
using UTILS::SMALL_FLOAT;
using UTILS::Sq;
using UTILS::Timer;
using UTILS::TValue;
using UTILS::Weights;
using UTILS::GRAPHICS::SmallImageBitmaps;

constexpr uint32_t NUM_SHAPES_PER_TUBE = 45;
// Strangely, 'NUM_SHAPES_PER_TUBE = 100' gives a small gap in
// circle at 90 and 270 degrees.

constexpr bool OSCILLATING_SHAPE_PATHS = true;

// TODO: Probability and random should be higher up???????
constexpr float PROB_INTERIOR_SHAPE = 45.0F / 50.0F;
constexpr uint32_t MAX_INTERIOR_SHAPES_TIME = 500;
constexpr float PROB_NO_BOUNDARY_SHAPES = 10.0F / 50.0F;
constexpr uint32_t MAX_NO_BOUNDARY_SHAPES_TIME = 1;
constexpr float PROB_HEX_DOT_SHAPE = 1.0F / 50.0F;
constexpr uint32_t MAX_HEX_DOT_SHAPES_TIME = 100;

constexpr float PROB_INCREASED_CHROMA = 0.8F;

constexpr uint32_t MIN_STRIPE_WIDTH = NUM_SHAPES_PER_TUBE / 6;
constexpr uint32_t MAX_STRIPE_WIDTH = NUM_SHAPES_PER_TUBE / 3;
static_assert(MIN_STRIPE_WIDTH > 0, "MIN_STRIPE_WIDTH must be > 0.");

constexpr uint32_t MIN_NUM_CIRCLES_IN_GROUP = 10;
constexpr uint32_t MAX_NUM_CIRCLES_IN_GROUP = 100;

constexpr float MIN_HEX_SIZE = 3.0F;
constexpr float MAX_HEX_SIZE = 9.0F;

constexpr float MIN_CIRCLE_SPEED = 0.0005F;
constexpr float NML_CIRCLE_SPEED = 0.005F;
constexpr float MAX_CIRCLE_SPEED = 0.008F;

constexpr float MIN_CENTRE_SPEED = 0.0005F;
constexpr float NML_CENTRE_SPEED = 0.005F;
constexpr float MAX_CENTRE_SPEED = 0.05F;

enum class LowColorTypes
{
  TRUE_LOW_COLOR,
  MAIN_COLOR,
  LIGHTENED_LOW_COLOR,
  _NUM // unused and must be last
};
constexpr uint32_t MIN_LOW_COLOR_TYPE_TIME = 100;
constexpr uint32_t MAX_LOW_COLOR_TYPE_TIME = 1000;

constexpr float OUTER_CIRCLE_BRIGHTNESS = 0.4F;
constexpr float LIGHTER_COLOR_POWER = 10.0F;

class ShapeColorizer;

class ParametricPath : public LinearTimePath
{
public:
  explicit ParametricPath(const TValue& t) noexcept;

  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;

private:
  static constexpr float DEFAULT_B = 350.0F;
  float m_b = DEFAULT_B;
  static constexpr float DEFAULT_K_X = 3.0F;
  float m_kX = DEFAULT_K_X;
  static constexpr float DEFAULT_K_Y = 3.0F;
  float m_kY = DEFAULT_K_Y;
};

ParametricPath::ParametricPath(const TValue& t) noexcept
  : LinearTimePath{
        Point2dInt{0, 0},
        Point2dInt{0, 0},
        t
}
{
}

auto ParametricPath::GetNextPoint() const -> Point2dInt
{
  const Point2dInt point{
      static_cast<int32_t>(std::round((m_b * std::cos(m_kX * GetT())) * std::cos(GetT()))),
      static_cast<int32_t>(std::round((m_b * std::cos(m_kY * GetT())) * std::sin(GetT()))),
  };

  return point;
}

struct Shape
{
  uint32_t shapeNum{};
  std::unique_ptr<OscillatingPath> path{};
  uint8_t lineThickness{1};
};

class Tube::TubeImpl
{
public:
  TubeImpl() noexcept = delete;
  TubeImpl(uint32_t tubeId,
           DrawFuncs drawFuncs,
           uint32_t screenWidth,
           uint32_t screenHeight,
           const IGoomRand& goomRand,
           std::shared_ptr<RandomColorMaps> colorMaps,
           std::shared_ptr<RandomColorMaps> lowColorMaps,
           float radiusEdgeOffset,
           float brightnessFactor) noexcept;

  [[nodiscard]] auto GetTubeId() const -> uint32_t;
  [[nodiscard]] auto IsActive() const -> bool;

  void SetWeightedColorMaps(std::shared_ptr<RandomColorMaps> colorMaps);
  void SetWeightedLowColorMaps(std::shared_ptr<RandomColorMaps> lowColorMaps);

  void ResetColorMaps();
  void RotateShapeColorMaps();

  [[nodiscard]] auto GetBrightnessFactor() const -> float;
  void SetBrightnessFactor(float val);

  [[nodiscard]] auto GetMaxJitterOffset() const -> int32_t;
  void SetMaxJitterOffset(int32_t val);

  void SetTransformCentreFunc(const TransformCentreFunc& func);
  [[nodiscard]] auto GetCentrePathT() const -> float;
  void SetCentrePathT(float val);
  [[nodiscard]] auto GetCentreSpeed() const -> float;
  void SetCentreSpeed(float val);
  void IncreaseCentreSpeed();
  void DecreaseCentreSpeed();

  void SetAllowOscillatingCirclePaths(bool val);
  void SetCirclePathParams(const PathParams& params);
  [[nodiscard]] auto GetCircleSpeed() const -> float;
  void SetCircleSpeed(float val);
  void IncreaseCircleSpeed();
  void DecreaseCircleSpeed();

  void DrawShapes();
  void UpdateTValues();
  void UpdateTimers();

private:
  const uint32_t m_tubeId;
  const DrawFuncs m_drawFuncs;
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  const IGoomRand& m_goomRand;
  const std::unique_ptr<ShapeColorizer> m_colorizer;
  bool m_active = true;
  static constexpr float PATH_STEP = NML_CIRCLE_SPEED;
  static constexpr uint32_t SHAPE_T_DELAY_TIME = 10;
  static constexpr float T_AT_CENTRE = 0.5F;
  // clang-format off
  TValue m_shapeT{
      TValue::StepType::CONTINUOUS_REVERSIBLE,
      PATH_STEP,
            {
               {0.0F,        SHAPE_T_DELAY_TIME},
               {T_AT_CENTRE, SHAPE_T_DELAY_TIME},
               {1.0F,        SHAPE_T_DELAY_TIME}
            }
  };
  // clang-format on
  std::vector<Shape> m_shapes{};
  int32_t m_maxJitterOffset{};
  TValue m_centrePathT{TValue::StepType::CONTINUOUS_REVERSIBLE, NML_CENTRE_SPEED};
  std::unique_ptr<ParametricPath> m_centrePath{};
  TransformCentreFunc m_getTransformedCentre{};

  Timer m_circleGroupTimer;
  Timer m_interiorShapeTimer{MAX_INTERIOR_SHAPES_TIME};
  Timer m_noBoundaryShapeTimer{MAX_NO_BOUNDARY_SHAPES_TIME};
  Timer m_hexDotShapeTimer{MAX_HEX_DOT_SHAPES_TIME, true};
  float m_hexLen = MIN_HEX_SIZE;
  [[nodiscard]] auto GetHexLen() const -> float;
  uint32_t m_interiorShapeSize;
  [[nodiscard]] auto GetInteriorShapeSize(float hexLen) -> uint32_t;

  Timer m_lowColorTypeTimer{MAX_LOW_COLOR_TYPE_TIME};
  LowColorTypes m_currentLowColorType = LowColorTypes::TRUE_LOW_COLOR;
  const Weights<LowColorTypes> m_lowColorTypes;

  void InitShapes(float radiusEdgeOffset);
  void DrawShape(const Shape& shape, const Vec2dInt& centreOffset) const;
  void DrawInteriorShape(const Point2dInt& shapeCentrePos, const ShapeColors& allColors) const;
  void DrawHexOutline(const Point2dInt& hexCentre,
                      const ShapeColors& allColors,
                      uint8_t lineThickness) const;
  void DrawOuterCircle(const Point2dInt& shapeCentrePos, const ShapeColors& allColors) const;
};

const float Tube::NORMAL_CENTRE_SPEED = NML_CENTRE_SPEED;
const float Tube::NORMAL_CIRCLE_SPEED = NML_CIRCLE_SPEED;

Tube::Tube(const uint32_t tubeId,
           const DrawFuncs& drawFuncs,
           const uint32_t screenWidth,
           const uint32_t screenHeight,
           const IGoomRand& goomRand,
           const std::shared_ptr<RandomColorMaps> colorMaps,
           const std::shared_ptr<RandomColorMaps> lowColorMaps,
           const float radiusEdgeOffset,
           float brightnessFactor) noexcept
  : m_impl{std::make_unique<Tube::TubeImpl>(tubeId,
                                            drawFuncs,
                                            screenWidth,
                                            screenHeight,
                                            goomRand,
                                            colorMaps,
                                            lowColorMaps,
                                            radiusEdgeOffset,
                                            brightnessFactor)}
{
}

void Tube::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> colorMaps)
{
  m_impl->SetWeightedColorMaps(colorMaps);
}

void Tube::SetWeightedLowColorMaps(const std::shared_ptr<RandomColorMaps> lowColorMaps)
{
  m_impl->SetWeightedLowColorMaps(lowColorMaps);
}

void Tube::ResetColorMaps()
{
  m_impl->ResetColorMaps();
}

auto Tube::IsActive() const -> bool
{
  return m_impl->IsActive();
}

void Tube::SetTransformCentreFunc(const TransformCentreFunc& func)
{
  m_impl->SetTransformCentreFunc(func);
}

void Tube::SetCentrePathT(const float val)
{
  m_impl->SetCentrePathT(val);
}

void Tube::SetCentreSpeed(const float val)
{
  m_impl->SetCentreSpeed(val);
}

void Tube::IncreaseCentreSpeed()
{
  m_impl->IncreaseCentreSpeed();
}

void Tube::DecreaseCentreSpeed()
{
  m_impl->DecreaseCentreSpeed();
}

void Tube::SetAllowOscillatingCirclePaths(const bool val)
{
  m_impl->SetAllowOscillatingCirclePaths(val);
}

void Tube::SetCirclePathParams(const PathParams& params)
{
  m_impl->SetCirclePathParams(params);
}

void Tube::SetCircleSpeed(const float val)
{
  m_impl->SetCircleSpeed(val);
}

void Tube::IncreaseCircleSpeed()
{
  m_impl->IncreaseCircleSpeed();
}

void Tube::DecreaseCircleSpeed()
{
  m_impl->DecreaseCircleSpeed();
}

void Tube::DrawCircleOfShapes()
{
  m_impl->DrawShapes();
}

void Tube::SetBrightnessFactor(const float val)
{
  m_impl->SetBrightnessFactor(val);
}

void Tube::SetMaxJitterOffset(const int32_t val)
{
  m_impl->SetMaxJitterOffset(val);
}

class ShapeColorizer
{
public:
  struct ShapeColorMaps
  {
    const IColorMap* colorMap{};
    const IColorMap* lowColorMap{};
    const IColorMap* innerColorMap{};
    const IColorMap* innerLowColorMap{};
  };

  ShapeColorizer() noexcept = delete;
  ShapeColorizer(uint32_t screenWidth,
                 uint32_t screenHeight,
                 uint32_t numShapes,
                 uint32_t numCircles,
                 const IGoomRand& goomRand,
                 std::shared_ptr<RandomColorMaps> colorMaps,
                 std::shared_ptr<RandomColorMaps> innerColorMaps,
                 float brightnessFactor);

  [[nodiscard]] auto GetBrightnessFactor() const -> float;
  void SetBrightnessFactor(float val);

  void SetWeightedColorMaps(std::shared_ptr<RandomColorMaps> colorMaps);
  void SetWeightedLowColorMaps(std::shared_ptr<RandomColorMaps> lowColorMaps);

  void ResetColorMaps();
  void RotateShapeColorMaps();
  [[nodiscard]] auto GetColors(LowColorTypes lowColorType,
                               uint32_t circleNum,
                               const Shape& shape,
                               const Point2dInt& shapeCentrePos) -> ShapeColors;
  void UpdateAllTValues();

private:
  const IGoomRand& m_goomRand;
  std::shared_ptr<RandomColorMaps> m_randomColorMaps;
  std::shared_ptr<RandomColorMaps> m_randomInnerColorMaps;
  float m_brightnessFactor;

  static constexpr float GAMMA = 1.0F / 2.0F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
  [[nodiscard]] auto GetGammaCorrection(float brightness, const Pixel& color) const -> Pixel;

  std::vector<ShapeColorMaps> m_shapeColorMaps;
  std::vector<ShapeColors> m_oldShapeColors;
  static constexpr uint32_t NUM_SHAPE_COLOR_STEPS = 1000;
  TValue m_shapeColorsT{TValue::StepType::CONTINUOUS_REPEATABLE, NUM_SHAPE_COLOR_STEPS};

  std::vector<ShapeColorMaps> m_circleColorMaps;
  std::vector<ShapeColors> m_oldCircleColors;
  TValue m_circleColorsT{TValue::StepType::CONTINUOUS_REPEATABLE, MAX_NUM_CIRCLES_IN_GROUP};

  std::reference_wrapper<const IColorMap> m_outerCircleColorMap;
  std::reference_wrapper<const IColorMap> m_outerCircleLowColorMap;
  static constexpr uint32_t NUM_OUTER_CIRCLE_COLOR_STEPS = 100;
  TValue m_outerCircleT{TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_OUTER_CIRCLE_COLOR_STEPS};

  ColorMapMixMode m_colorMapMixMode = ColorMapMixMode::CIRCLES_ONLY;
  static constexpr uint32_t NUM_MIX_COLOR_STEPS = 1000;
  TValue m_mixT{TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_MIX_COLOR_STEPS};
  const Weights<ColorMapMixMode> m_colorMapMixModes;

  static constexpr uint32_t NUM_STEPS_FROM_OLD = 50;
  TValue m_oldT{TValue::StepType::SINGLE_CYCLE, NUM_STEPS_FROM_OLD};

  void InitColorMaps();
  void ResetColorMixMode();
  void ResetColorMapsLists();
  void ResetColorMapsList(std::vector<ShapeColorMaps>* colorMaps,
                          std::vector<ShapeColors>* oldColors,
                          TValue* t);
  void ResetColorMaps(ShapeColorMaps* colorMaps) const;
  void CopyColors(const ShapeColorMaps& colorMaps, const TValue& t, ShapeColors* oldColors) const;

  uint32_t m_stripeWidth = MIN_STRIPE_WIDTH;
  [[nodiscard]] auto GetShapeNumToUse(uint32_t shapeNum) const -> uint32_t;
  [[nodiscard]] auto GetBrightness(const Shape& shape, const Point2dInt& shapeCentrePos) const
      -> float;

  [[nodiscard]] auto GetShapeColors(uint32_t shapeNum, float brightness) const -> ShapeColors;
  [[nodiscard]] auto GetCircleColors(uint32_t circleNum, float brightness) const -> ShapeColors;
  [[nodiscard]] auto GetColors(const ShapeColorMaps& shapeColorMaps,
                               const TValue& t,
                               const ShapeColors& oldShapeColors,
                               float brightness) const -> ShapeColors;
  [[nodiscard]] static auto GetLowColor(LowColorTypes colorType, const ShapeColors& colors)
      -> Pixel;
  [[nodiscard]] static auto GetLowMixedColor(LowColorTypes colorType,
                                             const ShapeColors& colors1,
                                             const ShapeColors& colors2,
                                             float mixT) -> Pixel;
  [[nodiscard]] static auto GetInnerLowColor(LowColorTypes colorType, const ShapeColors& colors)
      -> Pixel;
  [[nodiscard]] static auto GetInnerLowMixedColor(LowColorTypes colorType,
                                                  const ShapeColors& colors1,
                                                  const ShapeColors& colors2,
                                                  float mixT) -> Pixel;
  bool m_useIncreasedChroma = true;
  static constexpr float CUTOFF_BRIGHTNESS = 0.005F;
  const BrightnessAttenuation m_brightnessAttenuation;
  [[nodiscard]] auto GetFinalColor(const Pixel& oldColor, const Pixel& color) const -> Pixel;
  [[nodiscard]] auto GetShapesOnlyColors(const LowColorTypes& lowColorType,
                                         const Shape& shape,
                                         float brightness) const -> ShapeColors;
  [[nodiscard]] auto GetCirclesOnlyColors(const LowColorTypes& lowColorType,
                                          uint32_t circleNum,
                                          float brightness) const -> ShapeColors;
  [[nodiscard]] auto GetShapesAndCirclesColors(const LowColorTypes& lowColorType,
                                               uint32_t circleNum,
                                               const Shape& shape,
                                               float brightness) const -> ShapeColors;
};

// clang-format off
constexpr float TRUE_LOW_COLOR_WEIGHT      = 30.0F;
constexpr float MAIN_COLOR_WEIGHT          = 10.0F;
constexpr float LIGHTENED_LOW_COLOR_WEIGHT = 10.0F;
// clang-format on

Tube::TubeImpl::TubeImpl(const uint32_t tubeId,
                         DrawFuncs drawFuncs,
                         const uint32_t screenWidth,
                         const uint32_t screenHeight,
                         const IGoomRand& goomRand,
                         const std::shared_ptr<RandomColorMaps> colorMaps,
                         const std::shared_ptr<RandomColorMaps> lowColorMaps,
                         const float radiusEdgeOffset,
                         float brightnessFactor) noexcept
  : m_tubeId{tubeId},
    m_drawFuncs{std::move(drawFuncs)},
    m_screenWidth{screenWidth},
    m_screenHeight{screenHeight},
    m_goomRand{goomRand},
    m_colorizer{std::make_unique<ShapeColorizer>(screenWidth,
                                                 screenHeight,
                                                 NUM_SHAPES_PER_TUBE,
                                                 MAX_NUM_CIRCLES_IN_GROUP,
                                                 m_goomRand,
                                                 colorMaps,
                                                 lowColorMaps,
                                                 brightnessFactor)},
    m_shapes(NUM_SHAPES_PER_TUBE),
    m_circleGroupTimer{
        m_goomRand.GetRandInRange(MIN_NUM_CIRCLES_IN_GROUP, MAX_NUM_CIRCLES_IN_GROUP)},
    m_interiorShapeSize{GetInteriorShapeSize(m_hexLen)},
    // clang-format off
    m_lowColorTypes{
        m_goomRand,
        {
            {LowColorTypes::TRUE_LOW_COLOR,      TRUE_LOW_COLOR_WEIGHT},
            {LowColorTypes::MAIN_COLOR,          MAIN_COLOR_WEIGHT},
            {LowColorTypes::LIGHTENED_LOW_COLOR, LIGHTENED_LOW_COLOR_WEIGHT},
        }
    }
    // clang-format off
{
  InitShapes(radiusEdgeOffset);
}

void Tube::TubeImpl::InitShapes(const float radiusEdgeOffset)
{
  const Point2dInt middlePos{static_cast<int32_t>(m_screenWidth / 2),
                         static_cast<int32_t>(m_screenHeight / 2)};
  const auto radius =
      (0.5F * static_cast<float>(std::min(m_screenWidth, m_screenHeight))) - radiusEdgeOffset;
  const float angleStep = m_two_pi / static_cast<float>(m_shapes.size());

  m_centrePath = std::make_unique<ParametricPath>(m_centrePathT);

  float angle = 0.0;
  uint32_t shapeNum = 0;
  for (auto& shape : m_shapes)
  {
    const float cosAngle = std::cos(angle);
    const float sinAngle = std::sin(angle);
    const float xFrom = radius * cosAngle;
    const float yFrom = radius * sinAngle;
    const Point2dInt fromPos = middlePos + Vec2dInt{static_cast<int32_t>(std::round(xFrom)),
                                              static_cast<int32_t>(std::round(yFrom))};
    const float xTo = radius * std::cos(m_pi + angle);
    const float yTo = radius * std::sin(m_pi + angle);
    const Point2dInt toPos = middlePos + Vec2dInt{static_cast<int32_t>(std::round(xTo)),
                                            static_cast<int32_t>(std::round(yTo))};

    shape.shapeNum = shapeNum;
    shape.path = std::make_unique<OscillatingPath>(fromPos,
                                                   toPos,
                                                   m_shapeT,
                                                   OSCILLATING_SHAPE_PATHS);

    angle += angleStep;
    ++shapeNum;
  }
}

void Tube::TubeImpl::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> colorMaps)
{
  m_colorizer->SetWeightedColorMaps(colorMaps);
}

void Tube::TubeImpl::SetWeightedLowColorMaps(const std::shared_ptr<RandomColorMaps> lowColorMaps)
{
  m_colorizer->SetWeightedLowColorMaps(lowColorMaps);
}

void Tube::TubeImpl::ResetColorMaps()
{
  m_colorizer->ResetColorMaps();
  m_circleGroupTimer.SetTimeLimit(
      m_goomRand.GetRandInRange(MIN_NUM_CIRCLES_IN_GROUP, MAX_NUM_CIRCLES_IN_GROUP));
}

inline void Tube::TubeImpl::RotateShapeColorMaps()
{
  m_colorizer->RotateShapeColorMaps();
}

inline auto Tube::TubeImpl::GetTubeId() const -> uint32_t
{
  return m_tubeId;
}

inline auto Tube::TubeImpl::IsActive() const -> bool
{
  return m_active;
}

inline auto Tube::TubeImpl::GetBrightnessFactor() const -> float
{
  return m_colorizer->GetBrightnessFactor();
}

inline void Tube::TubeImpl::SetBrightnessFactor(const float val)
{
  m_colorizer->SetBrightnessFactor(val);
}

inline auto Tube::TubeImpl::GetMaxJitterOffset() const -> int32_t
{
  return m_maxJitterOffset;
}

inline void Tube::TubeImpl::SetMaxJitterOffset(const int32_t val)
{
  m_maxJitterOffset = val;
}

inline void Tube::TubeImpl::SetTransformCentreFunc(const TransformCentreFunc& func)
{
  m_getTransformedCentre = func;
}

inline auto Tube::TubeImpl::GetCentrePathT() const -> float
{
  return m_centrePath->GetT();
}

inline void Tube::TubeImpl::SetCentrePathT(const float val)
{
  m_centrePathT.Reset(val);
}

inline auto Tube::TubeImpl::GetCentreSpeed() const -> float
{
  return m_centrePathT.GetStepSize();
}

inline void Tube::TubeImpl::SetCentreSpeed(const float val)
{
  m_centrePathT.SetStepSize(val);
}

inline void Tube::TubeImpl::IncreaseCentreSpeed()
{
  const float factor = m_goomRand.GetRandInRange(1.01F, 10.0F);
  const float newSpeed = std::min(MAX_CENTRE_SPEED, m_centrePath->GetStepSize() * factor);
  m_centrePathT.SetStepSize(newSpeed);
}

inline void Tube::TubeImpl::DecreaseCentreSpeed()
{
  const float factor = m_goomRand.GetRandInRange(0.1F, 0.99F);
  const float newSpeed = std::min(MIN_CENTRE_SPEED, m_centrePath->GetStepSize() * factor);
  m_centrePathT.SetStepSize(newSpeed);
}

inline void Tube::TubeImpl::SetAllowOscillatingCirclePaths(const bool val)
{
  for (const auto& shape : m_shapes)
  {
    shape.path->SetAllowOscillatingPath(val);
  }
}

inline void Tube::TubeImpl::SetCirclePathParams(const PathParams& params)
{
  for (const auto& shape : m_shapes)
  {
    shape.path->SetPathParams(params);
  }
}

inline auto Tube::TubeImpl::GetCircleSpeed() const -> float
{
  return m_shapeT.GetStepSize();
}

inline void Tube::TubeImpl::SetCircleSpeed(const float val)
{
  m_shapeT.SetStepSize(val);
}

inline void Tube::TubeImpl::IncreaseCircleSpeed()
{
  constexpr float MIN_INCREASE_SPEED_FACTOR = 1.01F;
  constexpr float MAX_INCREASE_SPEED_FACTOR = 10.0F;
  const float factor =
      m_goomRand.GetRandInRange(MIN_INCREASE_SPEED_FACTOR, MAX_INCREASE_SPEED_FACTOR);

  const float newSpeed = std::min(MAX_CIRCLE_SPEED, m_shapeT.GetStepSize() * factor);
  m_shapeT.SetStepSize(newSpeed);
}

inline void Tube::TubeImpl::DecreaseCircleSpeed()
{
  constexpr float MIN_DECREASE_SPEED_FACTOR = 0.1F;
  constexpr float MAX_DECREASE_SPEED_FACTOR = 0.99F;
  const float factor =
      m_goomRand.GetRandInRange(MIN_DECREASE_SPEED_FACTOR, MAX_DECREASE_SPEED_FACTOR);

  const float newSpeed = std::max(MIN_CIRCLE_SPEED, m_shapeT.GetStepSize() * factor);
  m_shapeT.SetStepSize(newSpeed);
}

void Tube::TubeImpl::DrawShapes()
{
  m_hexLen = GetHexLen();
  m_interiorShapeSize = GetInteriorShapeSize(m_hexLen);

  const Vec2dInt centreOffset = m_getTransformedCentre(m_tubeId, m_centrePath->GetNextPoint());
  for (const auto& shape : m_shapes)
  {
    DrawShape(shape, centreOffset);
  }

  UpdateTValues();
  UpdateTimers();
}

inline auto Tube::TubeImpl::GetHexLen() const -> float
{
  const float hexSizeT = std::fabs(m_shapes[0].path->GetT() - T_AT_CENTRE) / T_AT_CENTRE;
  return stdnew::lerp(MIN_HEX_SIZE, MAX_HEX_SIZE, hexSizeT);
}

inline void Tube::TubeImpl::UpdateTValues()
{
  m_shapeT.Increment();
  m_centrePathT.Increment();
  m_colorizer->UpdateAllTValues();
}

inline void Tube::TubeImpl::UpdateTimers()
{
  m_circleGroupTimer.Increment();
  if (m_circleGroupTimer.Finished())
  {
    m_circleGroupTimer.ResetToZero();
  }

  m_interiorShapeTimer.Increment();
  if (m_interiorShapeTimer.Finished() && m_goomRand.ProbabilityOf(PROB_INTERIOR_SHAPE))
  {
    m_interiorShapeTimer.ResetToZero();
  }

  m_noBoundaryShapeTimer.Increment();
  if (m_noBoundaryShapeTimer.Finished() && m_goomRand.ProbabilityOf(PROB_NO_BOUNDARY_SHAPES))
  {
    m_noBoundaryShapeTimer.ResetToZero();
  }

  m_hexDotShapeTimer.Increment();
  if (m_hexDotShapeTimer.Finished() && m_goomRand.ProbabilityOf(PROB_HEX_DOT_SHAPE))
  {
    m_hexDotShapeTimer.ResetToZero();
  }

  m_lowColorTypeTimer.Increment();
  if (m_lowColorTypeTimer.Finished())
  {
    m_currentLowColorType = m_lowColorTypes.GetRandomWeighted();
    m_lowColorTypeTimer.SetTimeLimit(
        m_goomRand.GetRandInRange(MIN_LOW_COLOR_TYPE_TIME, MAX_LOW_COLOR_TYPE_TIME + 1));
  }
}

inline auto Tube::TubeImpl::GetInteriorShapeSize(const float hexLen) -> uint32_t
{
  constexpr float MIN_SIZE_FACTOR = 0.5F;
  constexpr float MAX_SIZE_FACTOR = 1.3F;
  return static_cast<uint32_t>(
      std::round(m_goomRand.GetRandInRange(MIN_SIZE_FACTOR, MAX_SIZE_FACTOR) * hexLen));
}

void Tube::TubeImpl::DrawShape(const Shape& shape, const Vec2dInt& centreOffset) const
{
  const int32_t jitterXOffset = m_goomRand.GetRandInRange(0, m_maxJitterOffset + 1);
  const int32_t jitterYOffset = jitterXOffset;
  const Vec2dInt jitterOffset{jitterXOffset, jitterYOffset};
  const Point2dInt shapeCentrePos = shape.path->GetNextPoint() + jitterOffset + centreOffset;

  const ShapeColors allColors = m_colorizer->GetColors(
      m_currentLowColorType, static_cast<uint32_t>(m_circleGroupTimer.GetCurrentCount()), shape,
      shapeCentrePos);

  if (m_noBoundaryShapeTimer.Finished())
  {
    DrawHexOutline(shapeCentrePos, allColors, shape.lineThickness);
  }

  constexpr float MIN_HEX_LEN_FOR_INTERIOR = 2.0;
  if ((!m_interiorShapeTimer.Finished()) && (m_hexLen > (MIN_HEX_LEN_FOR_INTERIOR + SMALL_FLOAT)))
  {
    DrawInteriorShape(shapeCentrePos, allColors);
    DrawOuterCircle(shapeCentrePos, allColors);
  }
}

void Tube::TubeImpl::DrawHexOutline(const Point2dInt& hexCentre,
                                    const ShapeColors& allColors,
                                    const uint8_t lineThickness) const
{
  constexpr uint32_t NUM_HEX_SIDES = 6;
  constexpr float ANGLE_STEP = GOOM::UTILS::m_third_pi;
  constexpr float START_ANGLE = 2.0F * ANGLE_STEP;
  const std::vector<Pixel> lineColors{allColors.color, allColors.lowColor};
  const std::vector<Pixel> outerCircleColors{allColors.outerCircleColor,
                                             allColors.outerCircleLowColor};
  const bool drawHexDot = !m_hexDotShapeTimer.Finished();

  // Start hex shape to right of centre position.
  auto x0 = static_cast<int32_t>(std::round(static_cast<float>(hexCentre.x) + m_hexLen));
  int32_t y0 = hexCentre.y;
  float angle = START_ANGLE;

  for (uint32_t i = 0; i < NUM_HEX_SIDES; ++i)
  {
    const int32_t x1 = x0 + static_cast<int32_t>(std::round(m_hexLen * std::cos(angle)));
    const int32_t y1 = y0 + static_cast<int32_t>(std::round(m_hexLen * std::sin(angle)));

    m_drawFuncs.drawLine(x0, y0, x1, y1, lineColors, lineThickness);
    if (drawHexDot)
    {
      constexpr uint32_t HEX_DOT_SIZE = 3;
      m_drawFuncs.drawSmallImage(x1, y1, SmallImageBitmaps::ImageNames::SPHERE, HEX_DOT_SIZE,
                                 outerCircleColors);
    }

    angle += ANGLE_STEP;
    x0 = x1;
    y0 = y1;
  }
}

inline void Tube::TubeImpl::DrawInteriorShape(const Point2dInt& shapeCentrePos,
                                              const ShapeColors& allColors) const
{
  const std::vector<Pixel> colors{allColors.innerColor, allColors.innerLowColor};
  m_drawFuncs.drawSmallImage(shapeCentrePos.x, shapeCentrePos.y,
                             SmallImageBitmaps::ImageNames::SPHERE, m_interiorShapeSize, colors);
}

inline void Tube::TubeImpl::DrawOuterCircle(const Point2dInt& shapeCentrePos,
                                            const ShapeColors& allColors) const
{
  constexpr float OUTER_CIRCLE_RADIUS_FACTOR = 1.5;
  const auto outerCircleRadius =
      static_cast<int32_t>(std::round(OUTER_CIRCLE_RADIUS_FACTOR * m_hexLen));
  constexpr uint8_t OUTER_CIRCLE_LINE_THICKNESS = 1;
  const std::vector<Pixel> outerCircleColors{allColors.outerCircleColor,
                                             allColors.outerCircleLowColor};
  m_drawFuncs.drawCircle(shapeCentrePos.x, shapeCentrePos.y, outerCircleRadius, outerCircleColors,
                         OUTER_CIRCLE_LINE_THICKNESS);
}

// clang-format off
constexpr float SHAPES_ONLY_WEIGHT                = 20.0F;
constexpr float STRIPED_SHAPES_ONLY_WEIGHT        = 10.0F;
constexpr float CIRCLES_ONLY_WEIGHT               = 20.0F;
constexpr float SHAPES_AND_CIRCLES_WEIGHT         =  5.0F;
constexpr float STRIPED_SHAPES_AND_CIRCLES_WEIGHT = 15.0F;
// clang-format on

ShapeColorizer::ShapeColorizer(const uint32_t screenWidth,
                               const uint32_t screenHeight,
                               const uint32_t numShapes,
                               const uint32_t numCircles,
                               const IGoomRand& goomRand,
                               const std::shared_ptr<RandomColorMaps> colorMaps,
                               const std::shared_ptr<RandomColorMaps> innerColorMaps,
                               const float brightnessFactor)
  : m_goomRand{goomRand},
    m_randomColorMaps{colorMaps},
    m_randomInnerColorMaps{innerColorMaps},
    m_brightnessFactor{brightnessFactor},
    m_shapeColorMaps(numShapes),
    m_oldShapeColors(numShapes),
    m_circleColorMaps(numCircles),
    m_oldCircleColors(numCircles),
    m_outerCircleColorMap{m_randomInnerColorMaps->GetRandomColorMap()},
    m_outerCircleLowColorMap{m_randomInnerColorMaps->GetRandomColorMap()},
    // clang-format off
    m_colorMapMixModes{
        m_goomRand,
        {
            {ColorMapMixMode::SHAPES_ONLY,                SHAPES_ONLY_WEIGHT},
            {ColorMapMixMode::STRIPED_SHAPES_ONLY,        STRIPED_SHAPES_ONLY_WEIGHT},
            {ColorMapMixMode::CIRCLES_ONLY,               CIRCLES_ONLY_WEIGHT},
            {ColorMapMixMode::SHAPES_AND_CIRCLES,         SHAPES_AND_CIRCLES_WEIGHT},
            {ColorMapMixMode::STRIPED_SHAPES_AND_CIRCLES, STRIPED_SHAPES_AND_CIRCLES_WEIGHT},
        }
    },
    // clang-format on
    m_brightnessAttenuation{screenWidth, screenHeight, CUTOFF_BRIGHTNESS}
{
  InitColorMaps();
  ResetColorMaps();
}

inline auto ShapeColorizer::GetBrightnessFactor() const -> float
{
  return m_brightnessFactor;
}

inline void ShapeColorizer::SetBrightnessFactor(const float val)
{
  m_brightnessFactor = val;
}

void ShapeColorizer::InitColorMaps()
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

void ShapeColorizer::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> colorMaps)
{
  m_randomColorMaps = colorMaps;
}

void ShapeColorizer::SetWeightedLowColorMaps(const std::shared_ptr<RandomColorMaps> lowColorMaps)
{
  m_randomInnerColorMaps = lowColorMaps;
}

void ShapeColorizer::ResetColorMaps()
{
  ResetColorMixMode();
  ResetColorMapsLists();

  m_stripeWidth = m_goomRand.GetRandInRange(MIN_STRIPE_WIDTH, MAX_STRIPE_WIDTH + 1);
  m_useIncreasedChroma = m_goomRand.ProbabilityOf(PROB_INCREASED_CHROMA);
}

inline void ShapeColorizer::ResetColorMapsLists()
{
  ResetColorMapsList(&m_shapeColorMaps, &m_oldShapeColors, &m_shapeColorsT);
  ResetColorMapsList(&m_circleColorMaps, &m_oldCircleColors, &m_circleColorsT);
}

void ShapeColorizer::RotateShapeColorMaps()
{
  std::rotate(begin(m_shapeColorMaps), begin(m_shapeColorMaps) + 1, end(m_shapeColorMaps));
  std::rotate(begin(m_oldShapeColors), begin(m_oldShapeColors) + 1, end(m_oldShapeColors));
}

void ShapeColorizer::ResetColorMapsList(std::vector<ShapeColorMaps>* const colorMaps,
                                        std::vector<ShapeColors>* const oldColors,
                                        TValue* const t)
{
  assert(colorMaps->size() == oldColors->size());

  m_outerCircleColorMap = m_randomInnerColorMaps->GetRandomColorMap();
  m_outerCircleLowColorMap = m_randomInnerColorMaps->GetRandomColorMap();

  for (size_t i = 0; i < colorMaps->size(); ++i)
  {
    CopyColors((*colorMaps)[i], *t, &(*oldColors)[i]);
    ResetColorMaps(&(*colorMaps)[i]);
  }
  t->Reset(0.0);
}

void ShapeColorizer::ResetColorMaps(ShapeColorMaps* const colorMaps) const
{
  colorMaps->colorMap = &m_randomColorMaps->GetRandomColorMap();
  colorMaps->lowColorMap = &m_randomColorMaps->GetRandomColorMap();
  colorMaps->innerColorMap = &m_randomInnerColorMaps->GetRandomColorMap();
  colorMaps->innerLowColorMap = &m_randomInnerColorMaps->GetRandomColorMap();
}

void ShapeColorizer::CopyColors(const ShapeColorMaps& colorMaps,
                                const TValue& t,
                                ShapeColors* const oldColors) const
{
  oldColors->color = colorMaps.colorMap->GetColor(t());
  oldColors->lowColor = colorMaps.lowColorMap->GetColor(t());
  oldColors->innerColor = colorMaps.innerColorMap->GetColor(t());
  oldColors->innerLowColor = colorMaps.innerLowColorMap->GetColor(t());
  oldColors->outerCircleColor = m_outerCircleColorMap.get().GetColor(t());
  oldColors->outerCircleLowColor = m_outerCircleLowColorMap.get().GetColor(t());
}

inline void ShapeColorizer::ResetColorMixMode()
{
  m_colorMapMixMode = m_colorMapMixModes.GetRandomWeighted();
}

void ShapeColorizer::UpdateAllTValues()
{
  m_shapeColorsT.Increment();
  m_circleColorsT.Increment();
  m_outerCircleT.Increment();
  m_oldT.Increment();
  m_mixT.Increment();
}

auto ShapeColorizer::GetBrightness(const Shape& shape, const Point2dInt& shapeCentrePos) const
    -> float
{
  constexpr float MIN_BRIGHTNESS = 0.5F;
  const float brightness =
      std::min(3.0F, m_brightnessFactor * m_brightnessAttenuation.GetPositionBrightness(
                                              shapeCentrePos, MIN_BRIGHTNESS));

  constexpr float SMALL_T = 0.15F;
  if (constexpr float HALFWAY_T = 0.5F; std::fabs(shape.path->GetT() - HALFWAY_T) < SMALL_T)
  {
    constexpr float SMALL_T_BRIGHTNESS = 0.250F;
    return SMALL_T_BRIGHTNESS * brightness;
  }
  return brightness;
}

auto ShapeColorizer::GetColors(const LowColorTypes lowColorType,
                               const uint32_t circleNum,
                               const Shape& shape,
                               const Point2dInt& shapeCentrePos) -> ShapeColors
{
  const float brightness = GetBrightness(shape, shapeCentrePos);

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
    default:
      throw std::logic_error("Invalid ColorMapMixMode.");
  }
}

inline auto ShapeColorizer::GetShapesOnlyColors(const LowColorTypes& lowColorType,
                                                const Shape& shape,
                                                const float brightness) const -> ShapeColors
{
  const ShapeColors colors = GetShapeColors(GetShapeNumToUse(shape.shapeNum), brightness);
  return {
      colors.color,
      GetLowColor(lowColorType, colors),
      colors.innerColor,
      GetInnerLowColor(lowColorType, colors),
      colors.outerCircleColor,
      colors.outerCircleLowColor,
  };
}

inline auto ShapeColorizer::GetCirclesOnlyColors(const LowColorTypes& lowColorType,
                                                 const uint32_t circleNum,
                                                 const float brightness) const -> ShapeColors
{
  const ShapeColors colors = GetCircleColors(circleNum, brightness);
  return {
      colors.color,
      GetLowColor(lowColorType, colors),
      colors.innerColor,
      GetInnerLowColor(lowColorType, colors),
      colors.outerCircleColor,
      colors.outerCircleLowColor,
  };
}

inline auto ShapeColorizer::GetShapesAndCirclesColors(const LowColorTypes& lowColorType,
                                                      const uint32_t circleNum,
                                                      const Shape& shape,
                                                      const float brightness) const -> ShapeColors
{
  const ShapeColors shapeColors = GetShapeColors(GetShapeNumToUse(shape.shapeNum), brightness);
  const ShapeColors circleColors = GetCircleColors(circleNum, brightness);

  return {
      IColorMap::GetColorMix(shapeColors.color, circleColors.color, m_mixT()),
      GetLowMixedColor(lowColorType, shapeColors, circleColors, m_mixT()),
      IColorMap::GetColorMix(shapeColors.innerColor, circleColors.innerColor, m_mixT()),
      GetInnerLowMixedColor(lowColorType, shapeColors, circleColors, m_mixT()),
      IColorMap::GetColorMix(shapeColors.outerCircleColor, circleColors.outerCircleColor, m_mixT()),
      IColorMap::GetColorMix(shapeColors.outerCircleLowColor, circleColors.outerCircleLowColor,
                             m_mixT()),
  };
}

inline auto ShapeColorizer::GetLowColor(const LowColorTypes colorType, const ShapeColors& colors)
    -> Pixel
{
  if (colorType == LowColorTypes::LIGHTENED_LOW_COLOR)
  {
    return GetLightenedColor(colors.color, LIGHTER_COLOR_POWER);
  }
  if (colorType == LowColorTypes::MAIN_COLOR)
  {
    return colors.color;
  }
  return colors.lowColor;
}

inline auto ShapeColorizer::GetInnerLowColor(const LowColorTypes colorType,
                                             const ShapeColors& colors) -> Pixel
{
  if (colorType == LowColorTypes::LIGHTENED_LOW_COLOR)
  {
    return GetLightenedColor(colors.innerLowColor, LIGHTER_COLOR_POWER);
  }
  if (colorType == LowColorTypes::MAIN_COLOR)
  {
    return colors.innerColor;
  }
  return colors.innerLowColor;
}

inline auto ShapeColorizer::GetLowMixedColor(const LowColorTypes colorType,
                                             const ShapeColors& colors1,
                                             const ShapeColors& colors2,
                                             const float mixT) -> Pixel
{
  if (colorType == LowColorTypes::LIGHTENED_LOW_COLOR)
  {
    const Pixel mixedColor = IColorMap::GetColorMix(colors1.lowColor, colors2.lowColor, mixT);
    return GetLightenedColor(mixedColor, LIGHTER_COLOR_POWER);
  }
  if (colorType == LowColorTypes::MAIN_COLOR)
  {
    const Pixel mixedColor = IColorMap::GetColorMix(colors1.color, colors2.color, mixT);
    return mixedColor;
  }
  const Pixel mixedColor = IColorMap::GetColorMix(colors1.lowColor, colors2.lowColor, mixT);
  return mixedColor;
}

inline auto ShapeColorizer::GetInnerLowMixedColor(const LowColorTypes colorType,
                                                  const ShapeColors& colors1,
                                                  const ShapeColors& colors2,
                                                  const float mixT) -> Pixel
{
  if (colorType == LowColorTypes::LIGHTENED_LOW_COLOR)
  {
    const Pixel mixedColor =
        IColorMap::GetColorMix(colors1.innerLowColor, colors2.innerLowColor, mixT);
    return GetLightenedColor(mixedColor, LIGHTER_COLOR_POWER);
  }
  if (colorType == LowColorTypes::MAIN_COLOR)
  {
    const Pixel mixedColor = IColorMap::GetColorMix(colors1.innerColor, colors2.innerColor, mixT);
    return mixedColor;
  }
  const Pixel mixedColor =
      IColorMap::GetColorMix(colors1.innerLowColor, colors2.innerLowColor, mixT);
  return mixedColor;
}

inline auto ShapeColorizer::GetShapeNumToUse(const uint32_t shapeNum) const -> uint32_t
{
  return (m_colorMapMixMode == ColorMapMixMode::STRIPED_SHAPES_ONLY) ||
                 (m_colorMapMixMode == ColorMapMixMode::STRIPED_SHAPES_AND_CIRCLES)
             ? (shapeNum / m_stripeWidth)
             : shapeNum;
}

inline auto ShapeColorizer::GetShapeColors(const uint32_t shapeNum, const float brightness) const
    -> ShapeColors
{
  return GetColors(m_shapeColorMaps[shapeNum], m_shapeColorsT, m_oldShapeColors[shapeNum],
                   brightness);
}

inline auto ShapeColorizer::GetCircleColors(const uint32_t circleNum, const float brightness) const
    -> ShapeColors
{
  return GetColors(m_circleColorMaps[circleNum], m_circleColorsT, m_oldCircleColors[circleNum],
                   brightness);
}

auto ShapeColorizer::GetColors(const ShapeColorMaps& shapeColorMaps,
                               const TValue& t,
                               const ShapeColors& oldShapeColors,
                               const float brightness) const -> ShapeColors
{
  const Pixel color = GetFinalColor(oldShapeColors.color, shapeColorMaps.colorMap->GetColor(t()));
  const Pixel lowColor =
      GetFinalColor(oldShapeColors.lowColor, shapeColorMaps.lowColorMap->GetColor(t()));
  const Pixel innerColor =
      GetFinalColor(oldShapeColors.innerColor, shapeColorMaps.innerColorMap->GetColor(t()));
  const Pixel innerLowColor =
      GetFinalColor(oldShapeColors.innerLowColor, shapeColorMaps.innerLowColorMap->GetColor(t()));
  const Pixel outerCircleColor = GetFinalColor(
      oldShapeColors.outerCircleColor, m_outerCircleColorMap.get().GetColor(m_outerCircleT()));
  const Pixel outerCircleLowColor =
      GetFinalColor(oldShapeColors.outerCircleLowColor,
                    m_outerCircleLowColorMap.get().GetColor(m_outerCircleT()));

  return {
      GetGammaCorrection(brightness, color),
      GetGammaCorrection(brightness, lowColor),
      GetGammaCorrection(brightness, innerColor),
      GetGammaCorrection(brightness, innerLowColor),
      GetGammaCorrection(OUTER_CIRCLE_BRIGHTNESS * brightness, outerCircleColor),
      GetGammaCorrection(OUTER_CIRCLE_BRIGHTNESS * brightness, outerCircleLowColor),
  };
}

inline auto ShapeColorizer::GetFinalColor(const Pixel& oldColor, const Pixel& color) const -> Pixel
{
  const Pixel finalColor = IColorMap::GetColorMix(oldColor, color, m_oldT());
  if (!m_useIncreasedChroma)
  {
    return finalColor;
  }
  return GetIncreasedChroma(finalColor);
}

inline auto ShapeColorizer::GetGammaCorrection(const float brightness, const Pixel& color) const
    -> Pixel
{
  if constexpr (1.0F == GAMMA)
  {
    return GetBrighterColor(brightness, color);
  }
  return m_gammaCorrect.GetCorrection(brightness, color);
}

BrightnessAttenuation::BrightnessAttenuation(const uint32_t screenWidth,
                                             const uint32_t screenHeight,
                                             const float cutoffBrightness)
  : m_cutoffBrightness{cutoffBrightness},
    m_maxRSquared{2 * Sq(GetHalf(std::min(screenWidth, screenHeight)))}

{
}

auto BrightnessAttenuation::GetPositionBrightness(const Point2dInt& pos,
                                                  const float minBrightnessPastCutoff) const
    -> float
{
  const float distFromCentre = GetDistFromCentreFactor(pos);
  return distFromCentre < DIST_SQ_CUTOFF ? m_cutoffBrightness
                                         : (minBrightnessPastCutoff + distFromCentre);
}

inline auto BrightnessAttenuation::GetDistFromCentreFactor(const Point2dInt& pos) const -> float
{
  return static_cast<float>(Sq(pos.x) + Sq(pos.y)) / static_cast<float>(m_maxRSquared);
}

} // namespace GOOM::VISUAL_FX::TUBES
