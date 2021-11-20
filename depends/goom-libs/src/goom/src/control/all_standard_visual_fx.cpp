#include "all_standard_visual_fx.h"

#include "goom_plugin_info.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/parallel_utils.h"
#include "v2d.h"
#include "visual_fx/flying_stars_fx.h"
#include "visual_fx/fx_helpers.h"
#include "visual_fx/goom_dots_fx.h"
#include "visual_fx/ifs_dancers_fx.h"
#include "visual_fx/image_fx.h"
#include "visual_fx/shader_fx.h"
#include "visual_fx/tentacles_fx.h"
#include "visual_fx/tube_fx.h"
#include "visual_fx_color_maps.h"

#include <memory>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

using CONTROL::GoomDrawables;
using DRAW::IGoomDraw;
using UTILS::Parallel;
using UTILS::SmallImageBitmaps;
using VISUAL_FX::FlyingStarsFx;
using VISUAL_FX::FxHelpers;
using VISUAL_FX::GoomDotsFx;
using VISUAL_FX::IfsDancersFx;
using VISUAL_FX::ImageFx;
using VISUAL_FX::ShaderFx;
using VISUAL_FX::TentaclesFx;
using VISUAL_FX::TubeFx;

AllStandardVisualFx::AllStandardVisualFx(Parallel& parallel,
                                         const FxHelpers& fxHelpers,
                                         const SmallImageBitmaps& smallBitmaps,
                                         const std::string& resourcesDirectory) noexcept
  : m_goomDots_fx{std::make_shared<GoomDotsFx>(fxHelpers, smallBitmaps)},
    m_ifs_fx{std::make_shared<IfsDancersFx>(fxHelpers, smallBitmaps)},
    m_image_fx{std::make_shared<ImageFx>(parallel, fxHelpers, resourcesDirectory)},
    m_shader_fx{std::make_shared<ShaderFx>(fxHelpers)},
    m_star_fx{std::make_shared<FlyingStarsFx>(fxHelpers, smallBitmaps)},
    m_tentacles_fx{std::make_shared<TentaclesFx>(fxHelpers, smallBitmaps)},
    m_tube_fx{std::make_shared<TubeFx>(fxHelpers, smallBitmaps)},
    m_list{
        m_star_fx, m_ifs_fx, m_image_fx, m_goomDots_fx, m_tentacles_fx, m_tube_fx,
    },
    m_drawablesMap{
        {GoomDrawables::STARS, m_star_fx},          {GoomDrawables::IFS, m_ifs_fx},
        {GoomDrawables::IMAGE, m_image_fx},         {GoomDrawables::DOTS, m_goomDots_fx},
        {GoomDrawables::TENTACLES, m_tentacles_fx}, {GoomDrawables::TUBE, m_tube_fx},
    },
    m_visualFxColorMaps{fxHelpers.GetGoomRand()}
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
  for (auto& v : m_list)
  {
    v->Start();
  }
}

void AllStandardVisualFx::Finish()
{
  for (auto& v : m_list)
  {
    v->Finish();
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

void AllStandardVisualFx::SetZoomMidPoint(const V2dInt& zoomMidPoint)
{
  for (const auto& currentlyDrawable : m_currentGoomDrawables)
  {
    if (!CanDraw(currentlyDrawable))
    {
      continue;
    }
    m_drawablesMap.at(currentlyDrawable)->SetZoomMidPoint(zoomMidPoint);
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
  m_goomDots_fx->ApplySingle();
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
  m_goomDots_fx->ApplyMultiple();
}

void AllStandardVisualFx::ApplyIfsToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::IFS))
  {
    m_ifs_fx->ApplyNoDraw();
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::IFS);
  m_ifs_fx->ApplyMultiple();
}

void AllStandardVisualFx::ApplyImageToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::IMAGE))
  {
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::IMAGE);
  m_image_fx->ApplyMultiple();
}

void AllStandardVisualFx::ApplyShaderToBothBuffersIfRequired()
{
  ResetDrawBuffSettings(GoomDrawables::SHADER);
  m_shader_fx->ApplyMultiple();
}

void AllStandardVisualFx::StartShaderExposureControl()
{
  m_shader_fx->StartExposureControl();
}

void AllStandardVisualFx::SetAverageLuminance(const float value)
{
  m_shader_fx->SetAverageLuminance(value);
}

auto AllStandardVisualFx::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_shader_fx->GetLastShaderEffects();
}

void AllStandardVisualFx::ApplyTentaclesToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::TENTACLES))
  {
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::TENTACLES);
  m_tentacles_fx->ApplyMultiple();
}

void AllStandardVisualFx::ApplyTubeToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::TUBE))
  {
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::TUBE);
  m_tube_fx->ApplyMultiple();
}

void AllStandardVisualFx::ApplyStarsToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::STARS))
  {
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::STARS);
  m_star_fx->ApplyMultiple();
}

void AllStandardVisualFx::ChangeColorMaps()
{
  m_visualFxColorMaps.SetNextColorMapSet();

  m_goomDots_fx->SetWeightedColorMaps(0, m_visualFxColorMaps.GetColorMap(GoomEffect::DOTS0));
  m_goomDots_fx->SetWeightedColorMaps(1, m_visualFxColorMaps.GetColorMap(GoomEffect::DOTS1));
  m_goomDots_fx->SetWeightedColorMaps(2, m_visualFxColorMaps.GetColorMap(GoomEffect::DOTS2));
  m_goomDots_fx->SetWeightedColorMaps(3, m_visualFxColorMaps.GetColorMap(GoomEffect::DOTS3));
  m_goomDots_fx->SetWeightedColorMaps(4, m_visualFxColorMaps.GetColorMap(GoomEffect::DOTS4));

  m_ifs_fx->SetWeightedColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::IFS));

  m_image_fx->SetWeightedColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::IMAGE));

  m_star_fx->SetWeightedColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::STARS));
  m_star_fx->SetWeightedLowColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::STARS_LOW));

  m_tentacles_fx->SetWeightedColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::TENTACLES));

  m_tube_fx->SetWeightedColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::TUBE));
  m_tube_fx->SetWeightedLowColorMaps(m_visualFxColorMaps.GetColorMap(GoomEffect::TUBE_LOW));
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
