#pragma once

#include "goom_states.h"
#include "v2d.h"
#include "visual_fx_color_maps.h"

#include <functional>
#include <map>
#include <memory>
#include <unordered_set>

namespace GOOM
{
class PluginInfo;

namespace UTILS
{
class Parallel;
class SmallImageBitmaps;
} // namespace UTILS

namespace DRAW
{
class IGoomDraw;
} // namespace DRAW

namespace VISUAL_FX
{
class IVisualFx;
class FlyingStarsFx;
class GoomDotsFx;
class IfsDancersFx;
class ImageFx;
class ShaderFx;
class TentaclesFx;
class TubeFx;

class LinesFx;
} // namespace VISUAL_FX

namespace CONTROL
{

class AllStandardVisualFx
{
public:
  using GoomDrawablesSet = std::unordered_set<GoomDrawables>;

  AllStandardVisualFx(UTILS::Parallel& parallel,
                      DRAW::IGoomDraw& draw,
                      const PluginInfo& goomInfo,
                      const UTILS::SmallImageBitmaps& smallBitmaps,
                      const std::string& resourcesDirectory) noexcept;

  [[nodiscard]] auto GetCurrentGoomDrawables() const -> const GoomDrawablesSet&;
  void SetCurrentGoomDrawables(const GoomDrawablesSet& goomDrawablesSet);

  void ChangeColorMaps();

  using ResetCurrentDrawBuffSettingsFunc = std::function<void(GoomDrawables fx)>;
  void SetResetDrawBuffSettingsFunc(const ResetCurrentDrawBuffSettingsFunc& func);

  void SetSingleBufferDots(bool value);

  void Start();
  void Finish();

  void RefreshAllFx();
  void SuspendFx();
  void ResumeFx();
  void SetZoomMidPoint(const V2dInt& zoomMidPoint);
  void PostStateUpdate(const GoomDrawablesSet& oldGoomDrawables);

  void ApplyCurrentStateToSingleBuffer();
  void ApplyCurrentStateToMultipleBuffers();

  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

  [[nodiscard]] auto CanDraw(GoomDrawables goomDrawable) const -> bool;

private:
  const std::shared_ptr<VISUAL_FX::GoomDotsFx> m_goomDots_fx;
  const std::shared_ptr<VISUAL_FX::IfsDancersFx> m_ifs_fx;
  const std::shared_ptr<VISUAL_FX::ImageFx> m_image_fx;
  const std::shared_ptr<VISUAL_FX::ShaderFx> m_shader_fx;
  const std::shared_ptr<VISUAL_FX::FlyingStarsFx> m_star_fx;
  const std::shared_ptr<VISUAL_FX::TentaclesFx> m_tentacles_fx;
  const std::shared_ptr<VISUAL_FX::TubeFx> m_tube_fx;

  const std::vector<std::shared_ptr<VISUAL_FX::IVisualFx>> m_list;
  const std::map<GoomDrawables, std::shared_ptr<VISUAL_FX::IVisualFx>> m_drawablesMap;
  VisualFxColorMaps m_visualFxColorMaps{};

  GoomDrawablesSet m_currentGoomDrawables{};
  [[nodiscard]] auto IsCurrentlyDrawable(GoomDrawables goomDrawable) const -> bool;
  ResetCurrentDrawBuffSettingsFunc m_resetCurrentDrawBuffSettingsFunc{};
  void ResetDrawBuffSettings(GoomDrawables fx);

  bool m_singleBufferDots = true;

  void ApplyDotsIfRequired();
  void ApplyDotsToBothBuffersIfRequired();
  void ApplyIfsToBothBuffersIfRequired();
  void ApplyImageToBothBuffersIfRequired();
  void ApplyShaderToBothBuffersIfRequired();
  void ApplyTentaclesToBothBuffersIfRequired();
  void ApplyStarsToBothBuffersIfRequired();
  void ApplyTubeToBothBuffersIfRequired();
};

inline auto AllStandardVisualFx::GetCurrentGoomDrawables() const -> const GoomDrawablesSet&
{
  return m_currentGoomDrawables;
}

inline void AllStandardVisualFx::SetCurrentGoomDrawables(const GoomDrawablesSet& goomDrawablesSet)
{
  m_currentGoomDrawables = goomDrawablesSet;
}

inline auto AllStandardVisualFx::CanDraw(const GoomDrawables goomDrawable) const -> bool
{
  return m_drawablesMap.find(goomDrawable) != m_drawablesMap.end();
}

inline void AllStandardVisualFx::SetResetDrawBuffSettingsFunc(
    const ResetCurrentDrawBuffSettingsFunc& func)
{
  m_resetCurrentDrawBuffSettingsFunc = func;
}

inline void AllStandardVisualFx::ResetDrawBuffSettings(const GoomDrawables fx)
{
  m_resetCurrentDrawBuffSettingsFunc(fx);
}

inline void AllStandardVisualFx::SetSingleBufferDots(const bool value)
{
  m_singleBufferDots = value;
}

inline void AllStandardVisualFx::ApplyCurrentStateToSingleBuffer()
{
  // applyIfsIfRequired();
  ApplyDotsIfRequired();
}

inline void AllStandardVisualFx::ApplyCurrentStateToMultipleBuffers()
{
  ApplyDotsToBothBuffersIfRequired();
  ApplyIfsToBothBuffersIfRequired();
  ApplyImageToBothBuffersIfRequired();
  ApplyTentaclesToBothBuffersIfRequired();
  ApplyStarsToBothBuffersIfRequired();
  ApplyTubeToBothBuffersIfRequired();
  ApplyShaderToBothBuffersIfRequired();
}

} // namespace CONTROL
} // namespace GOOM
