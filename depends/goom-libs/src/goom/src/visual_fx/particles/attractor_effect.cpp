module;

#include <cmath>
#include <glm/vec4.hpp>
#include <memory>

module Goom.VisualFx.ParticlesFx.Particles.AttractorEffect;

import Particles.ParticleGenerators;
import Particles.ParticleUpdaters;

namespace GOOM::VISUAL_FX::PARTICLES
{

using ::PARTICLES::ParticleEmitter;
using ::PARTICLES::GENERATORS::BasicColorGenerator;
using ::PARTICLES::GENERATORS::BasicTimeGenerator;
using ::PARTICLES::GENERATORS::BoxPositionGenerator;
using ::PARTICLES::GENERATORS::SphereVelocityGenerator;
using ::PARTICLES::UPDATERS::AttractorUpdater;
using ::PARTICLES::UPDATERS::BasicTimeUpdater;
using ::PARTICLES::UPDATERS::EulerUpdater;
using ::PARTICLES::UPDATERS::VelocityColorUpdater;

static constexpr auto EMIT_RATE_FACTOR = 0.1F;

static constexpr auto MIN_SPHERE_VELOCITY = 0.1F;
static constexpr auto MAX_SPHERE_VELOCITY = 0.1F;

static constexpr auto MIN_LIFETIME = 2.0F;
static constexpr auto MAX_LIFETIME = 100.0F;

static constexpr auto MIN_START_COLOR = glm::vec4{0.39F, 0.39F, 0.39F, 1.00F};
static constexpr auto MAX_START_COLOR = glm::vec4{0.69F, 0.69F, 0.69F, 1.00F};
static constexpr auto MIN_END_COLOR   = glm::vec4{0.09F, 0.09F, 0.09F, 0.00F};
static constexpr auto MAX_END_COLOR   = glm::vec4{0.39F, 0.39F, 0.39F, 0.25F};

static constexpr auto MIN_VELOCITY = glm::vec4{-0.5F, -0.5F, -0.5F, 0.0F};
static constexpr auto MAX_VELOCITY = glm::vec4{+2.0F, +2.0F, +2.0F, 2.0F};

static constexpr auto ATTRACTOR_POSITION0 = glm::vec4{0.0F, +0.00F, +0.75F, 1.0F};
static constexpr auto ATTRACTOR_POSITION1 = glm::vec4{0.0F, +0.00F, -0.75F, 1.0F};
static constexpr auto ATTRACTOR_POSITION2 = glm::vec4{0.0F, +0.75F, +0.00F, 1.0F};
static constexpr auto ATTRACTOR_POSITION3 = glm::vec4{0.0F, -0.75F, +0.00F, 1.0F};

static constexpr auto Z_GEN_POS0 = -0.25F;
static constexpr auto Z_GEN_POS1 = +0.25F;
static constexpr auto Z_GEN_POS2 = +0.25F;

static constexpr auto GEN_POS0              = glm::vec4{0.0F, 0.0F, Z_GEN_POS0, 0.0F};
static constexpr auto MAX_START_POS_OFFSET0 = glm::vec4{0.0F, 0.0F, 0.00F, 0.0F};
static constexpr auto GEN_POS1              = glm::vec4{0.0F, 0.0F, Z_GEN_POS1, 0.0F};
static constexpr auto MAX_START_POS_OFFSET1 = glm::vec4{0.0F, 0.0F, 0.00F, 0.0F};
static constexpr auto GEN_POS2              = glm::vec4{0.0F, 0.0F, Z_GEN_POS2, 0.0F};
static constexpr auto MAX_START_POS_OFFSET2 = glm::vec4{0.0F, 0.0F, 0.00F, 0.0F};

static constexpr auto POS_LIFETIME_FACTOR0 = 2.5F;
static constexpr auto POS_LIFETIME_FACTOR1 = 2.0F;
static constexpr auto POS_LIFETIME_FACTOR2 = 6.0F;

static constexpr auto EULER_ACCELERATION = glm::vec4{0.0F, 0.0F, 0.0F, 0.0F};

static constexpr auto DEFAULT_NUM_PARTICLES = 250000UZ;

AttractorEffect::AttractorEffect(const size_t numParticles) noexcept
  : m_system{0 == numParticles ? DEFAULT_NUM_PARTICLES : numParticles},
    m_colorGenerator{std::make_unique<BasicColorGenerator>(
        MIN_START_COLOR, MAX_START_COLOR, MIN_END_COLOR, MAX_END_COLOR)},
    m_attractorUpdater{std::make_shared<AttractorUpdater>()},
    m_colorUpdater{std::make_shared<VelocityColorUpdater>(MIN_VELOCITY, MAX_VELOCITY)}
{
  //
  // common
  //
  const auto numParticlesToUse = m_system.GetNumAllParticles();

  const auto velocityGenerator =
      std::make_shared<SphereVelocityGenerator>(MIN_SPHERE_VELOCITY, MAX_SPHERE_VELOCITY);

  const auto timeGenerator = std::make_shared<BasicTimeGenerator>(MIN_LIFETIME, MAX_LIFETIME);

  //
  // emitter 0:
  //
  m_particleEmitters[0] = std::make_shared<ParticleEmitter>();
  m_particleEmitters[0]->SetEmitRate(EMIT_RATE_FACTOR * static_cast<float>(numParticlesToUse));

  m_positionGenerators[0] = std::make_shared<BoxPositionGenerator>(GEN_POS0, MAX_START_POS_OFFSET0);
  m_particleEmitters[0]->AddGenerator(m_positionGenerators[0]);

  m_particleEmitters[0]->AddGenerator(m_colorGenerator);
  m_particleEmitters[0]->AddGenerator(velocityGenerator);
  m_particleEmitters[0]->AddGenerator(timeGenerator);
  m_system.AddEmitter(m_particleEmitters[0]);

  //
  // emitter 1:
  //
  m_particleEmitters[1] = std::make_shared<ParticleEmitter>();
  m_particleEmitters[1]->SetEmitRate(EMIT_RATE_FACTOR * static_cast<float>(numParticlesToUse));

  m_positionGenerators[1] = std::make_shared<BoxPositionGenerator>(GEN_POS1, MAX_START_POS_OFFSET1);
  m_particleEmitters[1]->AddGenerator(m_positionGenerators[1]);

  m_particleEmitters[1]->AddGenerator(m_colorGenerator);
  m_particleEmitters[1]->AddGenerator(velocityGenerator);
  m_particleEmitters[1]->AddGenerator(timeGenerator);
  m_system.AddEmitter(m_particleEmitters[1]);

  //
  // emitter 2:
  //
  m_particleEmitters[2] = std::make_shared<ParticleEmitter>();
  m_particleEmitters[2]->SetEmitRate(EMIT_RATE_FACTOR * static_cast<float>(numParticlesToUse));

  m_positionGenerators[2] = std::make_shared<BoxPositionGenerator>(GEN_POS2, MAX_START_POS_OFFSET2);
  m_particleEmitters[2]->AddGenerator(m_positionGenerators[2]);

  m_particleEmitters[2]->AddGenerator(m_colorGenerator);
  m_particleEmitters[2]->AddGenerator(velocityGenerator);
  m_particleEmitters[2]->AddGenerator(timeGenerator);
  m_system.AddEmitter(m_particleEmitters[2]);

  //
  // updaters:
  //
  m_system.AddUpdater(m_colorUpdater);

  m_attractorUpdater->AddAttractorPosition(ATTRACTOR_POSITION0);
  m_attractorUpdater->AddAttractorPosition(ATTRACTOR_POSITION1);
  m_attractorUpdater->AddAttractorPosition(ATTRACTOR_POSITION2);
  m_attractorUpdater->AddAttractorPosition(ATTRACTOR_POSITION3);
  m_system.AddUpdater(m_attractorUpdater);

  const auto timeUpdater = std::make_shared<BasicTimeUpdater>();
  m_system.AddUpdater(timeUpdater);

  const auto eulerUpdater = std::make_shared<EulerUpdater>(EULER_ACCELERATION);
  m_system.AddUpdater(eulerUpdater);
}

auto AttractorEffect::SetMaxNumAliveParticles(const size_t maxNumAliveParticles) noexcept -> void
{
  for (auto& particleEmitter : m_particleEmitters)
  {
    particleEmitter->SetMaxNumAliveParticles(maxNumAliveParticles);
  }
}

auto AttractorEffect::UpdateEffect(const double dt) -> void
{
  static auto s_lifetime = 0.0F;
  s_lifetime += static_cast<float>(dt);

  static constexpr auto RADIUS = 0.55F;

  const auto zScale = 1.0F;

  m_positionGenerators[0]->SetPosition(
      {+RADIUS * std::sin(s_lifetime * POS_LIFETIME_FACTOR0),
       +RADIUS * std::cos(s_lifetime * POS_LIFETIME_FACTOR0),
       zScale * Z_GEN_POS0 * std::cos(s_lifetime * POS_LIFETIME_FACTOR0),
       0.0F});

  m_positionGenerators[1]->SetPosition(
      {-RADIUS * std::sin(s_lifetime * POS_LIFETIME_FACTOR1),
       +RADIUS * std::cos(s_lifetime * POS_LIFETIME_FACTOR1),
       zScale * Z_GEN_POS1 * std::cos(s_lifetime * POS_LIFETIME_FACTOR1),
       0.0F});

  m_positionGenerators[2]->SetPosition(
      {-RADIUS * std::sin(s_lifetime * POS_LIFETIME_FACTOR2),
       +RADIUS * std::cos(s_lifetime * POS_LIFETIME_FACTOR2),
       zScale * Z_GEN_POS2 * std::cos(s_lifetime * POS_LIFETIME_FACTOR2),
       0.0F});
}

} // namespace GOOM::VISUAL_FX::PARTICLES
