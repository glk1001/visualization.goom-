#pragma once

#include "misc.h"
#include "point2d.h"

#include <cmath>

namespace GOOM::UTILS::MATH
{

class Transform2d
{
public:
  Transform2d() noexcept = default;
  Transform2d(float angleInRadians, float scale, const Vec2dFlt& translation) noexcept;

  auto SetRotation(float angleInRadians) -> void;
  auto SetScale(float scale) -> void;
  auto SetTranslation(const Vec2dFlt& translation) -> void;

  [[nodiscard]] auto GetTransformedPoint(const Point2dInt& point) const -> Point2dInt;
  [[nodiscard]] auto GetTransformedPoint(const Point2dFlt& point) const -> Point2dFlt;

private:
  float m_rotationAngle = 0.0F;
  bool m_rotationAngleSet = false;
  float m_sinRotationAngle = 0.0F;
  float m_cosRotationAngle = 1.0F;
  float m_scale = 1.0F;
  bool m_scaleSet = false;
  Vec2dFlt m_translation{};
  bool m_translationSet = false;
};

inline Transform2d::Transform2d(const float angleInRadians,
                                const float scale,
                                const Vec2dFlt& translation) noexcept
  : m_rotationAngle{angleInRadians},
    m_rotationAngleSet{!FloatsEqual(0.0F, m_rotationAngle)},
    m_sinRotationAngle{std::sin(m_rotationAngle)},
    m_cosRotationAngle{std::cos(m_rotationAngle)},
    m_scale{scale},
    m_scaleSet{!FloatsEqual(1.0F, m_scale)},
    m_translation{translation},
    m_translationSet{!FloatsEqual(0.0F, m_translation.x) && !FloatsEqual(0.0F, m_translation.y)}
{
}

inline auto Transform2d::SetRotation(const float angleInRadians) -> void
{
  m_rotationAngle = angleInRadians;
  m_rotationAngleSet = !FloatsEqual(0.0F, m_rotationAngle);
  m_sinRotationAngle = std::sin(m_rotationAngle);
  m_cosRotationAngle = std::cos(m_rotationAngle);
}

inline auto Transform2d::SetScale(const float scale) -> void
{
  m_scale = scale;
  m_scaleSet = !FloatsEqual(1.0F, m_scale);
}

inline auto Transform2d::SetTranslation(const Vec2dFlt& translation) -> void
{
  m_translation = translation;
  m_translationSet = (!FloatsEqual(0.0F, m_translation.x)) && (!FloatsEqual(0.0F, m_translation.y));
}

inline auto Transform2d::GetTransformedPoint(const Point2dInt& point) const -> Point2dInt
{
  return GetTransformedPoint(point.ToFlt()).ToInt();
}

inline auto Transform2d::GetTransformedPoint(const Point2dFlt& point) const -> Point2dFlt
{
  Point2dFlt transformedPoint;

  if (!m_rotationAngleSet)
  {
    transformedPoint = point;
  }
  else
  {
    transformedPoint.x = (point.x * m_cosRotationAngle) - (point.y * m_sinRotationAngle);
    transformedPoint.y = (point.x * m_sinRotationAngle) + (point.y * m_cosRotationAngle);
  }

  if (m_scaleSet)
  {
    transformedPoint.Scale(m_scale);
  }

  if (m_translationSet)
  {
    transformedPoint.Translate(m_translation);
  }

  return transformedPoint;
}

} // namespace GOOM::UTILS::MATH
