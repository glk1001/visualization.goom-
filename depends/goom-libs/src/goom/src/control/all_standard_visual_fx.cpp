#include "all_standard_visual_fx.h"

#include "goom_plugin_info.h"
#include "point2d.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/parallel_utils.h"
#include "visual_fx/circles_fx.h"
#include "visual_fx/flying_stars_fx.h"
#include "visual_fx/fx_helper.h"
#include "visual_fx/goom_dots_fx.h"
#include "visual_fx/ifs_dancers_fx.h"
#include "visual_fx/image_fx.h"
#include "visual_fx/shader_fx.h"
#include "visual_fx/tentacles_fx.h"
#include "visual_fx/tubes_fx.h"
#include "visual_fx_color_maps.h"

#include <memory>

namespace GOOM::CONTROL
{

using CONTROL::GoomDrawables;
using UTILS::Parallel;
using UTILS::GRAPHICS::SmallImageBitmaps;
using VISUAL_FX::CirclesFx;
using VISUAL_FX::FlyingStarsFx;
using VISUAL_FX::FxHelper;
using VISUAL_FX::GoomDotsFx;
using VISUAL_FX::IfsDancersFx;
using VISUAL_FX::ImageFx;
using VISUAL_FX::ShaderFx;
using VISUAL_FX::TentaclesFx;
using VISUAL_FX::TubesFx;

AllStandardVisualFx::AllStandardVisualFx(Parallel& parallel,
                                         const FxHelper& fxHelper,
                                         const SmallImageBitmaps& smallBitmaps,
                                         const std::string& resourcesDirectory) noexcept
  : m_circlesFx{std::make_shared<CirclesFx>(fxHelper, smallBitmaps)},
    m_goomDotsFx{std::make_shared<GoomDotsFx>(fxHelper, smallBitmaps)},
    m_ifsFx{std::make_shared<IfsDancersFx>(fxHelper, smallBitmaps)},
    m_imageFx{std::make_shared<ImageFx>(parallel, fxHelper, resourcesDirectory)},
    m_shaderFx{std::make_shared<ShaderFx>(fxHelper)},
    m_starFx{std::make_shared<FlyingStarsFx>(fxHelper, smallBitmaps)},
    m_tentaclesFx{std::make_shared<TentaclesFx>(fxHelper, smallBitmaps)},
    m_tubesFx{std::make_shared<TubesFx>(fxHelper, smallBitmaps)},
    m_list{
        m_circlesFx, m_starFx, m_ifsFx, m_imageFx, m_goomDotsFx, m_tentaclesFx, m_tubesFx,
    },
    m_drawablesMap{
        {GoomDrawables::CIRCLES, m_circlesFx},
        {GoomDrawables::STARS, m_starFx},          {GoomDrawables::IFS, m_ifsFx},
        {GoomDrawables::IMAGE, m_imageFx},         {GoomDrawables::DOTS, m_goomDotsFx},
        {GoomDrawables::TENTACLES, m_tentaclesFx}, {GoomDrawables::TUBES, m_tubesFx},
    },
    m_visualFxColorMaps{fxHelper.GetGoomRand()}
{
}

inline auto AllStandardVisualFx::IsCurrentlyDrawable(const GoomDrawables goomDrawable) const -> bool
{
#if __cplusplus <= 201703L
  return m_currentGoomDrawables.find(goomDrawable) != m_currentGoomDrawables.end();
#else
  return m_currentGoomDrawables.contains(goomDrawable);
#endif
}

void AllStandardVisualFx::Start()
{
  for (const auto& visualFx : m_list)
  {
    visualFx->Start();
  }
}

void AllStandardVisualFx::Finish()
{
  for (const auto& visualFx : m_list)
  {
    visualFx->Finish();
  }
}

void AllStandardVisualFx::RefreshAllFx()
{
  for (const auto& currentlyDrawable : m_currentGoomDrawables)
  {
    if (!CanDraw(currentlyDrawable))
    {
      continue;
    }
    m_drawablesMap.at(currentlyDrawable)->Refresh();
  }
}

void AllStandardVisualFx::SuspendFx()
{
  for (const auto& currentlyDrawable : m_currentGoomDrawables)
  {
    if (!CanDraw(currentlyDrawable))
    {
      continue;
    }
    m_drawablesMap.at(currentlyDrawable)->Suspend();
  }
}

void AllStandardVisualFx::ResumeFx()
{
  for (const auto& currentlyDrawable : m_currentGoomDrawables)
  {
    if (!CanDraw(currentlyDrawable))
    {
      continue;
    }
    m_drawablesMap.at(currentlyDrawable)->Resume();
  }
}

void AllStandardVisualFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint)
{
  for (const auto& currentlyDrawable : m_currentGoomDrawables)
  {
    if (!CanDraw(currentlyDrawable))
    {
      continue;
    }
    m_drawablesMap.at(currentlyDrawable)->SetZoomMidpoint(zoomMidpoint);
  }
}

