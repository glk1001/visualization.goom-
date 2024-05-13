#include "all_standard_visual_fx.h"

#include "goom/goom_config.h"
#include "goom/point2d.h"
#include "goom_effects.h"
#include "goom_states.h"
#include "utils/enum_utils.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/stopwatch.h"
#include "visual_fx_color_maps.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_set>

import Goom.VisualFx.FxHelper;
import Goom.VisualFx.CirclesFx;
import Goom.VisualFx.FlyingStarsFx;
import Goom.VisualFx.GoomDotsFx;
import Goom.VisualFx.IfsDancersFx;
import Goom.VisualFx.ImageFx;
import Goom.VisualFx.LinesFx;
import Goom.VisualFx.LSystemFx;
import Goom.VisualFx.ParticlesFx;
import Goom.VisualFx.RaindropsFx;
import Goom.VisualFx.ShaderFx;
import Goom.VisualFx.ShapesFx;
import Goom.VisualFx.TentaclesFx;
import Goom.VisualFx.TubesFx;
import Goom.VisualFx.VisualFxBase;

namespace GOOM::CONTROL
{

using CONTROL::GoomDrawables;
using UTILS::NUM;
using UTILS::Parallel;
using UTILS::Stopwatch;
using UTILS::GRAPHICS::SmallImageBitmaps;
using VISUAL_FX::CirclesFx;
using VISUAL_FX::FlyingStarsFx;
using VISUAL_FX::FxHelper;
using VISUAL_FX::GoomDotsFx;
using VISUAL_FX::IfsDancersFx;
using VISUAL_FX::ImageFx;
using VISUAL_FX::IVisualFx;
using VISUAL_FX::LinesFx;
using VISUAL_FX::LSystemFx;
using VISUAL_FX::ParticlesFx;
using VISUAL_FX::RaindropsFx;
using VISUAL_FX::ShaderFx;
using VISUAL_FX::ShapesFx;
using VISUAL_FX::TentaclesFx;
using VISUAL_FX::TubesFx;
using VISUAL_FX::FLYING_STARS::StarTypesContainer;

AllStandardVisualFx::AllStandardVisualFx(Parallel& parallel,
                                         FxHelper& fxHelper,
                                         const SmallImageBitmaps& smallBitmaps,
                                         const std::string& resourcesDirectory) noexcept
  : m_shaderFx{std::make_unique<ShaderFx>(fxHelper)},
    m_drawablesMap{GetDrawablesMap(parallel, fxHelper, smallBitmaps, resourcesDirectory)},
    m_visualFxColorMaps{fxHelper.GetGoomRand()}
{
  Expects(NUM<GoomDrawables> == m_drawablesMap.size());
}

auto AllStandardVisualFx::GetDrawablesMap(Parallel& parallel,
                                          FxHelper& fxHelper,
                                          const SmallImageBitmaps& smallBitmaps,
                                          const std::string& resourcesDirectory)
    -> UTILS::EnumMap<GoomDrawables, std::unique_ptr<IVisualFx>>
{
  return UTILS::EnumMap<GoomDrawables, std::unique_ptr<IVisualFx>>{{{
      {GoomDrawables::CIRCLES, std::make_unique<CirclesFx>(fxHelper, smallBitmaps)},
      {GoomDrawables::DOTS, std::make_unique<GoomDotsFx>(fxHelper, smallBitmaps)},
      {GoomDrawables::IFS, std::make_unique<IfsDancersFx>(fxHelper, smallBitmaps)},
      {GoomDrawables::IMAGE, std::make_unique<ImageFx>(parallel, fxHelper, resourcesDirectory)},
      {GoomDrawables::L_SYSTEM, std::make_unique<LSystemFx>(fxHelper, resourcesDirectory)},
      {GoomDrawables::LINES, std::make_unique<LinesFx>(fxHelper, smallBitmaps)},
      {GoomDrawables::PARTICLES, std::make_unique<ParticlesFx>(fxHelper, smallBitmaps)},
      {GoomDrawables::RAINDROPS, std::make_unique<RaindropsFx>(fxHelper)},
      {GoomDrawables::SHAPES, std::make_unique<ShapesFx>(fxHelper)},
      {GoomDrawables::STARS, std::make_unique<FlyingStarsFx>(fxHelper, smallBitmaps)},
      {GoomDrawables::TENTACLES, std::make_unique<TentaclesFx>(fxHelper)},
      {GoomDrawables::TUBES, std::make_unique<TubesFx>(fxHelper, smallBitmaps)},
  }}};
}

auto AllStandardVisualFx::GetLinesFx() noexcept -> VISUAL_FX::LinesFx&
{
  return *dynamic_cast<LinesFx*>(m_drawablesMap[GoomDrawables::LINES].get());
}

auto AllStandardVisualFx::Start() -> void
{
  std::for_each(begin(m_drawablesMap), end(m_drawablesMap), [](auto& value) { value->Start(); });
}

auto AllStandardVisualFx::Finish() -> void
{
  std::for_each(begin(m_drawablesMap), end(m_drawablesMap), [](auto& value) { value->Finish(); });
}

auto AllStandardVisualFx::RefreshAllFx() -> void
{
  std::for_each(begin(m_currentGoomDrawables),
                end(m_currentGoomDrawables),
                [this](const auto currentlyDrawable)
                { m_drawablesMap[currentlyDrawable]->Refresh(); });
}

auto AllStandardVisualFx::SuspendFx() -> void
{
  std::for_each(begin(m_currentGoomDrawables),
                end(m_currentGoomDrawables),
                [this](const auto currentlyDrawable)
                { m_drawablesMap[currentlyDrawable]->Suspend(); });
}

auto AllStandardVisualFx::ResumeFx() -> void
{
  std::for_each(begin(m_currentGoomDrawables),
                end(m_currentGoomDrawables),
                [this](const auto currentlyDrawable)
                { m_drawablesMap[currentlyDrawable]->Resume(); });
}

auto AllStandardVisualFx::ChangeAllFxPixelBlenders(
    const IVisualFx::PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  std::for_each(begin(m_currentGoomDrawables),
                end(m_currentGoomDrawables),
                [this, &pixelBlenderParams](const auto currentlyDrawable)
                { m_drawablesMap[currentlyDrawable]->ChangePixelBlender(pixelBlenderParams); });
}

auto AllStandardVisualFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) -> void
{
  std::for_each(begin(m_currentGoomDrawables),
                end(m_currentGoomDrawables),
                [this, &zoomMidpoint](const auto currentlyDrawable)
                { m_drawablesMap[currentlyDrawable]->SetZoomMidpoint(zoomMidpoint); });
}

