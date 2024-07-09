module;

#include <array>
#include <cstdlib>
#include <glm/fwd.hpp>
#include <memory>

export module Goom.VisualFx.ParticlesFx.Particles.AttractorEffect;

import Particles.Effect;
import Particles.ParticleGenerators;
import Particles.ParticleUpdaters;
import Particles.Particles;

using ::PARTICLES::ParticleEmitter;
using ::PARTICLES::ParticleSystem;
using ::PARTICLES::EFFECTS::IEffect;
using ::PARTICLES::GENERATORS::BoxPositionGenerator;
using ::PARTICLES::UPDATERS::VelocityColorUpdater;

export namespace GOOM::VISUAL_FX::PARTICLES
{

class AttractorEffect : public IEffect
{
public:
  static constexpr auto NUM_EMITTERS = 3U;
  static constexpr auto NUM_BOX_POS_GENERATORS = 3U;

  explicit AttractorEffect(size_t numParticles) noexcept;

  auto Reset() noexcept -> void override;

  auto SetTintColor(const glm::vec4& tintColor) noexcept -> void override;
  auto SetTintMixAmount(float mixAmount) noexcept -> void override;
  auto SetMaxNumAliveParticles(size_t maxNumAliveParticles) noexcept -> void override;

  auto Update(double dt) noexcept -> void override;

  [[nodiscard]] auto GetSystem() const noexcept -> const ParticleSystem& override;

private:
  ParticleSystem m_system;

  std::shared_ptr<VelocityColorUpdater> m_colorUpdater;

  std::array<std::shared_ptr<ParticleEmitter>, NUM_EMITTERS> m_particleEmitters;

  std::array<std::shared_ptr<BoxPositionGenerator>, NUM_BOX_POS_GENERATORS> m_positionGenerators;

  auto UpdateEffect(double dt) -> void;
};

} // namespace GOOM::VISUAL_FX::PARTICLES

namespace GOOM::VISUAL_FX::PARTICLES
{

inline auto AttractorEffect::Reset() noexcept -> void
{
  m_system.Reset();
}

inline auto AttractorEffect::SetTintColor(const glm::vec4& tintColor) noexcept -> void
{
  m_colorUpdater->SetTintColor(tintColor);
}

inline auto AttractorEffect::SetTintMixAmount(const float mixAmount) noexcept -> void
{
  m_colorUpdater->SetTintMixAmount(mixAmount);
}

inline auto AttractorEffect::Update(const double dt) noexcept -> void
{
  UpdateEffect(dt);
  m_system.Update(dt);
}

inline auto AttractorEffect::GetSystem() const noexcept -> const ParticleSystem&
{
  return m_system;
}

} // namespace GOOM::VISUAL_FX::PARTICLES
