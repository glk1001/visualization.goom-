#include "all_standard_visual_fx.h"

#include "goom_config.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "utils/enumutils.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/stopwatch.h"
#include "visual_fx/circles_fx.h"
#include "visual_fx/flying_stars/star_types.h"
#include "visual_fx/flying_stars_fx.h"
#include "visual_fx/fx_helper.h"
#include "visual_fx/goom_dots_fx.h"
#include "visual_fx/ifs_dancers_fx.h"
#include "visual_fx/image_fx.h"
#include "visual_fx/lines_fx.h"
#include "visual_fx/shader_fx.h"
#include "visual_fx/shapes_fx.h"
#include "visual_fx/tentacles_fx.h"
#include "visual_fx/tubes_fx.h"
#include "visual_fx_color_maps.h"

#include <memory>

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
using VISUAL_FX::LinesFx;
using VISUAL_FX::ShaderFx;
using VISUAL_FX::ShapesFx;
using VISUAL_FX::TentaclesFx;
using VISUAL_FX::TubesFx;
using VISUAL_FX::FLYING_STARS::StarTypesContainer;

AllStandardVisualFx::AllStandardVisualFx(Parallel& parallel,
                                         const FxHelper& fxHelper,
                                         const SmallImageBitmaps& smallBitmaps,
                                         const std::string& resourcesDirectory) noexcept
  : m_shaderFx{std::make_unique<ShaderFx>(fxHelper)},
    m_drawablesMap{GetDrawablesMap(parallel, fxHelper, smallBitmaps, resourcesDirectory)},
    m_visualFxColorMaps{fxHelper.GetGoomRand()}
{
  Expects(NUM<GoomDrawables> == m_drawablesMap.size());
}

auto AllStandardVisualFx::GetDrawablesMap(Parallel& parallel,
                                          const FxHelper& fxHelper,
                                          const SmallImageBitmaps& smallBitmaps,
                                          const std::string& resourcesDirectory)
    -> std::map<GoomDrawables, PropagateConstUniquePtr>
{
  auto drawablesMap = std::map<GoomDrawables, PropagateConstUniquePtr>{};

  drawablesMap.emplace(GoomDrawables::CIRCLES, PropagateConstUniquePtr{std::make_unique<CirclesFx>(
                                                   fxHelper, smallBitmaps)});
  drawablesMap.emplace(GoomDrawables::DOTS, PropagateConstUniquePtr{std::make_unique<GoomDotsFx>(
                                                fxHelper, smallBitmaps)});
  drawablesMap.emplace(GoomDrawables::IFS, PropagateConstUniquePtr{std::make_unique<IfsDancersFx>(
                                               fxHelper, smallBitmaps)});
  drawablesMap.emplace(GoomDrawables::IMAGE, PropagateConstUniquePtr{std::make_unique<ImageFx>(
                                                 parallel, fxHelper, resourcesDirectory)});
  drawablesMap.emplace(GoomDrawables::LINES,
                       PropagateConstUniquePtr{std::make_unique<LinesFx>(fxHelper, smallBitmaps)});
  drawablesMap.emplace(GoomDrawables::SHAPES,
                       PropagateConstUniquePtr{std::make_unique<ShapesFx>(fxHelper)});
  drawablesMap.emplace(
      GoomDrawables::STARS,
      PropagateConstUniquePtr{std::make_unique<FlyingStarsFx>(fxHelper, smallBitmaps)});
  drawablesMap.emplace(
      GoomDrawables::TENTACLES,
      PropagateConstUniquePtr{std::make_unique<TentaclesFx>(fxHelper, smallBitmaps)});
  drawablesMap.emplace(GoomDrawables::TUBES,
                       PropagateConstUniquePtr{std::make_unique<TubesFx>(fxHelper, smallBitmaps)});

  return drawablesMap;
}

auto AllStandardVisualFx::GetLinesFx() noexcept -> VISUAL_FX::LinesFx&
{
  return *dynamic_cast<LinesFx*>(m_drawablesMap.at(GoomDrawables::LINES).get());
}

auto AllStandardVisualFx::SetSingleBufferDots(const bool val) -> void
{
  dynamic_cast<GoomDotsFx*>(m_drawablesMap.at(GoomDrawables::DOTS).get())->SetSingleBufferDots(val);
}

