module;

#include <glm/vec4.hpp>
#include <memory>

export module Goom.VisualFx.ParticlesFx.Particles.TunnelEffect;

import Particles.Effect;
import Particles.ParticleGenerators;
import Particles.ParticleUpdaters;
import Particles.Particles;

using ::PARTICLES::ParticleSystem;
using ::PARTICLES::EFFECTS::IEffect;
using ::PARTICLES::GENERATORS::BasicColorGenerator;
using ::PARTICLES::GENERATORS::RoundPositionGenerator;

export namespace GOOM::VISUAL_FX::PARTICLES
{

class TunnelEffect : public IEffect
{
public:
  explicit TunnelEffect(size_t numParticles) noexcept;

  auto Reset() noexcept -> void override;

  auto SetTintColor([[maybe_unused]] const glm::vec4& tintColor) noexcept -> void override;
  auto SetTintMixAmount([[maybe_unused]] const float mixAmount) noexcept -> void override;
  auto SetMaxNumAliveParticles([[maybe_unused]] const size_t maxNumAliveParticles) noexcept
      -> void override;

  auto Update(double dt) noexcept -> void override;

  [[nodiscard]] auto GetSystem() const noexcept -> const ParticleSystem& override;

private:
  ParticleSystem m_system;
  std::shared_ptr<RoundPositionGenerator> m_positionGenerator;
  std::shared_ptr<BasicColorGenerator> m_colorGenerator;

  auto UpdateEffect(double dt) noexcept -> void;
};

} // namespace GOOM::VISUAL_FX::PARTICLES

namespace GOOM::VISUAL_FX::PARTICLES
{

inline auto TunnelEffect::Reset() noexcept -> void
{
  m_system.Reset();
}

inline auto TunnelEffect::SetTintColor([[maybe_unused]] const glm::vec4& tintColor) noexcept -> void
{
}

inline auto TunnelEffect::SetTintMixAmount([[maybe_unused]] const float mixAmount) noexcept -> void
{
}

inline auto TunnelEffect::SetMaxNumAliveParticles(
    [[maybe_unused]] const size_t maxNumAliveParticles) noexcept -> void
{
}

inline auto TunnelEffect::Update(const double dt) noexcept -> void
{
  UpdateEffect(dt);
  m_system.Update(dt);
}

inline auto TunnelEffect::GetSystem() const noexcept -> const ParticleSystem&
{
  return m_system;
}

} // namespace GOOM::VISUAL_FX::PARTICLES