auto AllStandardVisualFx::GetActiveColorMapsNames() -> std::unordered_set<std::string>
{
  auto activeColorMapsNames = std::unordered_set<std::string>{};
  activeColorMapsNames.emplace("N/A");

  /**
  for (const auto& visualFx : m_drawablesMap)
  {
    for (const auto& colorMapsName : visualFx->GetCurrentColorMapsNames())
    {
      activeColorMapsNames.emplace(colorMapsName);
    }
  }
   **/

  return activeColorMapsNames;
}

auto AllStandardVisualFx::SetFrameMiscDataToStandardFx() -> void
{
  std::for_each(begin(m_currentGoomDrawables),
                end(m_currentGoomDrawables),
                [this](const auto& drawable)
                {
                  auto& visualFx = *m_drawablesMap[drawable];
                  visualFx.SetFrameMiscData(*m_frameMiscData);
                });
}

auto AllStandardVisualFx::ApplyStandardFxToImageBuffers(const AudioSamples& soundData) -> void
{
  std::for_each(begin(m_currentGoomDrawables),
                end(m_currentGoomDrawables),
                [this, &soundData](const auto& drawable)
                {
                  auto& visualFx = *m_drawablesMap[drawable];

                  visualFx.SetSoundData(soundData);

                  ResetDrawBuffSettings(drawable);
                  visualFx.ApplyToImageBuffers();
                });
}

auto AllStandardVisualFx::ApplyShaderFxToImageBuffers() -> void
{
  m_shaderFx->ApplyToImageBuffers();
}

auto AllStandardVisualFx::ApplyEndEffectIfNearEnd(const Stopwatch::TimeValues& timeValues) -> void
{
  m_shaderFx->ApplyEndEffect(timeValues);
}

auto AllStandardVisualFx::ChangeShaderVariables() -> void
{
  m_shaderFx->ChangeEffects();
}

auto AllStandardVisualFx::ChangeColorMaps() -> void
{
  m_visualFxColorMaps.ChangeRandomColorMaps();

  m_drawablesMap[GoomDrawables::CIRCLES]->SetWeightedColorMaps(
      {0,
       m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::CIRCLES_MAIN),
       m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::CIRCLES_LOW)});

  ChangeDotsColorMaps();

  m_drawablesMap[GoomDrawables::IFS]->SetWeightedColorMaps(
      {0, m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::IFS)});

  m_drawablesMap[GoomDrawables::IMAGE]->SetWeightedColorMaps(
      {0, m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::IMAGE)});

  m_drawablesMap[GoomDrawables::L_SYSTEM]->SetWeightedColorMaps(
      {0,
       m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::L_SYSTEM_MAIN),
       m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::L_SYSTEM_LOW)});

  ChangeLinesColorMaps();

  m_drawablesMap[GoomDrawables::PARTICLES]->SetWeightedColorMaps(
      {0,
       m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::PARTICLES_MAIN),
       m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::PARTICLES_LOW)});

  m_drawablesMap[GoomDrawables::RAINDROPS]->SetWeightedColorMaps(
      {0,
       m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::RAINDROPS_MAIN),
       m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::RAINDROPS_LOW)});

  ChangeShapesColorMaps();

  ChangeStarsColorMaps();

  ChangeTentaclesColorMaps();

  m_drawablesMap[GoomDrawables::TUBES]->SetWeightedColorMaps(
      {0,
       m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::TUBE_MAIN),
       m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::TUBE_LOW)});
}

