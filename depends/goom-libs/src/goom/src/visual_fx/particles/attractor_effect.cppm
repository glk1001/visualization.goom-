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

using ::PARTICLES::EFFECTS::IEffect;
using ::PARTICLES::GENERATORS::BasicColorGenerator;
using ::PARTICLES::GENERATORS::BoxPositionGenerator;
using ::PARTICLES::UPDATERS::EulerUpdater;
using ::PARTICLES::ParticleSystem;
using ::PARTICLES::ParticleEmitter;
using ::PARTICLES::UPDATERS::AttractorUpdater;
using ::PARTICLES::UPDATERS::VelocityColorUpdater;

export namespace GOOM::VISUAL_FX::PARTICLES
{

class AttractorEffect : public IEffect
{
public:
  explicit AttractorEffect(size_t numParticles) noexcept;

  auto Reset() noexcept -> void override;

  auto SetTintColor(const glm::vec4& tintColor) noexcept -> void override;
  auto SetTintMixAmount(float mixAmount) noexcept -> void override;
  auto SetMaxNumAliveParticles(size_t maxNumAliveParticles) noexcept -> void override;

  auto Update(double dt) noexcept -> void override;

  [[nodiscard]] auto GetSystem() const noexcept -> const ParticleSystem& override;

private:
  ParticleSystem m_system;

  static constexpr auto NUM_EMITTERS = 3U;
  std::array<std::shared_ptr<ParticleEmitter>, NUM_EMITTERS> m_particleEmitters;

  static constexpr auto NUM_BOX_POS_GENERATORS = 3U;
  std::array<std::shared_ptr<BoxPositionGenerator>, NUM_BOX_POS_GENERATORS> m_positionGenerators;
  static constexpr auto Z_GEN_POS1 = -0.25F;
  static constexpr auto Z_GEN_POS2 = +0.25F;
  static constexpr auto Z_GEN_POS3 = +0.25F;

  std::shared_ptr<BasicColorGenerator> m_colorGenerator;
  std::shared_ptr<AttractorUpdater> m_attractorUpdater;
  std::shared_ptr<VelocityColorUpdater> m_colorUpdater;

  auto UpdateEffect(double dt) -> void;
};

} // namespace PARTICLES::EFFECTS

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
