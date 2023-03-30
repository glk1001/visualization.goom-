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

class PixelBlender
{
public:
  explicit PixelBlender(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto Update() noexcept -> void;
  auto ChangePixelBlendFunc() noexcept -> void;
  [[nodiscard]] auto GetCurrentPixelBlendFunc() const noexcept -> DRAW::IGoomDraw::PixelBlendFunc;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  enum class PixelBlendType
  {
    ADD,
    MULTIPLY,
    LUMA_MIX,
    _num // unused, and marks the enum end
  };
  static constexpr auto ADD_WEIGHT      = 50.0F;
  static constexpr auto MULTIPLY_WEIGHT = 5.0F;
  static constexpr auto LUMA_MIX_WEIGHT = 5.0F;
  UTILS::MATH::Weights<PixelBlendType> m_pixelBlendTypeWeights{
      *m_goomRand,
      {
                  {PixelBlendType::ADD, ADD_WEIGHT},
                  {PixelBlendType::MULTIPLY, MULTIPLY_WEIGHT},
                  {PixelBlendType::LUMA_MIX, LUMA_MIX_WEIGHT},
                  }
  };
  PixelBlendType m_nextPixelBlendType                      = PixelBlendType::ADD;
  DRAW::IGoomDraw::PixelBlendFunc m_previousPixelBlendFunc = GetColorAddPixelBlendFunc();
  DRAW::IGoomDraw::PixelBlendFunc m_nextPixelBlendFunc     = m_previousPixelBlendFunc;
  DRAW::IGoomDraw::PixelBlendFunc m_currentPixelBlendFunc  = m_previousPixelBlendFunc;
  static constexpr auto MAX_LERP_STEPS                     = 500U;
  static constexpr auto MIN_LERP_STEPS                     = 50U;
  UTILS::TValue m_lerpT{
      {UTILS::TValue::StepType::SINGLE_CYCLE, MIN_LERP_STEPS}
  };
  [[nodiscard]] auto GetNextPixelBlendFunc() const noexcept -> DRAW::IGoomDraw::PixelBlendFunc;
  [[nodiscard]] auto GetLerpedPixelBlendFunc() const -> DRAW::IGoomDraw::PixelBlendFunc;
  [[nodiscard]] static auto GetColorAddPixelBlendFunc() -> DRAW::IGoomDraw::PixelBlendFunc;
  [[nodiscard]] static auto GetColorMultiplyPixelBlendFunc() -> DRAW::IGoomDraw::PixelBlendFunc;
  static constexpr auto MAX_LUMA_MIX_T = 1.0F;
  static constexpr auto MIN_LUMA_MIX_T = 0.3F;
  float m_lumaMixT                     = MIN_LUMA_MIX_T;
  [[nodiscard]] auto GetSameLumaMixPixelBlendFunc() const -> DRAW::IGoomDraw::PixelBlendFunc;
};

inline auto PixelBlender::Update() noexcept -> void
{
  m_lerpT.Increment();

  if (m_lerpT() >= 1.0F)
  {
    m_currentPixelBlendFunc = m_nextPixelBlendFunc;
  }
}

inline auto PixelBlender::GetCurrentPixelBlendFunc() const noexcept
    -> DRAW::IGoomDraw::PixelBlendFunc
{
  return m_currentPixelBlendFunc;
}

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

  auto SetAllowMultiThreadedStates(bool val) noexcept -> void;

  [[nodiscard]] auto GetZoomFilterFx() const noexcept -> const FILTER_FX::ZoomFilterFx&;

  auto SetNextState() noexcept -> void;
  [[nodiscard]] auto GetCurrentState() const noexcept -> GoomStates;
  [[nodiscard]] auto GetCurrentStateName() const noexcept -> std::string_view;

  auto SetSingleBufferDots(bool value) noexcept -> void;

  auto StartExposureControl() noexcept -> void;
  [[nodiscard]] auto GetCurrentExposure() const noexcept -> float;
  [[nodiscard]] auto GetLastShaderVariables() const noexcept -> const GoomShaderVariables&;

  using ResetDrawBuffSettingsFunc = std::function<void(const FXBuffSettings& settings)>;
  auto SetResetDrawBuffSettingsFunc(const ResetDrawBuffSettingsFunc& func) noexcept -> void;

  auto ChangeAllFxColorMaps() noexcept -> void;
  auto ChangeDrawPixelBlend() noexcept -> void;
  auto RefreshAllFx() noexcept -> void;

  auto ApplyCurrentStateToSingleBuffer() noexcept -> void;
  auto ApplyCurrentStateToMultipleBuffers(const AudioSamples& soundData) noexcept -> void;
  auto ApplyEndEffectIfNearEnd(const UTILS::Stopwatch::TimeValues& timeValues) noexcept -> void;

  auto UpdateFilterSettings(const FILTER_FX::ZoomFilterSettings& filterSettings) noexcept -> void;
  auto ApplyZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff) noexcept -> void;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::unordered_set<std::string>;
  [[nodiscard]] auto GetZoomFilterFxNameValueParams() const noexcept -> UTILS::NameValuePairs;

private:
  DRAW::IGoomDraw* m_draw;
  const UTILS::MATH::IGoomRand* m_goomRand;
  [[maybe_unused]] GoomLogger* m_goomLogger;
  spimpl::unique_impl_ptr<AllStandardVisualFx> m_allStandardVisualFx;
  std::experimental::propagate_const<std::unique_ptr<FILTER_FX::ZoomFilterFx>> m_zoomFilterFx;

  IGoomStateHandler* m_goomStateHandler;
  bool m_allowMultiThreadedStates = true;
  auto ChangeState() noexcept -> void;
  auto PostStateUpdate(const std::unordered_set<GoomDrawables>& oldGoomDrawables) noexcept -> void;
  std::unordered_set<GoomDrawables> m_currentGoomDrawables{};

  ResetDrawBuffSettingsFunc m_resetDrawBuffSettings{};
  auto ResetCurrentDrawBuffSettings(GoomDrawables fx) noexcept -> void;
  [[nodiscard]] auto GetCurrentBuffSettings(GoomDrawables fx) const noexcept -> FXBuffSettings;

  VisualFxColorMaps m_visualFxColorMaps{*m_goomRand};
  UTILS::AdaptiveExposure m_adaptiveExposure{};
  bool m_doExposureControl = false;
  auto UpdateZoomFilterLuminance() noexcept -> void;
  [[nodiscard]] auto GetCurrentBufferAverageLuminance() noexcept -> float;

  PixelBlender m_pixelBlender{*m_goomRand};
};

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
  ChangeDrawPixelBlend();
  PostStateUpdate(m_currentGoomDrawables);
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
  m_pixelBlender.Update();
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
