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
class FxHelper;
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
  [[nodiscard]] auto GetCurrentPixelBlendFunc() const noexcept -> DRAW::IGoomDraw::BlendPixelFunc;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  enum class PixelBlendType
  {
    ADD,
    REVERSE_ADD,
    MULTIPLY,
    LUMA_MIX,
    _num // unused, and marks the enum end
  };
  static constexpr float ADD_WEIGHT         = 50.0F;
  static constexpr float REVERSE_ADD_WEIGHT = 3.0F;
  static constexpr float MULTIPLY_WEIGHT    = 3.0F;
  static constexpr float LUMA_MIX_WEIGHT    = 3.0F;
  const UTILS::MATH::Weights<PixelBlendType> m_pixelBlendTypeWeights{
      m_goomRand,
      {
        {PixelBlendType::ADD, ADD_WEIGHT},
        {PixelBlendType::REVERSE_ADD, REVERSE_ADD_WEIGHT},
        {PixelBlendType::MULTIPLY, MULTIPLY_WEIGHT},
        {PixelBlendType::LUMA_MIX, LUMA_MIX_WEIGHT},
        }
  };
  PixelBlendType m_currentPixelBlendType                   = PixelBlendType::ADD;
  DRAW::IGoomDraw::BlendPixelFunc m_currentBlendPixelFunc  = GetColorAddBlendPixelFunc();
  DRAW::IGoomDraw::BlendPixelFunc m_previousBlendPixelFunc = GetColorAddBlendPixelFunc();
  static constexpr auto MAX_BLEND_STEPS                    = 500U;
  static constexpr auto MIN_BLEND_STEPS                    = 50U;
  UTILS::TValue m_blendT{UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE, MIN_BLEND_STEPS};
  [[nodiscard]] auto GetPixelBlendFunc() const noexcept -> DRAW::IGoomDraw::BlendPixelFunc;
  [[nodiscard]] auto GetColorAddBlendPixelFunc() const -> DRAW::IGoomDraw::BlendPixelFunc;
  [[nodiscard]] auto GetLerpedBlendPixelFunc() const -> DRAW::IGoomDraw::BlendPixelFunc;
  [[nodiscard]] auto GetReverseColorAddBlendPixelFunc() const -> DRAW::IGoomDraw::BlendPixelFunc;
  [[nodiscard]] auto GetColorMultiplyBlendPixelFunc() const -> DRAW::IGoomDraw::BlendPixelFunc;
  static constexpr auto MAX_LUMA_MIX_T = 1.0F;
  static constexpr auto MIN_LUMA_MIX_T = 0.3F;
  float m_lumaMixT                     = MIN_LUMA_MIX_T;
  [[nodiscard]] auto GetSameLumaMixBlendPixelFunc() const -> DRAW::IGoomDraw::BlendPixelFunc;
};

inline auto PixelBlender::Update() noexcept -> void
{
  m_blendT.Increment();
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
  GoomAllVisualFx(const GoomAllVisualFx&) noexcept = delete;
  GoomAllVisualFx(GoomAllVisualFx&&) noexcept      = delete;
  ~GoomAllVisualFx() noexcept;
  auto operator=(const GoomAllVisualFx&) noexcept -> GoomAllVisualFx& = delete;
  auto operator=(GoomAllVisualFx&&) noexcept -> GoomAllVisualFx&      = delete;

  auto Start() -> void;
  auto Finish() -> void;

  auto SetAllowMultiThreadedStates(bool val) -> void;

  [[nodiscard]] auto GetZoomFilterFx() const -> const FILTER_FX::ZoomFilterFx&;

  auto SetNextState() -> void;
  [[nodiscard]] auto GetCurrentState() const -> GoomStates;
  [[nodiscard]] auto GetCurrentStateName() const -> std::string_view;

  auto SetSingleBufferDots(bool value) -> void;

  auto StartExposureControl() -> void;
  [[nodiscard]] auto GetCurrentExposure() const -> float;
  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

  using ResetDrawBuffSettingsFunc = std::function<void(const FXBuffSettings& settings)>;
  auto SetResetDrawBuffSettingsFunc(const ResetDrawBuffSettingsFunc& func) -> void;

  auto ChangeAllFxColorMaps() -> void;
  auto ChangeDrawPixelBlend() -> void;
  auto RefreshAllFx() -> void;

  auto ApplyCurrentStateToSingleBuffer() -> void;
  auto ApplyCurrentStateToMultipleBuffers(const AudioSamples& soundData) -> void;
  auto ApplyEndEffectIfNearEnd(const UTILS::Stopwatch::TimeValues& timeValues) -> void;

  auto UpdateFilterSettings(const FILTER_FX::ZoomFilterSettings& filterSettings) -> void;
  auto ApplyZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff) -> void;

  [[nodiscard]] auto GetCurrentColorMapsNames() const -> std::unordered_set<std::string>;
  [[nodiscard]] auto GetZoomFilterFxNameValueParams() const -> UTILS::NameValuePairs;

