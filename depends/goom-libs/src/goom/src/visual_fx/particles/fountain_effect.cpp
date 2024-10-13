module;

#include <cmath>
#include <glm/vec4.hpp>
#include <memory>

module Particles.Effects.FountainEffect;

import Particles.ParticleGenerators;
import Particles.ParticleUpdaters;

namespace PARTICLES::EFFECTS
{

using GENERATORS::BasicColorGenerator;
using GENERATORS::BasicTimeGenerator;
using GENERATORS::BasicVelocityGenerator;
using GENERATORS::BoxPositionGenerator;
using UPDATERS::BasicTimeUpdater;
using UPDATERS::EulerUpdater;
using UPDATERS::FloorUpdater;
using UPDATERS::VelocityColorUpdater;

FountainEffect::FountainEffect(const size_t numParticles) noexcept
  : m_system{0 == numParticles ? 10000 : numParticles}
{
  const auto numParticlesToUse = m_system.GetNumAllParticles();

  //
  // emitter:
  //
  const auto particleEmitter             = std::make_shared<ParticleEmitter>();
  static constexpr auto EMIT_RATE_FACTOR = 0.25F;
  particleEmitter->SetEmitRate(EMIT_RATE_FACTOR * static_cast<float>(numParticlesToUse));

  // pos:
  static constexpr auto GEN_POS              = glm::vec4{0.0F, FLOOR_Y, 0.0F, 0.0F};
  static constexpr auto MAX_START_POS_OFFSET = glm::vec4{0.0F, 0.0F, 0.0F, 0.0F};
  m_positionGenerator = std::make_shared<BoxPositionGenerator>(GEN_POS, MAX_START_POS_OFFSET);
  particleEmitter->AddGenerator(m_positionGenerator);

  static constexpr auto MIN_START_COLOR = glm::vec4{0.7F, 0.7F, 0.7F, 0.5F};
  static constexpr auto MAX_START_COLOR = glm::vec4{1.0F, 1.0F, 1.0F, 0.7F};
  static constexpr auto MIN_END_COLOR   = glm::vec4{0.5F, 0.0F, 0.6F, 0.5F};
  static constexpr auto MAX_END_COLOR   = glm::vec4{0.7F, 0.5F, 1.0F, 0.7F};
  m_colorGenerator                      = std::make_shared<BasicColorGenerator>(
      MIN_START_COLOR, MAX_START_COLOR, MIN_END_COLOR, MAX_END_COLOR);
  particleEmitter->AddGenerator(m_colorGenerator);

  static constexpr auto MIN_START_VELOCITY = glm::vec4{-0.2F, 0.52F, 0.0F, 0.0F};
  static constexpr auto MAX_START_VELOCITY = glm::vec4{+0.2F, 0.75F, 0.0F, 0.0F};
  const auto velocityGenerator =
      std::make_shared<BasicVelocityGenerator>(MIN_START_VELOCITY, MAX_START_VELOCITY);
  particleEmitter->AddGenerator(velocityGenerator);

  static constexpr auto MIN_LIFETIME = 3.0F;
  static constexpr auto MAX_LIFETIME = 4.0F;
  const auto timeGenerator = std::make_shared<BasicTimeGenerator>(MIN_LIFETIME, MAX_LIFETIME);
  particleEmitter->AddGenerator(timeGenerator);

  m_system.AddEmitter(particleEmitter);

  const auto timeUpdater = std::make_shared<BasicTimeUpdater>();
  m_system.AddUpdater(timeUpdater);

  //const auto colorUpdater = std::make_shared<BasicColorUpdater>();
  static constexpr auto MIN_VELOCITY = glm::vec4{-0.5F, -0.5F, -0.5F, 0.0F};
  static constexpr auto MAX_VELOCITY = glm::vec4{+2.0F, +2.0F, +2.0F, 2.0F};
  const auto colorUpdater = std::make_shared<VelocityColorUpdater>(MIN_VELOCITY, MAX_VELOCITY);
  m_system.AddUpdater(colorUpdater);

  static constexpr auto GRAVITY            = -25.0F;
  static constexpr auto EULER_ACCELERATION = glm::vec4{0.0F, GRAVITY, 0.0F, 0.0F};
  m_eulerUpdater                           = std::make_shared<EulerUpdater>(EULER_ACCELERATION);
  m_system.AddUpdater(m_eulerUpdater);

  static constexpr auto BOUNCE_FACTOR = 0.5F;
  m_floorUpdater                      = std::make_shared<FloorUpdater>(FLOOR_Y, BOUNCE_FACTOR);
  m_system.AddUpdater(m_floorUpdater);
}

auto FountainEffect::UpdateEffect(const double dt) noexcept -> void
{
  static auto s_lifetime = 0.0F;
  s_lifetime += static_cast<float>(dt);

  static constexpr auto LIFETIME_FACTOR = 2.5F;
  static constexpr auto POS_FACTOR      = 0.1F;
  m_positionGenerator->SetPosition({POS_FACTOR * std::sin(s_lifetime * LIFETIME_FACTOR),
                                    FLOOR_Y,
                                    POS_FACTOR * std::cos(s_lifetime * LIFETIME_FACTOR),
                                    0.0F});
}

} // namespace PARTICLES::EFFECTS
