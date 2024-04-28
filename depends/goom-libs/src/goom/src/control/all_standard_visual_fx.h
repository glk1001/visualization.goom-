#pragma once

#include "goom/frame_data.h"
#include "goom/goom_config.h"
#include "goom/point2d.h"
#include "goom_state_handler.h"
#include "goom_states.h"
#include "visual_fx_color_maps.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_set>

import Goom.Utils;
import Goom.VisualFx.VisualFxBase;
import Goom.VisualFx.LinesFx;
import Goom.VisualFx.ShaderFx;

namespace GOOM
{
class AudioSamples;

namespace UTILS
{

namespace GRAPHICS
{
class SmallImageBitmaps;
}
}

namespace CONTROL
{

class AllStandardVisualFx
{
public:
  AllStandardVisualFx(UTILS::Parallel& parallel,
                      VISUAL_FX::FxHelper& fxHelper,
                      const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
                      const std::string& resourcesDirectory) noexcept;

  [[nodiscard]] auto GetCurrentGoomDrawables() const -> const IGoomStateHandler::DrawablesState&;
  auto SetCurrentGoomDrawables(const IGoomStateHandler::DrawablesState& goomDrawablesSet) -> void;

  auto ChangeColorMaps() -> void;
  [[nodiscard]] static auto GetActiveColorMapsNames() -> std::unordered_set<std::string>;

  using ResetCurrentDrawBuffSettingsFunc = std::function<void(GoomDrawables fx)>;
  auto SetResetDrawBuffSettingsFunc(const ResetCurrentDrawBuffSettingsFunc& func) -> void;

  auto Start() -> void;
  auto Finish() -> void;

  auto RefreshAllFx() -> void;
  auto SuspendFx() -> void;
  auto ResumeFx() -> void;
  auto ChangeAllFxPixelBlenders(
      const VISUAL_FX::IVisualFx::PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) -> void;

  [[nodiscard]] auto GetFrameMiscData() const noexcept -> const MiscData&;
  auto SetFrameMiscData(MiscData& miscData) noexcept -> void;
  auto ApplyCurrentStateToImageBuffers(const AudioSamples& soundData) -> void;
  auto ApplyEndEffectIfNearEnd(const UTILS::Stopwatch::TimeValues& timeValues) -> void;

  [[nodiscard]] auto GetLinesFx() noexcept -> VISUAL_FX::LinesFx&;

  auto ChangeShaderVariables() -> void;

private:
  std::unique_ptr<VISUAL_FX::ShaderFx> m_shaderFx;
  UTILS::EnumMap<GoomDrawables, std::unique_ptr<VISUAL_FX::IVisualFx>> m_drawablesMap;
  [[nodiscard]] static auto GetDrawablesMap(UTILS::Parallel& parallel,
                                            VISUAL_FX::FxHelper& fxHelper,
                                            const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
                                            const std::string& resourcesDirectory)
      -> UTILS::EnumMap<GoomDrawables, std::unique_ptr<VISUAL_FX::IVisualFx>>;
  VisualFxColorMaps m_visualFxColorMaps;
  MiscData* m_frameMiscData = nullptr;
  auto ChangeDotsColorMaps() noexcept -> void;
  auto ChangeLinesColorMaps() noexcept -> void;
  auto ChangeShapesColorMaps() noexcept -> void;
  auto ChangeStarsColorMaps() noexcept -> void;
  auto ChangeTentaclesColorMaps() noexcept -> void;

  IGoomStateHandler::DrawablesState m_currentGoomDrawables{};
  ResetCurrentDrawBuffSettingsFunc m_resetCurrentDrawBuffSettingsFunc{};
  auto ResetDrawBuffSettings(GoomDrawables fx) -> void;

  auto SetFrameMiscDataToStandardFx() -> void;
  auto SetFrameMiscDataToShaderFx() -> void;

  auto ApplyStandardFxToImageBuffers(const AudioSamples& soundData) -> void;
  auto ApplyShaderFxToImageBuffers() -> void;
};

inline auto AllStandardVisualFx::GetCurrentGoomDrawables() const
    -> const IGoomStateHandler::DrawablesState&
{
  return m_currentGoomDrawables;
}

inline void AllStandardVisualFx::SetCurrentGoomDrawables(
    const IGoomStateHandler::DrawablesState& goomDrawablesSet)
{
  m_currentGoomDrawables = goomDrawablesSet;
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

inline auto AllStandardVisualFx::GetFrameMiscData() const noexcept -> const MiscData&
{
  Expects(m_frameMiscData != nullptr);

  return *m_frameMiscData;
}

inline auto AllStandardVisualFx::SetFrameMiscData(MiscData& miscData) noexcept -> void
{
  m_frameMiscData = &miscData;

  SetFrameMiscDataToStandardFx();
  SetFrameMiscDataToShaderFx();
}

inline auto AllStandardVisualFx::SetFrameMiscDataToShaderFx() -> void
{
  m_shaderFx->SetFrameMiscData(*m_frameMiscData);
}

inline auto AllStandardVisualFx::ApplyCurrentStateToImageBuffers(const AudioSamples& soundData)
    -> void
{
  ApplyStandardFxToImageBuffers(soundData);
  ApplyShaderFxToImageBuffers();
}

} // namespace CONTROL
} // namespace GOOM
