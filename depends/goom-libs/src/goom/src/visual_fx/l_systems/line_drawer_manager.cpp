#include "line_drawer_manager.h"

#include "draw/goom_draw.h"
#include "draw/shape_drawers/line_drawer_moving_noise.h"
#include "goom_config.h"
#include "utils/math/goom_rand_base.h"

namespace GOOM::VISUAL_FX::L_SYSTEM
{
using DRAW::IGoomDraw;
using DRAW::SHAPE_DRAWERS::LineDrawerMovingNoise;
using UTILS::MATH::IGoomRand;

static constexpr auto LINE_BRIGHTNESS_FACTOR = 2.0F;

static constexpr auto SWITCH_LINE_DRAWER_TYPE_CONST_WEIGHT  = 0.4F;
static constexpr auto SWITCH_LINE_DRAWER_TYPE_MOVING_WEIGHT = 0.5F;
static constexpr auto SWITCH_LINE_DRAWER_TYPE_NONE_WEIGHT   = 0.1F;

// clang-format off
LineDrawerManager::LineDrawerManager(IGoomDraw& draw, const IGoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_lineDrawerWithMovingNoiseEffect{
        LineDrawerMovingNoise{draw,
                              *m_goomRand,
                              MIN_MAX_MOVING_NOISE_RADIUS,
                              NUM_MOVING_NOISE_RADIUS_STEPS,
                              MIN_MAX_NUM_MOVING_NOISE_PIXELS,
                              NUM_NUM_MOVING_NOISE_PIXEL_STEPS}
    },
    m_switchLineDrawerWeights{
        *m_goomRand,
        {
            { SwitchLineDrawerType::CONST, SWITCH_LINE_DRAWER_TYPE_CONST_WEIGHT },
            { SwitchLineDrawerType::MOVING, SWITCH_LINE_DRAWER_TYPE_MOVING_WEIGHT },
            { SwitchLineDrawerType::NONE, SWITCH_LINE_DRAWER_TYPE_NONE_WEIGHT },
        }
    }
{
  m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().SetUseMainPointWithoutNoise(false);
  m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().SetBrightnessFactor(LINE_BRIGHTNESS_FACTOR);
}
// clang-format on

auto LineDrawerManager::SwitchLineDrawers() noexcept -> void
{
  SwitchLineDrawers(m_switchLineDrawerWeights.GetRandomWeighted());
}

auto LineDrawerManager::SwitchLineDrawers(const SwitchLineDrawerType forceType) noexcept -> void
{
  switch (forceType)
  {
    case SwitchLineDrawerType::CONST:
    {
      const auto currentNoiseRadius =
          m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().GetCurrentNoiseRadius();
      const auto currentNumNoisePixelsPerPixel =
          m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().GetCurrentNumNoisePixelsPerPixel();
      m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().SetMinMaxNoiseValues(
          {currentNoiseRadius, currentNoiseRadius},
          {currentNumNoisePixelsPerPixel, currentNumNoisePixelsPerPixel});
      break;
    }
    case SwitchLineDrawerType::NONE:
    {
      m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().SetMinMaxNoiseValues({0U, 0U}, {1U, 1U});
      break;
    }
    case SwitchLineDrawerType::MOVING:
    {
      m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().SetMinMaxNoiseValues(
          MIN_MAX_MOVING_NOISE_RADIUS, MIN_MAX_NUM_MOVING_NOISE_PIXELS);
      break;
    }
    default:
      FailFast();
  }
}

auto LineDrawerManager::Update() noexcept -> void
{
  m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().IncrementNoise();
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