inline auto AllStandardVisualFx::IsCurrentlyDrawable(const GoomDrawables goomDrawable) const -> bool
{
#if __cplusplus <= 201703L
  return m_currentGoomDrawables.find(goomDrawable) != m_currentGoomDrawables.end();
#else
  return m_currentGoomDrawables.contains(goomDrawable);
#endif
}

auto AllStandardVisualFx::Start() -> void
{
  std::for_each(begin(m_drawablesMap), end(m_drawablesMap),
                [](auto& keyValue) { keyValue.second->Start(); });
}

auto AllStandardVisualFx::Finish() -> void
{
  std::for_each(begin(m_drawablesMap), end(m_drawablesMap),
                [](auto& keyValue) { keyValue.second->Finish(); });
}

auto AllStandardVisualFx::RefreshAllFx() -> void
{
  std::for_each(begin(m_currentGoomDrawables), end(m_currentGoomDrawables),
                [this](const auto currentlyDrawable)
                {
                  m_drawablesMap.at(currentlyDrawable)->Refresh();
                });
}

auto AllStandardVisualFx::SuspendFx() -> void
{
  std::for_each(begin(m_currentGoomDrawables), end(m_currentGoomDrawables),
                [this](const auto currentlyDrawable)
                {
                  m_drawablesMap.at(currentlyDrawable)->Suspend();
                });
}

auto AllStandardVisualFx::ResumeFx() -> void
{
  std::for_each(begin(m_currentGoomDrawables), end(m_currentGoomDrawables),
                [this](const auto currentlyDrawable)
                {
                  m_drawablesMap.at(currentlyDrawable)->Resume();
                });
}

auto AllStandardVisualFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) -> void
{
  std::for_each(begin(m_currentGoomDrawables), end(m_currentGoomDrawables),
                [this, &zoomMidpoint](const auto currentlyDrawable)
                {
                  m_drawablesMap.at(currentlyDrawable)->SetZoomMidpoint(zoomMidpoint);
                });
}

auto AllStandardVisualFx::PostStateUpdate(const GoomDrawablesSet& oldGoomDrawables) -> void
{
  std::for_each(begin(m_currentGoomDrawables), end(m_currentGoomDrawables),
                [this, &oldGoomDrawables](const auto currentlyDrawable)
                {
                  const auto wasActiveInPreviousState =
                      oldGoomDrawables.find(currentlyDrawable) != oldGoomDrawables.end();
                  m_drawablesMap.at(currentlyDrawable)->PostStateUpdate(wasActiveInPreviousState);
                });
}

auto AllStandardVisualFx::GetActiveColorMapsNames() const -> std::unordered_set<std::string>
{
  auto activeColorMapsNames = std::unordered_set<std::string>{};

  for (const auto& [drawable, visualFx] : m_drawablesMap)
  {
    for (const auto& colorMapsName : visualFx->GetCurrentColorMapsNames())
    {
      activeColorMapsNames.emplace(colorMapsName);
    }
  }

  return activeColorMapsNames;
}

auto AllStandardVisualFx::ApplyCurrentStateToSingleBuffer() -> void
{
  std::for_each(begin(m_drawablesMap), end(m_drawablesMap),
                [this](auto& keyValue)
                {
                  auto& visualFx = *m_drawablesMap.at(keyValue.first);

                  if (not IsCurrentlyDrawable(keyValue.first))
                  {
                    visualFx.ApplyNoDraw();
                    return;
                  }

                  ResetDrawBuffSettings(keyValue.first);
                  visualFx.ApplySingle();
                });
}

auto AllStandardVisualFx::ApplyStandardFxToMultipleBuffers(const AudioSamples& soundData) -> void
{
  std::for_each(begin(m_drawablesMap), end(m_drawablesMap),
                [this, &soundData](auto& keyValue)
                {
                  auto& visualFx = *m_drawablesMap.at(keyValue.first);

                  visualFx.SetSoundData(soundData);

                  if (not IsCurrentlyDrawable(keyValue.first))
                  {
                    visualFx.ApplyNoDraw();
                    return;
                  }

                  ResetDrawBuffSettings(keyValue.first);
                  visualFx.ApplyMultiple();
                });
}