private:
  DRAW::IGoomDraw& m_goomDraw;
  const UTILS::MATH::IGoomRand& m_goomRand;
  GoomLogger& m_goomLogger;
  spimpl::unique_impl_ptr<AllStandardVisualFx> m_allStandardVisualFx;
  std::experimental::propagate_const<std::unique_ptr<FILTER_FX::ZoomFilterFx>> m_zoomFilterFx;

  IGoomStateHandler& m_goomStateHandler;
  bool m_allowMultiThreadedStates = true;
  auto ChangeState() -> void;
  auto PostStateUpdate(const std::unordered_set<GoomDrawables>& oldGoomDrawables) -> void;
  std::unordered_set<GoomDrawables> m_currentGoomDrawables{};

  ResetDrawBuffSettingsFunc m_resetDrawBuffSettings{};
  auto ResetCurrentDrawBuffSettings(GoomDrawables fx) -> void;
  [[nodiscard]] auto GetCurrentBuffSettings(GoomDrawables fx) const -> FXBuffSettings;

  VisualFxColorMaps m_visualFxColorMaps{m_goomRand};
  UTILS::AdaptiveExposure m_adaptiveExposure{};
  bool m_doExposureControl = false;
  auto UpdateZoomFilterLuminance() -> void;
  [[nodiscard]] auto GetCurrentBufferAverageLuminance() noexcept -> float;

  PixelBlender m_pixelBlender{m_goomRand};
};

inline auto GoomAllVisualFx::SetAllowMultiThreadedStates(const bool val) -> void
{
  m_allowMultiThreadedStates = val;
}

inline auto GoomAllVisualFx::GetZoomFilterFx() const -> const FILTER_FX::ZoomFilterFx&
{
  return *m_zoomFilterFx;
}

inline auto GoomAllVisualFx::SetNextState() -> void
{
  ChangeState();
  ChangeAllFxColorMaps();
  ChangeDrawPixelBlend();
  PostStateUpdate(m_currentGoomDrawables);
}

inline auto GoomAllVisualFx::SetResetDrawBuffSettingsFunc(const ResetDrawBuffSettingsFunc& func)
    -> void
{
  m_resetDrawBuffSettings = func;
}

inline auto GoomAllVisualFx::ApplyZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff) -> void
{
  m_zoomFilterFx->ZoomFilterFastRgb(srceBuff, destBuff);

  UpdateZoomFilterLuminance();
  m_pixelBlender.Update();
}

inline auto GoomAllVisualFx::UpdateZoomFilterLuminance() -> void
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

inline auto GoomAllVisualFx::GetCurrentExposure() const -> float
{
  return m_adaptiveExposure.GetCurrentExposure();
}

inline auto GoomAllVisualFx::GetCurrentState() const -> GoomStates
{
  return m_goomStateHandler.GetCurrentState();
}

inline auto GoomAllVisualFx::GetCurrentStateName() const -> std::string_view
{
  return GoomStateInfo::GetStateInfo(m_goomStateHandler.GetCurrentState()).name;
}

} // namespace CONTROL
} // namespace GOOM
