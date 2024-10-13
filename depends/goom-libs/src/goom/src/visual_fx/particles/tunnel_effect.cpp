module;

#include <cmath>
#include <glm/vec4.hpp>
#include <memory>

module Particles.Effects.TunnelEffect;

import Particles.ParticleGenerators;
import Particles.ParticleUpdaters;

namespace PARTICLES::EFFECTS
{

using GENERATORS::BasicColorGenerator;
using GENERATORS::BasicTimeGenerator;
using GENERATORS::BasicVelocityGenerator;
using GENERATORS::RoundPositionGenerator;
using UPDATERS::BasicTimeUpdater;
using UPDATERS::EulerUpdater;
using UPDATERS::PositionColorUpdater;

TunnelEffect::TunnelEffect(const size_t numParticles) noexcept
  : m_system{0 == numParticles ? 10000 : numParticles}
{
  const auto numParticlesToUse = m_system.GetNumAllParticles();

  //
  // emitter:
  //
  const auto particleEmitter             = std::make_shared<ParticleEmitter>();
  static constexpr auto EMIT_RATE_FACTOR = 0.45F;
  particleEmitter->SetEmitRate(EMIT_RATE_FACTOR * static_cast<float>(numParticlesToUse));

  // pos:
  static constexpr auto ROUND_POS_CENTER = glm::vec4{0.0, 0.0, 0.0, 0.0};
  static constexpr auto X_RADIUS         = 0.15F;
  static constexpr auto Y_RADIUS         = 0.15F;
  m_positionGenerator =
      std::make_shared<RoundPositionGenerator>(ROUND_POS_CENTER, X_RADIUS, Y_RADIUS);
  particleEmitter->AddGenerator(m_positionGenerator);

  static constexpr auto MIN_START_COLOR = glm::vec4{0.7F, 0.0F, 0.7F, 1.0F};
  static constexpr auto MAX_START_COLOR = glm::vec4{1.0F, 1.0F, 1.0F, 1.0F};
  static constexpr auto MIN_END_COLOR   = glm::vec4{0.5F, 0.0F, 0.6F, 0.0F};
  static constexpr auto MAX_END_COLOR   = glm::vec4{0.7F, 0.5F, 1.0F, 0.0F};
  m_colorGenerator                      = std::make_shared<BasicColorGenerator>(
      MIN_START_COLOR, MAX_START_COLOR, MIN_END_COLOR, MAX_END_COLOR);
  particleEmitter->AddGenerator(m_colorGenerator);

  static constexpr auto MIN_START_VELOCITY = glm::vec4{0.0F, 0.0F, 0.15F, 0.0F};
  static constexpr auto MAX_START_VELOCITY = glm::vec4{0.0F, 0.0F, 0.45F, 0.0F};
  const auto velocityGenerator =
      std::make_shared<BasicVelocityGenerator>(MIN_START_VELOCITY, MAX_START_VELOCITY);
  particleEmitter->AddGenerator(velocityGenerator);

  static constexpr auto MIN_LIFETIME = 1.0F;
  static constexpr auto MAX_LIFETIME = 3.5F;
  const auto timeGenerator = std::make_shared<BasicTimeGenerator>(MIN_LIFETIME, MAX_LIFETIME);
  particleEmitter->AddGenerator(timeGenerator);

  m_system.AddEmitter(particleEmitter);

  const auto timeUpdater = std::make_shared<BasicTimeUpdater>();
  m_system.AddUpdater(timeUpdater);

  //const auto colorUpdater = std::make_shared<BasicColorUpdater>();
  static constexpr auto MIN_COLOR_POSITION = glm::vec4{-0.5F, -0.5F, -0.5F, 0.0F};
  static constexpr auto MAX_COLOR_POSITION = glm::vec4{+2.0F, +3.0F, +3.0F, 2.0F};
  const auto colorUpdater =
      std::make_shared<PositionColorUpdater>(MIN_COLOR_POSITION, MAX_COLOR_POSITION);
  m_system.AddUpdater(colorUpdater);

  static constexpr auto EULER_ACCELERATION = glm::vec4{0.0F, 0.0F, 0.0F, 0.0F};
  const auto eulerUpdater                  = std::make_shared<EulerUpdater>(EULER_ACCELERATION);
  m_system.AddUpdater(eulerUpdater);
}

auto TunnelEffect::UpdateEffect(const double dt) noexcept -> void
{
  static auto s_lifetime = 0.0F;
  s_lifetime += static_cast<float>(dt);

  static constexpr auto LIFETIME_FACTOR = 2.5F;
  static constexpr auto CENTRE_FACTOR   = 0.1F;
  const auto centre = glm::vec4{CENTRE_FACTOR * std::sin(s_lifetime * LIFETIME_FACTOR),
                                CENTRE_FACTOR * std::cos(s_lifetime * LIFETIME_FACTOR),
                                0.0F,
                                0.0F};

  static constexpr auto MIN_RADIUS          = 0.15F;
  static constexpr auto RADIUS_FACTOR       = 0.05F;
  static constexpr auto Y_RADIUS_COS_FACTOR = 0.5F;
  const auto xRadius                        = MIN_RADIUS + (RADIUS_FACTOR * std::sin(s_lifetime));
  //      0.15F + (0.01F * std::sin(time)),
  const auto yRadius =
      MIN_RADIUS +
      (RADIUS_FACTOR * (std::sin(s_lifetime) * std::cos(s_lifetime * Y_RADIUS_COS_FACTOR)));
  //      0.15F + (0.01F * std::cos(time)));

  m_positionGenerator->SetCentreAndRadius(centre, xRadius, yRadius);
}

} // namespace PARTICLES::EFFECTS
