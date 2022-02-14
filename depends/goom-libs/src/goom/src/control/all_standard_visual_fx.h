#pragma once

#include "goom_states.h"
#include "point2d.h"
#include "visual_fx_color_maps.h"

#include <functional>
#include <map>
#include <memory>
#include <unordered_set>

namespace GOOM
{

namespace UTILS
{
class Parallel;
}
namespace UTILS::GRAPHICS
{
class SmallImageBitmaps;
}

namespace VISUAL_FX
{
class IVisualFx;
class FxHelpers;
class CirclesFx;
class FlyingStarsFx;
class GoomDotsFx;
class IfsDancersFx;
class ImageFx;
class ShaderFx;
class TentaclesFx;
class TubesFx;

class LinesFx;
} // namespace VISUAL_FX

namespace CONTROL
{

class AllStandardVisualFx
{
public:
  using GoomDrawablesSet = std::unordered_set<GoomDrawables>;

  AllStandardVisualFx(UTILS::Parallel& parallel,
                      const VISUAL_FX::FxHelpers& fxHelpers,
                      const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
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
  void SetZoomMidPoint(const Point2dInt& zoomMidPoint);
  void PostStateUpdate(const GoomDrawablesSet& oldGoomDrawables);

  void ApplyCurrentStateToSingleBuffer();
  void ApplyCurrentStateToMultipleBuffers();

  void ChangeShaderEffects();
  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

  [[nodiscard]] auto CanDraw(GoomDrawables goomDrawable) const -> bool;

private:
  const std::shared_ptr<VISUAL_FX::CirclesFx> m_circlesFx;
  const std::shared_ptr<VISUAL_FX::GoomDotsFx> m_goomDotsFx;
  const std::shared_ptr<VISUAL_FX::IfsDancersFx> m_ifsFx;
  const std::shared_ptr<VISUAL_FX::ImageFx> m_imageFx;
  const std::shared_ptr<VISUAL_FX::ShaderFx> m_shaderFx;
  const std::shared_ptr<VISUAL_FX::FlyingStarsFx> m_starFx;
  const std::shared_ptr<VISUAL_FX::TentaclesFx> m_tentaclesFx;
  const std::shared_ptr<VISUAL_FX::TubesFx> m_tubesFx;

  const std::vector<std::shared_ptr<VISUAL_FX::IVisualFx>> m_list;
  const std::map<GoomDrawables, std::shared_ptr<VISUAL_FX::IVisualFx>> m_drawablesMap;
  VisualFxColorMaps m_visualFxColorMaps;

  GoomDrawablesSet m_currentGoomDrawables{};
  [[nodiscard]] auto IsCurrentlyDrawable(GoomDrawables goomDrawable) const -> bool;
  ResetCurrentDrawBuffSettingsFunc m_resetCurrentDrawBuffSettingsFunc{};
  void ResetDrawBuffSettings(GoomDrawables fx);

  bool m_singleBufferDots = true;

  void ApplyCirclesToBothBuffersIfRequired();
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
  ApplyDotsIfRequired();
}

inline void AllStandardVisualFx::ApplyCurrentStateToMultipleBuffers()
{
  ApplyCirclesToBothBuffersIfRequired();
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
