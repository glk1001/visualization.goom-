#include "all_standard_visual_fx.h"

#include "goom_plugin_info.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/parallel_utils.h"
#include "v2d.h"
#include "visual_fx/flying_stars_fx.h"
#include "visual_fx/goom_dots_fx.h"
#include "visual_fx/ifs_dancers_fx.h"
#include "visual_fx/image_fx.h"
#include "visual_fx/tentacles_fx.h"
#include "visual_fx/tube_fx.h"

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
using VISUAL_FX::GoomDotsFx;
using VISUAL_FX::IfsDancersFx;
using VISUAL_FX::ImageFx;
using VISUAL_FX::TentaclesFx;
using VISUAL_FX::TubeFx;

AllStandardVisualFx::AllStandardVisualFx(Parallel& parallel,
                                         IGoomDraw& draw,
                                         const PluginInfo& goomInfo,
                                         const SmallImageBitmaps& smallBitmaps,
                                         const std::string& resourcesDirectory) noexcept
  : m_goomDots_fx{std::make_shared<GoomDotsFx>(draw, goomInfo, smallBitmaps)},
    m_ifs_fx{std::make_shared<IfsDancersFx>(draw, goomInfo, smallBitmaps)},
    m_image_fx{std::make_shared<ImageFx>(parallel, draw, goomInfo, resourcesDirectory)},
    m_star_fx{std::make_shared<FlyingStarsFx>(draw, goomInfo, smallBitmaps)},
    m_tentacles_fx{std::make_shared<TentaclesFx>(draw, goomInfo)},
    m_tube_fx{std::make_shared<TubeFx>(draw, goomInfo, smallBitmaps)},
    m_list{
        m_star_fx, m_ifs_fx, m_image_fx, m_goomDots_fx, m_tentacles_fx, m_tube_fx,
    },
    m_drawablesMap{
        {GoomDrawables::STARS, m_star_fx},          {GoomDrawables::IFS, m_ifs_fx},
        {GoomDrawables::IMAGE, m_image_fx},         {GoomDrawables::DOTS, m_goomDots_fx},
        {GoomDrawables::TENTACLES, m_tentacles_fx}, {GoomDrawables::TUBE, m_tube_fx},
    }
{
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

void AllStandardVisualFx::ApplyTentaclesToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::TENTACLES))
  {
    m_tentacles_fx->ApplyNoDraw();
    return;
  }

  ResetDrawBuffSettings(GoomDrawables::TENTACLES);
  m_tentacles_fx->ApplyMultiple();
}

void AllStandardVisualFx::ApplyTubeToBothBuffersIfRequired()
{
  if (!IsCurrentlyDrawable(GoomDrawables::TUBE))
  {
    m_tube_fx->ApplyNoDraw();
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

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
