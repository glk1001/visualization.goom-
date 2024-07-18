module;

#include <cmath>
#include <cstdint>
#include <vector>

module Goom.VisualFx.LSystemFx:LSysGeom;

import Goom.Utils.Math.IncrementedValues;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.AssertUtils;
import Goom.Lib.Point2d;

using GOOM::UTILS::MATH::DEGREES_360;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::IncrementedValue;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::ToRadians;
using GOOM::UTILS::MATH::TValue;

namespace GOOM::VISUAL_FX::L_SYSTEM
{

class LSysGeometry
{
public:
  LSysGeometry(const GoomRand& goomRand, float xScale, float yScale) noexcept;

  auto SetNumLSysCopies(uint32_t numLSysCopies) noexcept -> void;
  auto SetVerticalMoveMaxMin(float verticalMoveMin, float verticalMoveMax) noexcept -> void;
  auto SetVerticalMoveNumSteps(uint32_t numSteps) noexcept -> void;
  auto SetYScaleNumSteps(uint32_t numSteps) noexcept -> void;
  auto SetRotateDegreesAdjustNumSteps(uint32_t numSteps) noexcept -> void;
  auto SetSpinDegreesAdjustNumSteps(uint32_t numSteps) noexcept -> void;
  auto ReverseRotateDirection() noexcept -> void;
  auto ReverseSpinDirection() noexcept -> void;
  auto SetTranslateAdjust(const Vec2dFlt& translateAdjust) noexcept -> void;

  auto UpdateCurrentTransformArray() noexcept -> void;
  [[nodiscard]] auto GetTransformedPoint(const Point2dFlt& point,
                                         uint32_t copyNum) const noexcept -> Point2dFlt;
  auto IncrementTs() noexcept -> void;

  [[nodiscard]] auto GetVerticalMove() const noexcept -> const IncrementedValue<float>&;

private:
  const GoomRand* m_goomRand;
  float m_xScale;
  float m_yScale;
  uint32_t m_numLSysCopies = 1U;

  struct TransformAdjust
  {
    float xScale{};
    float yScale{};
    float rotateDegrees{};
    Vec2dFlt translate;
  };
  struct TransformLSys
  {
    // This is the order the transform operations should occur in.
    float xScale{};
    float yScale{};
    float sinSpinAngle{};
    float cosSpinAngle{};
    float verticalMove{};
    float sinRotateAngle{};
    float cosRotateAngle{};
    Vec2dFlt translate;
  };
  std::vector<TransformAdjust> m_transformAdjustArray;
  [[nodiscard]] auto GetTransformAdjustArray() const noexcept -> std::vector<TransformAdjust>;
  std::vector<TransformLSys> m_currentTransformArray;

  Vec2dFlt m_translateAdjust{};

  static constexpr auto DEFAULT_NUM_ROTATE_DEGREES_STEPS = 100U;
  IncrementedValue<float> m_rotateDegreesAdjust{
      0.0F, DEGREES_360, TValue::StepType::CONTINUOUS_REVERSIBLE, DEFAULT_NUM_ROTATE_DEGREES_STEPS};
  float m_rotateSign = +1.0F;

  static constexpr auto DEFAULT_NUM_SPIN_DEGREES_STEPS = 100U;
  IncrementedValue<float> m_spinDegreesAdjust{
      0.0F, DEGREES_360, TValue::StepType::CONTINUOUS_REVERSIBLE, DEFAULT_NUM_SPIN_DEGREES_STEPS};
  float m_spinSign = -1.0F;

  static constexpr auto MIN_Y_SCALE_ADJUST           = 1.0F;
  static constexpr auto MAX_Y_SCALE_ADJUST           = 1.9F;
  static constexpr auto DEFAULT_Y_SCALE_ADJUST_STEPS = 300U;
  IncrementedValue<float> m_yScaleAdjust{MIN_Y_SCALE_ADJUST,
                                         MAX_Y_SCALE_ADJUST,
                                         TValue::StepType::CONTINUOUS_REVERSIBLE,
                                         DEFAULT_Y_SCALE_ADJUST_STEPS};

  static constexpr auto MIN_VERTICAL_MOVE           = -100.0F;
  static constexpr auto MAX_VERTICAL_MOVE           = +100.0F;
  static constexpr auto DEFAULT_VERTICAL_MOVE_STEPS = 150U;
  IncrementedValue<float> m_verticalMove{MIN_VERTICAL_MOVE,
                                         MAX_VERTICAL_MOVE,
                                         TValue::StepType::CONTINUOUS_REVERSIBLE,
                                         DEFAULT_VERTICAL_MOVE_STEPS};
};

} // namespace GOOM::VISUAL_FX::L_SYSTEM

