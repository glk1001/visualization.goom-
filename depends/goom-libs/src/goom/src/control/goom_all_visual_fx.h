#pragma once

#include "draw/goom_draw.h"
#include "filter_fx/filter_buffer_color_info.h"
#include "filter_fx/filter_settings.h"
#include "filter_fx/zoom_filter_fx.h"
#include "goom_config.h"
#include "goom_state_handler.h"
#include "goom_states.h"
#include "spimpl.h"
#include "utils/adaptive_exposure.h"
#include "utils/math/misc.h"
#include "utils/propagate_const.h"
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
class FilterColorsService;
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
                  std::unique_ptr<FILTER_FX::FilterBuffersService> filterBuffersService,
                  std::unique_ptr<FILTER_FX::FilterColorsService> filterColorsService) noexcept;

  auto Start() noexcept -> void;
  auto Finish() noexcept -> void;

  auto SetTranBufferDest(const std_spn::span<Point2dFlt>& tranBufferFlt) noexcept -> void;
  auto SetAllowMultiThreadedStates(bool val) noexcept -> void;

  [[nodiscard]] auto GetZoomFilterFx() const noexcept -> const FILTER_FX::ZoomFilterFx&;

  auto SetNextState() noexcept -> void;
  [[nodiscard]] auto GetCurrentState() const noexcept -> GoomStates;
  [[nodiscard]] auto GetCurrentStateName() const noexcept -> std::string_view;

  auto StartExposureControl() noexcept -> void;
  [[nodiscard]] auto GetCurrentExposure() const noexcept -> float;
  [[nodiscard]] auto GetLastShaderVariables() const noexcept -> const GoomShaderVariables&;

  using ResetDrawBuffSettingsFunc = std::function<void(const FXBuffSettings& settings)>;
  auto SetResetDrawBuffSettingsFunc(const ResetDrawBuffSettingsFunc& func) noexcept -> void;

  auto ChangeAllFxColorMaps() noexcept -> void;
  auto ChangeAllFxPixelBlenders() noexcept -> void;
  auto RefreshAllFx() noexcept -> void;

  auto ApplyCurrentStateToMultipleBuffers(const AudioSamples& soundData) noexcept -> void;
  auto ApplyEndEffectIfNearEnd(const UTILS::Stopwatch::TimeValues& timeValues) noexcept -> void;

  auto UpdateFilterSettings(const FILTER_FX::ZoomFilterSettings& filterSettings) noexcept -> void;
  auto ApplyZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff) noexcept -> void;
  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> uint32_t;

  [[nodiscard]] auto IsFilterPosDataReady() const noexcept -> bool;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::unordered_set<std::string>;
  [[nodiscard]] auto GetZoomFilterFxNameValueParams() const noexcept -> UTILS::NameValuePairs;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  [[maybe_unused]] GoomLogger* m_goomLogger;
  spimpl::unique_impl_ptr<AllStandardVisualFx> m_allStandardVisualFx;
  std::experimental::propagate_const<std::unique_ptr<FILTER_FX::ZoomFilterFx>> m_zoomFilterFx;

  IGoomStateHandler* m_goomStateHandler;
  bool m_allowMultiThreadedStates = true;
  auto ChangeState() noexcept -> void;
  IGoomStateHandler::DrawablesState m_currentGoomDrawables{};

  ResetDrawBuffSettingsFunc m_resetDrawBuffSettings{};
  auto ResetCurrentDrawBuffSettings(GoomDrawables fx) noexcept -> void;
  [[nodiscard]] auto GetCurrentBuffSettings(GoomDrawables fx) const noexcept -> FXBuffSettings;

  VisualFxColorMaps m_visualFxColorMaps{*m_goomRand};
  UTILS::AdaptiveExposure m_adaptiveExposure{};
  bool m_doExposureControl = false;
  auto UpdateZoomFilterLuminance() noexcept -> void;
  [[nodiscard]] auto GetCurrentBufferAverageLuminance() noexcept -> float;

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

inline auto GoomAllVisualFx::SetTranBufferDest(
    const std_spn::span<Point2dFlt>& tranBufferFlt) noexcept -> void
{
  m_zoomFilterFx->SetTranBufferDest(tranBufferFlt);
}

inline auto GoomAllVisualFx::SetAllowMultiThreadedStates(const bool val) noexcept -> void
{
  m_allowMultiThreadedStates = val;
}

inline auto GoomAllVisualFx::GetZoomFilterFx() const noexcept -> const FILTER_FX::ZoomFilterFx&
{
  return *m_zoomFilterFx;
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

inline auto GoomAllVisualFx::ApplyZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff) noexcept
    -> void
{
  m_zoomFilterFx->ZoomFilterFastRgb(srceBuff, destBuff);

  UpdateZoomFilterLuminance();
}

inline auto GoomAllVisualFx::GetTranLerpFactor() const noexcept -> uint32_t
{
  return m_zoomFilterFx->GetTranLerpFactor();
}

inline auto GoomAllVisualFx::IsFilterPosDataReady() const noexcept -> bool
{
  return m_zoomFilterFx->IsFilterPosDataReady();
}

inline auto GoomAllVisualFx::UpdateZoomFilterLuminance() noexcept -> void
{
  const auto averageLuminanceToUse = GetCurrentBufferAverageLuminance();
  if (averageLuminanceToUse < UTILS::MATH::SMALL_FLOAT)
  {
    // No point trying to handle zero luminance.
    return;
  }

  m_adaptiveExposure.UpdateAverageLuminance(averageLuminanceToUse);

  if (m_doExposureControl)
  {
    m_zoomFilterFx->SetZoomFilterBrightness(m_adaptiveExposure.GetCurrentExposure());
  }
}

inline auto GoomAllVisualFx::GetCurrentBufferAverageLuminance() noexcept -> float
{
  m_zoomFilterFx->GetLastFilterBufferColorInfo().CalculateLuminances();

  const auto& filterBufferColorInfo = m_zoomFilterFx->GetLastFilterBufferColorInfo();

  const auto maxRegionAverageLuminance = filterBufferColorInfo.GetMaxRegionAverageLuminance();
  if (maxRegionAverageLuminance < UTILS::MATH::SMALL_FLOAT)
  {
    return 0.0F;
  }

  const auto zoomPointRegionAverageLuminance =
      filterBufferColorInfo.GetRegionAverageLuminanceAtPoint(
          m_zoomFilterFx->GetFilterEffectsSettings().zoomMidpoint);
  static constexpr auto LUMINANCE_MIX = 0.5F;

  return STD20::lerp(zoomPointRegionAverageLuminance, maxRegionAverageLuminance, LUMINANCE_MIX);
}

inline auto GoomAllVisualFx::GetCurrentExposure() const noexcept -> float
{
  return m_adaptiveExposure.GetCurrentExposure();
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
