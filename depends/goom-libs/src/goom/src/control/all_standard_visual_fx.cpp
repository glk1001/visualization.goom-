#include "all_standard_visual_fx.h"

#include "goom_plugin_info.h"
#include "point2d.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/parallel_utils.h"
#include "utils/stopwatch.h"
#include "visual_fx/circles_fx.h"
#include "visual_fx/flying_stars_fx.h"
#include "visual_fx/fx_helper.h"
#include "visual_fx/goom_dots_fx.h"
#include "visual_fx/ifs_dancers_fx.h"
#include "visual_fx/image_fx.h"
#include "visual_fx/shader_fx.h"
#include "visual_fx/shapes_fx.h"
#include "visual_fx/tentacles_fx.h"
#include "visual_fx/tubes_fx.h"
#include "visual_fx_color_maps.h"

#include <memory>

namespace GOOM::CONTROL
{

using CONTROL::GoomDrawables;
using UTILS::Parallel;
using UTILS::Stopwatch;
using UTILS::GRAPHICS::SmallImageBitmaps;
using VISUAL_FX::CirclesFx;
using VISUAL_FX::FlyingStarsFx;
using VISUAL_FX::FxHelper;
using VISUAL_FX::GoomDotsFx;
using VISUAL_FX::IfsDancersFx;
using VISUAL_FX::ImageFx;
using VISUAL_FX::ShaderFx;
using VISUAL_FX::ShapesFx;
using VISUAL_FX::TentaclesFx;
using VISUAL_FX::TubesFx;

AllStandardVisualFx::AllStandardVisualFx(Parallel& parallel,
                                         const FxHelper& fxHelper,
                                         const SmallImageBitmaps& smallBitmaps,
                                         const std::string& resourcesDirectory) noexcept
  : m_shaderFx{std::make_unique<ShaderFx>(fxHelper)},
    m_drawablesMap{GetDrawablesMap(parallel, fxHelper, smallBitmaps, resourcesDirectory)},
    m_visualFxColorMaps{fxHelper.GetGoomRand()}
{
}

auto AllStandardVisualFx::GetDrawablesMap(Parallel& parallel,
                                          const FxHelper& fxHelper,
                                          const SmallImageBitmaps& smallBitmaps,
                                          const std::string& resourcesDirectory)
    -> std::map<GoomDrawables, PropagateConstUniquePtr>
{
  std::map<GoomDrawables, PropagateConstUniquePtr> drawablesMap{};

  drawablesMap.emplace(GoomDrawables::CIRCLES, PropagateConstUniquePtr(std::make_unique<CirclesFx>(
                                                   fxHelper, smallBitmaps)));
  drawablesMap.emplace(GoomDrawables::DOTS, PropagateConstUniquePtr(std::make_unique<GoomDotsFx>(
                                                fxHelper, smallBitmaps)));
  drawablesMap.emplace(GoomDrawables::IFS, PropagateConstUniquePtr(std::make_unique<IfsDancersFx>(
                                               fxHelper, smallBitmaps)));
  drawablesMap.emplace(GoomDrawables::IMAGE, PropagateConstUniquePtr(std::make_unique<ImageFx>(
                                                 parallel, fxHelper, resourcesDirectory)));
  drawablesMap.emplace(GoomDrawables::SHAPES,
                       PropagateConstUniquePtr(std::make_unique<ShapesFx>(fxHelper)));
  drawablesMap.emplace(
      GoomDrawables::STARS,
      PropagateConstUniquePtr(std::make_unique<FlyingStarsFx>(fxHelper, smallBitmaps)));
  drawablesMap.emplace(
      GoomDrawables::TENTACLES,
      PropagateConstUniquePtr(std::make_unique<TentaclesFx>(fxHelper, smallBitmaps)));
  drawablesMap.emplace(GoomDrawables::TUBES,
                       PropagateConstUniquePtr(std::make_unique<TubesFx>(fxHelper, smallBitmaps)));

  return drawablesMap;
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
                  if (not CanDraw(currentlyDrawable))
                  {
                    return;
                  }
                  m_drawablesMap.at(currentlyDrawable)->Refresh();
                });
}

auto AllStandardVisualFx::SuspendFx() -> void
{
  std::for_each(begin(m_currentGoomDrawables), end(m_currentGoomDrawables),
                [this](const auto currentlyDrawable)
                {
                  if (not CanDraw(currentlyDrawable))
                  {
                    return;
                  }
                  m_drawablesMap.at(currentlyDrawable)->Suspend();
                });
}

auto AllStandardVisualFx::ResumeFx() -> void
{
  std::for_each(begin(m_currentGoomDrawables), end(m_currentGoomDrawables),
                [this](const auto currentlyDrawable)
                {
                  if (not CanDraw(currentlyDrawable))
                  {
                    return;
                  }
                  m_drawablesMap.at(currentlyDrawable)->Resume();
                });
}

auto AllStandardVisualFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) -> void
{
  std::for_each(begin(m_currentGoomDrawables), end(m_currentGoomDrawables),
                [this, &zoomMidpoint](const auto currentlyDrawable)
                {
                  if (not CanDraw(currentlyDrawable))
                  {
                    return;
                  }
                  m_drawablesMap.at(currentlyDrawable)->SetZoomMidpoint(zoomMidpoint);
                });
}