auto AllStandardVisualFx::ApplyShaderToBothBuffersIfRequired() -> void
{
  m_shaderFx->ApplyMultiple();
}

auto AllStandardVisualFx::ApplyEndEffectIfNearEnd(const Stopwatch::TimeValues& timeValues) -> void
{
  m_shaderFx->ApplyEndEffect(timeValues);
}

auto AllStandardVisualFx::ChangeShaderEffects() -> void
{
  m_shaderFx->ChangeEffects();
}

auto AllStandardVisualFx::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_shaderFx->GetLastShaderEffects();
}

auto AllStandardVisualFx::ChangeColorMaps() -> void
{
  m_visualFxColorMaps.ChangeRandomColorMaps();

  m_drawablesMap.at(GoomDrawables::CIRCLES)
      ->SetWeightedColorMaps(
          {0, m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::CIRCLES_MAIN),
           m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::CIRCLES_LOW)});

  ChangeDotsColorMaps();

  m_drawablesMap.at(GoomDrawables::IFS)
      ->SetWeightedColorMaps({0, m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::IFS)});

  m_drawablesMap.at(GoomDrawables::IMAGE)
      ->SetWeightedColorMaps({0, m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::IMAGE)});

  ChangeLinesColorMaps();

  ChangeShapesColorMaps();

  ChangeStarsColorMaps();

  m_drawablesMap.at(GoomDrawables::TENTACLES)
      ->SetWeightedColorMaps(
          {0, m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::TENTACLES)});

  m_drawablesMap.at(GoomDrawables::TUBES)
      ->SetWeightedColorMaps({0,
                              m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::TUBE_MAIN),
                              m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::TUBE_LOW)});
}

auto AllStandardVisualFx::ChangeDotsColorMaps() noexcept -> void
{
  constexpr auto EXPECTED_NUM_DOT_TYPES =
      1U + (static_cast<uint32_t>(GoomEffect::DOTS4) - static_cast<uint32_t>(GoomEffect::DOTS0));
  static_assert(GoomDotsFx::NUM_DOT_TYPES == EXPECTED_NUM_DOT_TYPES);

  for (auto i = 0U; i < GoomDotsFx::NUM_DOT_TYPES; ++i)
  {
    const auto dotEffect = static_cast<GoomEffect>(i + static_cast<uint32_t>(GoomEffect::DOTS0));
    m_drawablesMap.at(GoomDrawables::DOTS)
        ->SetWeightedColorMaps({i, m_visualFxColorMaps.GetCurrentRandomColorMaps(dotEffect)});
  }
}

auto AllStandardVisualFx::ChangeLinesColorMaps() noexcept -> void
{
  m_drawablesMap.at(GoomDrawables::LINES)
      ->SetWeightedColorMaps(
          {0, m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::LINES1)});
  m_drawablesMap.at(GoomDrawables::LINES)
      ->SetWeightedColorMaps(
          {1, m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::LINES2)});
}

auto AllStandardVisualFx::ChangeShapesColorMaps() noexcept -> void
{
  constexpr auto EXPECTED_NUM_SHAPES = 1U + ((static_cast<uint32_t>(GoomEffect::SHAPES_MAIN) -
                                              static_cast<uint32_t>(GoomEffect::SHAPES_MAIN)) /
                                             3U);
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

    m_drawablesMap.at(GoomDrawables::SHAPES)
        ->SetWeightedColorMaps({i, m_visualFxColorMaps.GetCurrentRandomColorMaps(goomEffectMain),
                                m_visualFxColorMaps.GetCurrentRandomColorMaps(goomEffectLow),
                                m_visualFxColorMaps.GetCurrentRandomColorMaps(goomEffectInner)});
  }
}

auto AllStandardVisualFx::ChangeStarsColorMaps() noexcept -> void
{
  constexpr auto EXPECTED_NUM_STAR_MODES =
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

    m_drawablesMap.at(GoomDrawables::STARS)
        ->SetWeightedColorMaps({i, m_visualFxColorMaps.GetCurrentRandomColorMaps(goomEffectMain),
                                m_visualFxColorMaps.GetCurrentRandomColorMaps(goomEffectLow)});
  }
}

} // namespace GOOM::CONTROL
