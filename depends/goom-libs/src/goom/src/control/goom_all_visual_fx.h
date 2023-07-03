#pragma once

#include "draw/goom_draw.h"
#include "filter_fx/filter_buffers_service.h"
#include "filter_fx/filter_settings.h"
#include "goom_config.h"
#include "goom_state_handler.h"
#include "goom_states.h"
#include "spimpl.h"
#include "utils/math/misc.h"
#include "utils/stopwatch.h"
#include "utils/t_values.h"
#include "visual_fx/goom_visual_fx.h"
#include "visual_fx_color_maps.h"

#include <functional>
#include <memory>
#include <string_view>
#include <unordered_set>

namespace GOOM
{
class AudioSamples;
class GoomLogger;
class PluginInfo;
class PixelBuffer;

namespace UTILS
{
class Parallel;
namespace GRAPHICS
{
class SmallImageBitmaps;
}
namespace MATH
{
class IGoomRand;
}
} // namespace UTILS

namespace VISUAL_FX
{
struct FxHelper;
}

namespace FILTER_FX
{
class FilterBuffersService;
struct ZoomFilterSettings;
}

namespace CONTROL
{

class AllStandardVisualFx;

class GoomAllVisualFx
{
public:
  GoomAllVisualFx() noexcept = delete;
  GoomAllVisualFx(UTILS::Parallel& parallel,
                  const VISUAL_FX::FxHelper& fxHelper,
                  const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
                  const std::string& resourcesDirectory,
                  IGoomStateHandler& goomStateHandler,
                  std::unique_ptr<FILTER_FX::FilterBuffersService> filterBuffersService) noexcept;

  auto Start() noexcept -> void;
  auto Finish() noexcept -> void;

  [[nodiscard]] auto IsTranBufferFltReady() const noexcept -> bool;
  auto CopyTranBufferFlt(std_spn::span<Point2dFlt>& destBuff) noexcept -> void;

  auto SetAllowMultiThreadedStates(bool val) noexcept -> void;

  auto SetNextState() noexcept -> void;
  [[nodiscard]] auto GetCurrentState() const noexcept -> GoomStates;
  [[nodiscard]] auto GetCurrentStateName() const noexcept -> std::string_view;

  [[nodiscard]] auto GetLastShaderVariables() const noexcept -> const GoomShaderVariables&;

  using ResetDrawBuffSettingsFunc = std::function<void(const FXBuffSettings& settings)>;
  auto SetResetDrawBuffSettingsFunc(const ResetDrawBuffSettingsFunc& func) noexcept -> void;

  auto ChangeAllFxColorMaps() noexcept -> void;
  auto ChangeAllFxPixelBlenders() noexcept -> void;
  auto RefreshAllFx() noexcept -> void;

  auto ApplyCurrentStateToMultipleBuffers(const AudioSamples& soundData) noexcept -> void;
  auto ApplyEndEffectIfNearEnd(const UTILS::Stopwatch::TimeValues& timeValues) noexcept -> void;

  auto UpdateFilterSettings(const FILTER_FX::ZoomFilterSettings& filterSettings) noexcept -> void;
  auto UpdateZoomBuffers() noexcept -> void;
  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> uint32_t;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::unordered_set<std::string>;
  [[nodiscard]] auto GetZoomFilterFxNameValueParams() const noexcept -> UTILS::NameValuePairs;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  [[maybe_unused]] GoomLogger* m_goomLogger;
  spimpl::unique_impl_ptr<AllStandardVisualFx> m_allStandardVisualFx;
  std::unique_ptr<FILTER_FX::FilterBuffersService> m_filterBuffersService;

  IGoomStateHandler* m_goomStateHandler;
  bool m_allowMultiThreadedStates = true;
  auto ChangeState() noexcept -> void;
  IGoomStateHandler::DrawablesState m_currentGoomDrawables{};

  ResetDrawBuffSettingsFunc m_resetDrawBuffSettings{};
  auto ResetCurrentDrawBuffSettings(GoomDrawables fx) noexcept -> void;
  [[nodiscard]] auto GetCurrentBuffSettings(GoomDrawables fx) const noexcept -> FXBuffSettings;

  VisualFxColorMaps m_visualFxColorMaps{*m_goomRand};

  [[nodiscard]] auto GetNextPixelBlenderParams() const noexcept
      -> VISUAL_FX::IVisualFx::PixelBlenderParams;
  enum class GlobalBlendType
  {
    NONRANDOM,
    ASYNC_RANDOM,
    SYNC_RANDOM,
    _num // unused, and marks the enum end
  };
  static constexpr auto NONRANDOM_WEIGHT    = 50.0F;
  static constexpr auto SYNC_RANDOM_WEIGHT  = 50.0F;
  static constexpr auto ASYNC_RANDOM_WEIGHT = 50.0F;
  UTILS::MATH::Weights<GlobalBlendType> m_globalBlendTypeWeight{
      *m_goomRand,
      {{GlobalBlendType::NONRANDOM, NONRANDOM_WEIGHT},
                  {GlobalBlendType::SYNC_RANDOM, SYNC_RANDOM_WEIGHT},
                  {GlobalBlendType::ASYNC_RANDOM, ASYNC_RANDOM_WEIGHT}}
  };
};


inline auto GoomAllVisualFx::IsTranBufferFltReady() const noexcept -> bool
{
  return m_filterBuffersService->IsTranBufferFltReady();
}

inline auto GoomAllVisualFx::CopyTranBufferFlt(std_spn::span<Point2dFlt>& destBuff) noexcept -> void
{
  m_filterBuffersService->CopyTranBufferFlt(destBuff);
}

inline auto GoomAllVisualFx::SetAllowMultiThreadedStates(const bool val) noexcept -> void
{
  m_allowMultiThreadedStates = val;
}

inline auto GoomAllVisualFx::SetNextState() noexcept -> void
{
  ChangeState();
  ChangeAllFxColorMaps();
  ChangeAllFxPixelBlenders();
}

inline auto GoomAllVisualFx::SetResetDrawBuffSettingsFunc(
    const ResetDrawBuffSettingsFunc& func) noexcept -> void
{
  m_resetDrawBuffSettings = func;
}

inline auto GoomAllVisualFx::UpdateZoomBuffers() noexcept -> void
{
  m_filterBuffersService->UpdateZoomBuffers();
}

inline auto GoomAllVisualFx::GetTranLerpFactor() const noexcept -> uint32_t
{
  return m_filterBuffersService->GetTranLerpFactor();
}

inline auto GoomAllVisualFx::GetCurrentState() const noexcept -> GoomStates
{
  return m_goomStateHandler->GetCurrentState();
}

inline auto GoomAllVisualFx::GetCurrentStateName() const noexcept -> std::string_view
{
  return GoomStateInfo::GetStateInfo(m_goomStateHandler->GetCurrentState()).name;
}

} // namespace CONTROL
} // namespace GOOM