void AllStandardVisualFx::PostStateUpdate(const GoomDrawablesSet& oldGoomDrawables)
{
  for (const auto& currentlyDrawable : m_currentGoomDrawables)
  {
    if (!CanDraw(currentlyDrawable))
    {
      continue;
    }
    const bool wasActiveInPreviousState =
        oldGoomDrawables.find(GoomDrawables::IFS) != oldGoomDrawables.end();
    m_drawablesMap.at(currentlyDrawable)->PostStateUpdate(wasActiveInPreviousState);
  }
}

void AllStandardVisualFx::ApplyDotsIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::DOTS))
  {
    return;
  }
  if (!m_singleBufferDots)
  {
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::DOTS);
  m_goomDotsFx->ApplySingle();
}

void AllStandardVisualFx::ApplyDotsToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::DOTS))
  {
    return;
  }
  if (m_singleBufferDots)
  {
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::DOTS);
  m_goomDotsFx->ApplyMultiple();
}

void AllStandardVisualFx::ApplyIfsToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::IFS))
  {
    m_ifsFx->ApplyNoDraw();
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::IFS);
  m_ifsFx->ApplyMultiple();
}

void AllStandardVisualFx::ApplyCirclesToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::CIRCLES))
  {
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::CIRCLES);
  m_circlesFx->ApplyMultiple();
}

void AllStandardVisualFx::ApplyImageToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::IMAGE))
  {
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::IMAGE);
  m_imageFx->ApplyMultiple();
}

void AllStandardVisualFx::ApplyShaderToBothBuffersIfRequired()
{
  ResetDrawBuffSettings(GoomDrawables::SHADER);
  m_shaderFx->ApplyMultiple();
}

void AllStandardVisualFx::ChangeShaderEffects()
{
  m_shaderFx->ChangeEffects();
}

auto AllStandardVisualFx::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_shaderFx->GetLastShaderEffects();
}

void AllStandardVisualFx::ApplyTentaclesToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::TENTACLES))
  {
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::TENTACLES);
  m_tentaclesFx->ApplyMultiple();
}

void AllStandardVisualFx::ApplyTubeToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::TUBES))
  {
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::TUBES);
  m_tubesFx->ApplyMultiple();
}

void AllStandardVisualFx::ApplyStarsToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::STARS))
  {
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::STARS);
  m_starFx->ApplyMultiple();
}

void AllStandardVisualFx::ChangeColorMaps()
{
  m_visualFxColorMaps.SetNextColorMapSet();

  m_circlesFx->SetWeightedColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::CIRCLES),
                                    m_visualFxColorMaps.GetColorMap(GoomEffect::CIRCLES_LOW));

  m_goomDotsFx->SetWeightedColorMaps(0, m_visualFxColorMaps.GetColorMap(GoomEffect::DOTS0));
  m_goomDotsFx->SetWeightedColorMaps(1, m_visualFxColorMaps.GetColorMap(GoomEffect::DOTS1));
  m_goomDotsFx->SetWeightedColorMaps(2, m_visualFxColorMaps.GetColorMap(GoomEffect::DOTS2));
  m_goomDotsFx->SetWeightedColorMaps(3, m_visualFxColorMaps.GetColorMap(GoomEffect::DOTS3));
  m_goomDotsFx->SetWeightedColorMaps(4, m_visualFxColorMaps.GetColorMap(GoomEffect::DOTS4));

  m_ifsFx->SetWeightedColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::IFS));

  m_imageFx->SetWeightedColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::IMAGE));

  m_starFx->SetWeightedColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::STARS));
  m_starFx->SetWeightedLowColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::STARS_LOW));

  m_tentaclesFx->SetWeightedColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::TENTACLES));

  m_tubesFx->SetWeightedColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::TUBE));
  m_tubesFx->SetWeightedLowColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::TUBE_LOW));
}

} // namespace GOOM::CONTROL