auto AllStandardVisualFx::ChangeDotsColorMaps() noexcept -> void
{
  static constexpr auto EXPECTED_NUM_DOT_TYPES =
      1U + (static_cast<uint32_t>(GoomEffect::DOTS4) - static_cast<uint32_t>(GoomEffect::DOTS0));
  static_assert(GoomDotsFx::NUM_DOT_TYPES == EXPECTED_NUM_DOT_TYPES);

  for (auto i = 0U; i < GoomDotsFx::NUM_DOT_TYPES; ++i)
  {
    const auto dotEffect = static_cast<GoomEffect>(i + static_cast<uint32_t>(GoomEffect::DOTS0));
    m_drawablesMap[GoomDrawables::DOTS]->SetWeightedColorMaps(
        {i, m_visualFxColorMaps.GetCurrentRandomColorMaps(dotEffect)});
  }
}

auto AllStandardVisualFx::ChangeLinesColorMaps() noexcept -> void
{
  m_drawablesMap[GoomDrawables::LINES]->SetWeightedColorMaps(
      {0, m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::LINES1)});
  m_drawablesMap[GoomDrawables::LINES]->SetWeightedColorMaps(
      {1, m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::LINES2)});
}

auto AllStandardVisualFx::ChangeShapesColorMaps() noexcept -> void
{
  static constexpr auto EXPECTED_NUM_SHAPES = 1U;
  static_assert(ShapesFx::NUM_SHAPES == EXPECTED_NUM_SHAPES);

  for (auto i = 0U; i < ShapesFx::NUM_SHAPES; ++i)
  {
    const auto offsetFromZero = 3 * i;
    const auto goomEffectMain =
        static_cast<GoomEffect>(static_cast<uint32_t>(GoomEffect::SHAPES_MAIN) + offsetFromZero);
    const auto goomEffectLow =
        static_cast<GoomEffect>(static_cast<uint32_t>(GoomEffect::SHAPES_LOW) + offsetFromZero);
    const auto goomEffectInner =
        static_cast<GoomEffect>(static_cast<uint32_t>(GoomEffect::SHAPES_INNER) + offsetFromZero);

    m_drawablesMap[GoomDrawables::SHAPES]->SetWeightedColorMaps(
        {i,
         m_visualFxColorMaps.GetCurrentRandomColorMaps(goomEffectMain),
         m_visualFxColorMaps.GetCurrentRandomColorMaps(goomEffectLow),
         m_visualFxColorMaps.GetCurrentRandomColorMaps(goomEffectInner)});
  }
}

auto AllStandardVisualFx::ChangeStarsColorMaps() noexcept -> void
{
  static constexpr auto EXPECTED_NUM_STAR_MODES =
      1U + ((static_cast<uint32_t>(GoomEffect::STARS_LOW_FOUNTAIN) -
             static_cast<uint32_t>(GoomEffect::STARS_MAIN_FIREWORKS)) /
            2U);
  static_assert(StarTypesContainer::NUM_STAR_TYPES == EXPECTED_NUM_STAR_MODES);

  for (auto i = 0U; i < StarTypesContainer::NUM_STAR_TYPES; ++i)
  {
    const auto offsetFromZero = 3 * i;
    const auto goomEffectMain = static_cast<GoomEffect>(
        static_cast<uint32_t>(GoomEffect::STARS_MAIN_FIREWORKS) + offsetFromZero);
    const auto goomEffectLow = static_cast<GoomEffect>(
        static_cast<uint32_t>(GoomEffect::STARS_LOW_FIREWORKS) + offsetFromZero);

    m_drawablesMap[GoomDrawables::STARS]->SetWeightedColorMaps(
        {i,
         m_visualFxColorMaps.GetCurrentRandomColorMaps(goomEffectMain),
         m_visualFxColorMaps.GetCurrentRandomColorMaps(goomEffectLow)});
  }
}

auto AllStandardVisualFx::ChangeTentaclesColorMaps() noexcept -> void
{
  static constexpr auto EXPECTED_NUM_TENTACLE_MODES =
      1U + ((static_cast<uint32_t>(GoomEffect::TENTACLES_LOW) -
             static_cast<uint32_t>(GoomEffect::TENTACLES_DOMINANT_MAIN)) /
            2);
  static_assert(TentaclesFx::NUM_TENTACLE_COLOR_TYPES == EXPECTED_NUM_TENTACLE_MODES);

  for (auto i = 0U; i < TentaclesFx::NUM_TENTACLE_COLOR_TYPES; ++i)
  {
    const auto offsetFromZero = 2 * i;
    const auto goomEffectMain = static_cast<GoomEffect>(
        static_cast<uint32_t>(GoomEffect::TENTACLES_DOMINANT_MAIN) + offsetFromZero);
    const auto goomEffectLow = static_cast<GoomEffect>(
        static_cast<uint32_t>(GoomEffect::TENTACLES_DOMINANT_LOW) + offsetFromZero);

    m_drawablesMap[GoomDrawables::TENTACLES]->SetWeightedColorMaps(
        {i,
         m_visualFxColorMaps.GetCurrentRandomColorMaps(goomEffectMain),
         m_visualFxColorMaps.GetCurrentRandomColorMaps(goomEffectLow)});
  }
}

} // namespace GOOM::CONTROL