auto AllStandardVisualFx::PostStateUpdate(const GoomDrawablesSet& oldGoomDrawables) -> void
{
  std::for_each(begin(m_currentGoomDrawables), end(m_currentGoomDrawables),
                [this, &oldGoomDrawables](const auto currentlyDrawable)
                {
                  if (not CanDraw(currentlyDrawable))
                  {
                    return;
                  }
                  const bool wasActiveInPreviousState =
                      oldGoomDrawables.find(currentlyDrawable) != oldGoomDrawables.end();
                  m_drawablesMap.at(currentlyDrawable)->PostStateUpdate(wasActiveInPreviousState);
                });
}

auto AllStandardVisualFx::ApplyCurrentStateToSingleBuffer() -> void
{
  std::for_each(begin(m_drawablesMap), end(m_drawablesMap),
                [this](auto& keyValue)
                {
                  if (not IsCurrentlyDrawable(keyValue.first))
                  {
                    m_drawablesMap.at(keyValue.first)->ApplyNoDraw();
                    return;
                  }

                  ResetDrawBuffSettings(keyValue.first);
                  m_drawablesMap.at(keyValue.first)->ApplySingle();
                });
}

auto AllStandardVisualFx::ApplyStandardFxToMultipleBuffers() -> void
{
  std::for_each(begin(m_drawablesMap), end(m_drawablesMap),
                [this](auto& keyValue)
                {
                  if (not IsCurrentlyDrawable(keyValue.first))
                  {
                    m_drawablesMap.at(keyValue.first)->ApplyNoDraw();
                    return;
                  }

                  ResetDrawBuffSettings(keyValue.first);
                  m_drawablesMap.at(keyValue.first)->ApplyMultiple();
                });
}

auto AllStandardVisualFx::ApplyShaderToBothBuffersIfRequired() -> void
{
  ResetDrawBuffSettings(GoomDrawables::SHADER);
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
  m_visualFxColorMaps.SetNextColorMapSet();

  m_drawablesMap.at(GoomDrawables::CIRCLES)
      ->SetWeightedColorMaps({0, m_visualFxColorMaps.GetColorMap(GoomEffect::CIRCLES),
                              m_visualFxColorMaps.GetColorMap(GoomEffect::CIRCLES_LOW)});

  ChangeDotsColorMaps();

  m_drawablesMap.at(GoomDrawables::IFS)
      ->SetWeightedColorMaps({0, m_visualFxColorMaps.GetColorMap(GoomEffect::IFS)});

  m_drawablesMap.at(GoomDrawables::IMAGE)
      ->SetWeightedColorMaps({0, m_visualFxColorMaps.GetColorMap(GoomEffect::IMAGE)});

  ChangeShapesColorMaps();

  m_drawablesMap.at(GoomDrawables::STARS)
      ->SetWeightedColorMaps({0, m_visualFxColorMaps.GetColorMap(GoomEffect::STARS),
                              m_visualFxColorMaps.GetColorMap(GoomEffect::STARS_LOW)});

  m_drawablesMap.at(GoomDrawables::TENTACLES)
      ->SetWeightedColorMaps({0, m_visualFxColorMaps.GetColorMap(GoomEffect::TENTACLES)});

  m_drawablesMap.at(GoomDrawables::TUBES)
      ->SetWeightedColorMaps({0, m_visualFxColorMaps.GetColorMap(GoomEffect::TUBE),
                              m_visualFxColorMaps.GetColorMap(GoomEffect::TUBE_LOW)});
}

inline auto AllStandardVisualFx::ChangeDotsColorMaps() -> void
{
  static_assert(GoomDotsFx::NUM_DOT_TYPES == EXPECTED_NUM_DOT_TYPES);

  for (uint32_t i = 0; i < GoomDotsFx::NUM_DOT_TYPES; ++i)
  {
    const auto dotEffect = static_cast<GoomEffect>(i + static_cast<uint32_t>(GoomEffect::DOTS0));
    m_drawablesMap.at(GoomDrawables::DOTS)
        ->SetWeightedColorMaps({i, m_visualFxColorMaps.GetColorMap(dotEffect)});
  }
}

inline auto AllStandardVisualFx::ChangeShapesColorMaps() -> void
{
  static_assert(ShapesFx::NUM_SHAPES == EXPECTED_NUM_SHAPES);

  for (uint32_t i = 0; i < ShapesFx::NUM_SHAPES; ++i)
  {
    const uint32_t offsetFromZero = 3 * i;
    const auto goomEffectMain =
        static_cast<GoomEffect>(static_cast<uint32_t>(GoomEffect::SHAPES0_MAIN) + offsetFromZero);
    const auto goomEffectLow =
        static_cast<GoomEffect>(static_cast<uint32_t>(GoomEffect::SHAPES0_LOW) + offsetFromZero);
    const auto goomEffectInner =
        static_cast<GoomEffect>(static_cast<uint32_t>(GoomEffect::SHAPES0_INNER) + offsetFromZero);

    m_drawablesMap.at(GoomDrawables::SHAPES)
        ->SetWeightedColorMaps({i, m_visualFxColorMaps.GetColorMap(goomEffectMain),
                                m_visualFxColorMaps.GetColorMap(goomEffectLow),
                                m_visualFxColorMaps.GetColorMap(goomEffectInner)});
  }
}

} // namespace GOOM::CONTROL
