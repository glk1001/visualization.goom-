#pragma once

#include "goom/goom_config.h"
#include "goom/math20.h"
#include "goom/point2d.h"
#include "tentacle2d.h"

#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

struct V3dFlt
{
  float x = 0.0;
  float y = 0.0;
  float z = 0.0;
};
[[nodiscard]] constexpr auto operator+(const V3dFlt& point1, const V3dFlt& point2) noexcept
    -> V3dFlt;
[[nodiscard]] constexpr auto lerp(const V3dFlt& point1, const V3dFlt& point2, float t) noexcept
    -> V3dFlt;

class Tentacle3D
{
public:
  Tentacle3D() noexcept = delete;
  explicit Tentacle3D(std::unique_ptr<Tentacle2D> tentacle) noexcept;
  Tentacle3D(const Tentacle3D&) noexcept = delete;
  Tentacle3D(Tentacle3D&&) noexcept      = default;
  ~Tentacle3D() noexcept;
  auto operator=(const Tentacle3D&) -> Tentacle3D& = delete;
  auto operator=(Tentacle3D&&) -> Tentacle3D&      = delete;

  // NOTE: All positions are relative to a zero origin.
  static constexpr float START_SMALL_X = 10.0F;
  [[nodiscard]] auto GetStartPos() const -> const Point2dFlt&;
  auto SetStartPos(const Point2dFlt& val) noexcept -> void;

  [[nodiscard]] auto GetEndPos() const -> const Point2dFlt&;
  auto SetEndPos(const Point2dFlt& val) noexcept -> void;
  auto SetEndPosOffset(const V3dFlt& endPosOffset) noexcept -> void;

  auto SetIterZeroYVal(float value) -> void;
  auto SetBaseYWeights(const Tentacle2D::BaseYWeights& value) -> void;
  auto StartIterating() -> void;

  auto Iterate() -> void;
  [[nodiscard]] auto GetTentacleVertices(const V3dFlt& startPosOffset) const -> std::vector<V3dFlt>;

private:
  std::unique_ptr<Tentacle2D> m_tentacle;

  Point2dFlt m_startPos{};
  Point2dFlt m_endPos{};
  V3dFlt m_endPosOffset{};
};

inline auto Tentacle3D::GetStartPos() const -> const Point2dFlt&
{
  return m_startPos;
}

inline auto Tentacle3D::SetStartPos(const Point2dFlt& val) noexcept -> void
{
  m_startPos = val;
}

inline auto Tentacle3D::GetEndPos() const -> const Point2dFlt&
{
  return m_endPos;
}

inline auto Tentacle3D::SetEndPos(const Point2dFlt& val) noexcept -> void
{
  m_endPos = val;
}

inline auto Tentacle3D::SetIterZeroYVal(const float value) -> void
{
  m_tentacle->SetIterZeroYVal(static_cast<double>(value));
}

inline auto Tentacle3D::SetBaseYWeights(const Tentacle2D::BaseYWeights& value) -> void
{
  m_tentacle->SetBaseYWeights(value);
}

inline auto Tentacle3D::StartIterating() -> void
{
  m_tentacle->StartIterating();
}

inline auto Tentacle3D::Iterate() -> void
{
  m_tentacle->Iterate();
}

constexpr auto operator+(const V3dFlt& point1, const V3dFlt& point2) noexcept -> V3dFlt
{
  return {
      point1.x + point2.x,
      point1.y + point2.y,
      point1.z + point2.z,
  };
}

constexpr auto lerp(const V3dFlt& point1, const V3dFlt& point2, const float t) noexcept -> V3dFlt
{
  return {
      STD20::lerp(point1.x, point2.x, t),
      STD20::lerp(point1.y, point2.y, t),
      STD20::lerp(point1.z, point2.z, t),
  };
}

} // namespace GOOM::VISUAL_FX::TENTACLES
