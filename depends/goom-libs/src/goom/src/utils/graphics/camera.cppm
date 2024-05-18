module;

#include "goom/goom_config.h"

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

export module Goom.Utils.Graphics.Camera;

import Goom.Utils.Math.Misc;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

export namespace GOOM::UTILS::GRAPHICS
{

class Camera
{
public:
  struct Properties
  {
    static constexpr auto DEFAULT_FIELD_OF_VIEW_DEGREES = 45.0F;
    static constexpr auto DEFAULT_NEAR_Z_CLIP_PLANE     = 0.1F;
    static constexpr auto DEFAULT_FAR_Z_CLIP_PLANE      = 100.0F;

    float scale              = 1.0F;
    float fieldOfViewDegrees = DEFAULT_FIELD_OF_VIEW_DEGREES;
    float nearZClipPlane     = DEFAULT_NEAR_Z_CLIP_PLANE;
    float farZClipPlane      = DEFAULT_FAR_Z_CLIP_PLANE;
    glm::vec3 eyePosition    = glm::vec3{0.0F, 0.0F, -1.0F};
    glm::vec3 targetPosition = glm::vec3{0.0F, 0.0F, 0.0F};
    glm::vec3 upDirection    = glm::vec3{0.0F, 1.0F, 0.0F};
    float rollRadians        = 0.0F;
  };

  Camera(const Properties& properties, const Dimensions& dimensions) noexcept;

  auto SetEyePosition(const glm::vec3& eyePosition) noexcept -> void;
  auto SetTargetPosition(const glm::vec3& targetPosition) noexcept -> void;
  auto SetRoll(float rollRadians) noexcept -> void;

  auto SetScreenPositionOffset(const Point2dInt& offset) noexcept -> void;
  [[nodiscard]] auto GetScreenPosition(const glm::vec4& worldPosition) const noexcept -> Point2dInt;

private:
  Properties m_cameraProperties;
  float m_screenWidth;
  float m_screenHeight;
  float m_halfScreenWidth  = UTILS::MATH::HALF * m_screenWidth;
  float m_halfScreenHeight = UTILS::MATH::HALF * m_screenHeight;
  glm::mat4 m_modelViewProjection;
  Point2dInt m_screenPositionOffset{};

  [[nodiscard]] static auto GetModelViewProjection(const Properties& properties,
                                                   float screenWidth,
                                                   float screenHeight) noexcept -> glm::mat4;
};

} // namespace GOOM::UTILS::GRAPHICS

namespace GOOM::UTILS::GRAPHICS
{

inline auto Camera::SetEyePosition(const glm::vec3& eyePosition) noexcept -> void
{
  m_cameraProperties.eyePosition = eyePosition;
  m_modelViewProjection = GetModelViewProjection(m_cameraProperties, m_screenWidth, m_screenHeight);
}

inline auto Camera::SetTargetPosition(const glm::vec3& targetPosition) noexcept -> void
{
  m_cameraProperties.targetPosition = targetPosition;
  m_modelViewProjection = GetModelViewProjection(m_cameraProperties, m_screenWidth, m_screenHeight);
}

inline auto Camera::SetRoll(const float rollRadians) noexcept -> void
{
  m_cameraProperties.rollRadians = rollRadians;
  m_modelViewProjection = GetModelViewProjection(m_cameraProperties, m_screenWidth, m_screenHeight);
}

inline auto Camera::SetScreenPositionOffset(const Point2dInt& offset) noexcept -> void
{
  m_screenPositionOffset = offset;
}

inline auto Camera::GetScreenPosition(const glm::vec4& worldPosition) const noexcept
    -> GOOM::Point2dInt
{
  const auto mvpPos = m_modelViewProjection * worldPosition;

  // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access): union hard to fix here
  return m_screenPositionOffset + ToVec2dInt(Vec2dFlt{m_halfScreenWidth * (1.0F + mvpPos.x),
                                                      m_halfScreenHeight * (1.0F - mvpPos.y)});
  // NOLINTEND(cppcoreguidelines-pro-type-union-access)
}

} // namespace GOOM::UTILS::GRAPHICS
