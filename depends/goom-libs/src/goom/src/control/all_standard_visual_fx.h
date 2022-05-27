#pragma once

#include "goom_graphic.h"
#include "goom_states.h"
#include "point2d.h"
#include "utils/propagate_const.h"
#include "utils/stopwatch.h"
#include "visual_fx/shader_fx.h"
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
class ShaderFx;
}

namespace CONTROL
{

class AllStandardVisualFx
{
public:
  using GoomDrawablesSet = std::unordered_set<GoomDrawables>;

  AllStandardVisualFx(UTILS::Parallel& parallel,
                      const VISUAL_FX::FxHelper& fxHelper,
                      const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
                      const std::string& resourcesDirectory) noexcept;

  [[nodiscard]] auto GetCurrentGoomDrawables() const -> const GoomDrawablesSet&;
  auto SetCurrentGoomDrawables(const GoomDrawablesSet& goomDrawablesSet) -> void;

  auto ChangeColorMaps() -> void;

  using ResetCurrentDrawBuffSettingsFunc = std::function<void(GoomDrawables fx)>;
  auto SetResetDrawBuffSettingsFunc(const ResetCurrentDrawBuffSettingsFunc& func) -> void;

  auto SetSingleBufferDots(bool val) -> void;

  auto Start() -> void;
  auto Finish() -> void;

  auto RefreshAllFx() -> void;
  auto SuspendFx() -> void;
  auto ResumeFx() -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) -> void;
  auto PostStateUpdate(const GoomDrawablesSet& oldGoomDrawables) -> void;

  auto ApplyCurrentStateToSingleBuffer() -> void;
  auto ApplyCurrentStateToMultipleBuffers() -> void;
  auto ApplyEndEffectIfNearEnd(const UTILS::Stopwatch::TimeValues& timeValues) -> void;

  auto ChangeShaderEffects() -> void;
  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

  [[nodiscard]] auto CanDraw(GoomDrawables goomDrawable) const -> bool;

private:
  std::experimental::propagate_const<std::unique_ptr<VISUAL_FX::ShaderFx>> m_shaderFx;
  using PropagateConstUniquePtr =
      std::experimental::propagate_const<std::unique_ptr<VISUAL_FX::IVisualFx>>;
  std::map<GoomDrawables, PropagateConstUniquePtr> m_drawablesMap;
  [[nodiscard]] static auto GetDrawablesMap(UTILS::Parallel& parallel,
                                            const VISUAL_FX::FxHelper& fxHelper,
                                            const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
                                            const std::string& resourcesDirectory)
      -> std::map<GoomDrawables, PropagateConstUniquePtr>;
  VisualFxColorMaps m_visualFxColorMaps;
  auto ChangeDotsColorMaps() -> void;
  auto ChangeShapesColorMaps() -> void;

  GoomDrawablesSet m_currentGoomDrawables{};
  [[nodiscard]] auto IsCurrentlyDrawable(GoomDrawables goomDrawable) const -> bool;
  ResetCurrentDrawBuffSettingsFunc m_resetCurrentDrawBuffSettingsFunc{};
  auto ResetDrawBuffSettings(GoomDrawables fx) -> void;

  auto ApplyStandardFxToMultipleBuffers() -> void;
  auto ApplyShaderToBothBuffersIfRequired() -> void;
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

inline auto AllStandardVisualFx::ApplyCurrentStateToMultipleBuffers() -> void
{
  ApplyStandardFxToMultipleBuffers();
  ApplyShaderToBothBuffersIfRequired();
}

} // namespace CONTROL
} // namespace GOOM