namespace GOOM::VISUAL_FX::L_SYSTEM
{

inline auto LSysGeometry::ReverseRotateDirection() noexcept -> void
{
  m_rotateSign = -m_rotateSign;
}

inline auto LSysGeometry::ReverseSpinDirection() noexcept -> void
{
  m_spinSign = -m_spinSign;
}

inline auto LSysGeometry::SetTranslateAdjust(const Vec2dFlt& translateAdjust) noexcept -> void
{
  m_translateAdjust = translateAdjust;
}

inline auto LSysGeometry::SetVerticalMoveMaxMin(const float verticalMoveMin,
                                                const float verticalMoveMax) noexcept -> void
{
  m_verticalMove.SetValues(verticalMoveMin, verticalMoveMax);
}

inline auto LSysGeometry::SetRotateDegreesAdjustNumSteps(const uint32_t numSteps) noexcept -> void
{
  m_rotateDegreesAdjust.SetNumSteps(numSteps);
}

inline auto LSysGeometry::SetSpinDegreesAdjustNumSteps(const uint32_t numSteps) noexcept -> void
{
  m_spinDegreesAdjust.SetNumSteps(numSteps);
}

inline auto LSysGeometry::SetVerticalMoveNumSteps(const uint32_t numSteps) noexcept -> void
{
  m_verticalMove.SetNumSteps(numSteps);
}

inline auto LSysGeometry::SetYScaleNumSteps(const uint32_t numSteps) noexcept -> void
{
  m_yScaleAdjust.SetNumSteps(numSteps);
}

inline auto LSysGeometry::GetVerticalMove() const noexcept
    -> const UTILS::MATH::IncrementedValue<float>&
{
  return m_verticalMove;
}

LSysGeometry::LSysGeometry(const UTILS::MATH::GoomRand& goomRand,
                           const float xScale,
                           const float yScale) noexcept
  : m_goomRand{&goomRand}, m_xScale{xScale}, m_yScale{yScale}
{
}

auto LSysGeometry::IncrementTs() noexcept -> void
{
  m_rotateDegreesAdjust.Increment();
  m_spinDegreesAdjust.Increment();
  m_yScaleAdjust.Increment();
  m_verticalMove.Increment();
}

auto LSysGeometry::SetNumLSysCopies(const uint32_t numLSysCopies) noexcept -> void
{
  Expects(numLSysCopies > 0U);
  m_numLSysCopies        = numLSysCopies;
  m_transformAdjustArray = GetTransformAdjustArray();
  UpdateCurrentTransformArray();
}

auto LSysGeometry::GetTransformAdjustArray() const noexcept -> std::vector<TransformAdjust>
{
  auto transformAdjustArray = std::vector<TransformAdjust>(m_numLSysCopies);

  auto t = TValue{
      {TValue::StepType::SINGLE_CYCLE, m_numLSysCopies}
  };
  for (auto& transformAdjust : transformAdjustArray)
  {
    static constexpr auto X_SCALE_RANGE = NumberRange{0.8F, 1.0F};
    static constexpr auto Y_SCALE_RANGE = NumberRange{0.9F, 1.1F};
    transformAdjust.xScale              = m_goomRand->GetRandInRange<X_SCALE_RANGE>();
    transformAdjust.yScale              = m_goomRand->GetRandInRange<Y_SCALE_RANGE>();
    transformAdjust.rotateDegrees       = t() * DEGREES_360;
    transformAdjust.translate           = {0.0F, 0.0F};

    t.Increment();
  }

  return transformAdjustArray;
}

auto LSysGeometry::UpdateCurrentTransformArray() noexcept -> void
{
  m_currentTransformArray.resize(m_numLSysCopies);

  for (auto i = 0U; i < m_numLSysCopies; ++i)
  {
    const auto rotateRadians = m_rotateSign * ToRadians(m_transformAdjustArray.at(i).rotateDegrees +
                                                        m_rotateDegreesAdjust());
    const auto spinRadians   = m_spinSign * ToRadians(m_spinDegreesAdjust());

    auto& currentTransform = m_currentTransformArray.at(i);

    currentTransform.xScale = m_xScale * m_transformAdjustArray.at(i).xScale;
    currentTransform.yScale = (m_yScale * m_transformAdjustArray.at(i).yScale) * m_yScaleAdjust();
    currentTransform.verticalMove   = m_verticalMove();
    currentTransform.sinRotateAngle = std::sin(rotateRadians);
    currentTransform.cosRotateAngle = std::cos(rotateRadians);
    currentTransform.sinSpinAngle   = std::sin(spinRadians);
    currentTransform.cosSpinAngle   = std::cos(spinRadians);
    currentTransform.translate      = m_transformAdjustArray.at(i).translate + m_translateAdjust;
  }
}

auto LSysGeometry::GetTransformedPoint(const Point2dFlt& point,
                                       const uint32_t copyNum) const noexcept -> Point2dFlt
{
  auto transformedPoint = point;

  // TODO(glk) Consolidate operations??
  transformedPoint = Scale(transformedPoint,
                           m_currentTransformArray.at(copyNum).xScale,
                           m_currentTransformArray.at(copyNum).yScale);
  transformedPoint = Rotate(transformedPoint,
                            m_currentTransformArray.at(copyNum).sinSpinAngle,
                            m_currentTransformArray.at(copyNum).cosSpinAngle);
  transformedPoint = TranslateY(transformedPoint, m_currentTransformArray.at(copyNum).verticalMove);
  transformedPoint = Rotate(transformedPoint,
                            m_currentTransformArray.at(copyNum).sinRotateAngle,
                            m_currentTransformArray.at(copyNum).cosRotateAngle);
  transformedPoint = Translate(transformedPoint, m_currentTransformArray.at(copyNum).translate);

  return transformedPoint;
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
