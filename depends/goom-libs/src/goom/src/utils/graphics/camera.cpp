module;

#include "goom/goom_config.h"
#include "goom/goom_types.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

module Goom.Utils.Graphics.Camera;

namespace GOOM::UTILS::GRAPHICS
{

Camera::Camera(const Properties& properties, const Dimensions& dimensions) noexcept
  : m_cameraProperties{properties},
    m_screenWidth{static_cast<float>(dimensions.GetWidth())},
    m_screenHeight{static_cast<float>(dimensions.GetHeight())},
    m_modelViewProjection{GetModelViewProjection(properties, m_screenWidth, m_screenHeight)}
{
  Expects(dimensions.GetWidth() > 0);
  Expects(dimensions.GetHeight() > 0);
}

auto Camera::GetModelViewProjection(const Properties& properties,
                                    const float screenWidth,
                                    const float screenHeight) noexcept -> glm::mat4
{
  const auto viewAspectRatio = screenWidth / screenHeight;

  const auto projection = glm::perspective(glm::radians(properties.fieldOfViewDegrees),
                                           viewAspectRatio,
                                           properties.nearZClipPlane,
                                           properties.farZClipPlane);

  const auto view =
      glm::lookAt(properties.eyePosition, properties.targetPosition, properties.upDirection);

  const auto rotateMatrix = glm::rotate(
      glm::mat4(1.0F), properties.rollRadians, properties.eyePosition - properties.targetPosition);
  const auto model = glm::mat4(properties.scale) * rotateMatrix;

  return projection * view * model;
}

} // namespace GOOM::UTILS::GRAPHICS
