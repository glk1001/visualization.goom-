module;

#include <glm/vec4.hpp>
#include <memory>

export module Particles.Effects.FountainEffect;

import Particles.Effect;
import Particles.ParticleGenerators;
import Particles.ParticleUpdaters;
import Particles.Particles;

using PARTICLES::GENERATORS::BasicColorGenerator;
using PARTICLES::GENERATORS::BoxPositionGenerator;
using PARTICLES::UPDATERS::EulerUpdater;
using PARTICLES::UPDATERS::FloorUpdater;

export namespace PARTICLES::EFFECTS
{

class FountainEffect : public IEffect
{
public:
  explicit FountainEffect(size_t numParticles) noexcept;

  auto Reset() noexcept -> void override;

  auto SetTintColor([[maybe_unused]] const glm::vec4& tintColor) noexcept -> void override;
  auto SetTintMixAmount([[maybe_unused]] const float mixAmount) noexcept -> void override;
  auto SetMaxNumAliveParticles([[maybe_unused]] const size_t maxNumAliveParticles) noexcept
      -> void override;

  auto Update(double dt) noexcept -> void override;

  [[nodiscard]] auto GetSystem() const noexcept -> const ParticleSystem& override;

private:
  ParticleSystem m_system;
  std::shared_ptr<BoxPositionGenerator> m_positionGenerator;
  std::shared_ptr<BasicColorGenerator> m_colorGenerator;
  std::shared_ptr<EulerUpdater> m_eulerUpdater;
  std::shared_ptr<FloorUpdater> m_floorUpdater;
  static constexpr auto FLOOR_Y = -0.25F;

  auto UpdateEffect(double dt) noexcept -> void;
};

} // namespace PARTICLES::EFFECTS

namespace PARTICLES::EFFECTS
{

inline auto FountainEffect::Reset() noexcept -> void
{
  m_system.Reset();
}

inline auto FountainEffect::SetTintColor([[maybe_unused]] const glm::vec4& tintColor) noexcept
    -> void
{
}

inline auto FountainEffect::SetTintMixAmount([[maybe_unused]] const float mixAmount) noexcept
    -> void
{
}

inline auto FountainEffect::SetMaxNumAliveParticles(
    [[maybe_unused]] const size_t maxNumAliveParticles) noexcept -> void
{
}

inline auto FountainEffect::Update(const double dt) noexcept -> void
{
  UpdateEffect(dt);
  m_system.Update(dt);
}

inline auto FountainEffect::GetSystem() const noexcept -> const ParticleSystem&
{
  return m_system;
}

} // namespace PARTICLES::EFFECTS
