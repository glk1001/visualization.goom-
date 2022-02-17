#pragma once

#include "draw/goom_draw.h"
#include "goom/spimpl.h"
#include "goom_state_handler.h"
#include "goom_states.h"
#include "utils/adaptive_exposure.h"
#include "utils/math/mathutils.h"
#include "visual_fx/filters/filter_buffer_row_color_info.h"
#include "visual_fx/lines_fx.h"
#include "visual_fx/zoom_filter_fx.h"
#include "visual_fx_color_maps.h"

#include <functional>
#include <memory>
#include <unordered_set>

namespace GOOM
{
class AudioSamples;
class PluginInfo;
class PixelBuffer;

namespace UTILS
{
class IGoomRand;
class Parallel;
}
namespace UTILS::GRAPHICS
{
class SmallImageBitmaps;
}

namespace VISUAL_FX
{
class FxHelper;

namespace FILTERS
{
class FilterBuffersService;
class FilterColorsService;
struct ZoomFilterSettings;
}
} // namespace VISUAL_FX

namespace CONTROL
{
class AllStandardVisualFx;

class GoomAllVisualFx
{
public:
  GoomAllVisualFx() noexcept = delete;
  GoomAllVisualFx(
      UTILS::Parallel& parallel,
      const VISUAL_FX::FxHelper& fxHelper,
      const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
      const std::string& resourcesDirectory,
      IGoomStateHandler& goomStateHandler,
      std::unique_ptr<VISUAL_FX::FILTERS::FilterBuffersService> filterBuffersService,
      std::unique_ptr<VISUAL_FX::FILTERS::FilterColorsService> filterColorsService) noexcept;

  void Start();
  void Finish();

  void SetNextState();
  [[nodiscard]] auto GetCurrentStateName() const -> std::string;

  void SetSingleBufferDots(bool value);

  void StartExposureControl();
  [[nodiscard]] auto GetCurrentExposure() const -> float;
  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

  using ResetDrawBuffSettingsFunc = std::function<void(const FXBuffSettings& settings)>;
  void SetResetDrawBuffSettingsFunc(const ResetDrawBuffSettingsFunc& func);

  void ChangeAllFxColorMaps();
  void ChangeDrawPixelBlend();
  void RefreshAllFx();

  void ApplyCurrentStateToSingleBuffer();
  void ApplyCurrentStateToMultipleBuffers();

  void UpdateFilterSettings(const VISUAL_FX::FILTERS::ZoomFilterSettings& filterSettings,
                            bool updateFilterEffects);
  void ApplyZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff);

  [[nodiscard]] auto CanDisplayLines() const -> bool;
  [[nodiscard]] auto IsScopeDrawable() const -> bool;
  [[nodiscard]] auto IsFarScopeDrawable() const -> bool;
  void DisplayGoomLines(const AudioSamples& soundData);
  [[nodiscard]] auto CanResetDestGoomLines() const -> bool;
  struct GoomLineSettings
  {
    struct Params
    {
      float line1{};
      float line2{};
    };
    struct Colors
    {
      Pixel line1{};
      Pixel line2{};
    };
    VISUAL_FX::LinesFx::LineType mode{};
    float amplitude{};
    Params params{};
    Colors colors{};
  };
  void ResetDestGoomLines(const GoomLineSettings& lineSettings);
  [[nodiscard]] auto GetGoomLine1RandomColor() const -> Pixel;
  [[nodiscard]] auto GetGoomLine2RandomColor() const -> Pixel;

  [[nodiscard]] auto GetZoomFilterFxNameValueParams() const -> UTILS::NameValuePairs;

private:
  spimpl::unique_impl_ptr<AllStandardVisualFx> m_allStandardVisualFx;
  const std::unique_ptr<VISUAL_FX::ZoomFilterFx> m_zoomFilterFx;
  const std::unique_ptr<VISUAL_FX::LinesFx> m_goomLine1;
  const std::unique_ptr<VISUAL_FX::LinesFx> m_goomLine2;
  DRAW::IGoomDraw& m_goomDraw;
  const UTILS::MATH::IGoomRand& m_goomRand;

  IGoomStateHandler& m_goomStateHandler;
  void ChangeState();
  void PostStateUpdate(const std::unordered_set<GoomDrawables>& oldGoomDrawables);
  std::unordered_set<GoomDrawables> m_currentGoomDrawables{};
  [[nodiscard]] auto IsCurrentlyDrawable(GoomDrawables goomDrawable) const -> bool;

  ResetDrawBuffSettingsFunc m_resetDrawBuffSettings{};
  void ResetCurrentDrawBuffSettings(GoomDrawables fx);
  [[nodiscard]] auto GetCurrentBuffSettings(GoomDrawables fx) const -> FXBuffSettings;

  VisualFxColorMaps m_visualFxColorMaps;
  UTILS::AdaptiveExposure m_adaptiveExposure{};
  bool m_doExposureControl = false;
  void UpdateZoomFilterLuminance();

  static constexpr float INITIAL_SCREEN_HEIGHT_FRACTION_LINE1 = 0.4F;
  static constexpr float INITIAL_SCREEN_HEIGHT_FRACTION_LINE2 = 0.2F;
  void ChangeLineColorMaps();

  [[nodiscard]] static auto GetReverseColorAddBlendPixelPixelFunc()
      -> DRAW::IGoomDraw::BlendPixelFunc;
  [[nodiscard]] static auto GetSameLumaBlendPixelFunc() -> DRAW::IGoomDraw::BlendPixelFunc;
  [[nodiscard]] static auto GetSameLumaMixBlendPixelFunc() -> DRAW::IGoomDraw::BlendPixelFunc;
};

inline void GoomAllVisualFx::SetNextState()
{
  ChangeState();
  ChangeAllFxColorMaps();
  ChangeDrawPixelBlend();
  PostStateUpdate(m_currentGoomDrawables);
}

inline void GoomAllVisualFx::SetResetDrawBuffSettingsFunc(const ResetDrawBuffSettingsFunc& func)
{
  m_resetDrawBuffSettings = func;
}

inline auto GoomAllVisualFx::IsCurrentlyDrawable(const GoomDrawables goomDrawable) const -> bool
{
#if __cplusplus <= 201703L
  return m_currentGoomDrawables.find(goomDrawable) != m_currentGoomDrawables.end();
#else
  return m_currentGoomDrawables.contains(goomDrawable);
#endif
}

inline void GoomAllVisualFx::ApplyZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff)
{
  m_zoomFilterFx->ZoomFilterFastRgb(srceBuff, destBuff);

  UpdateZoomFilterLuminance();
}

inline void GoomAllVisualFx::UpdateZoomFilterLuminance()
{
  const float currentBufferAverageLuminance =
      VISUAL_FX::FILTERS::FilterBufferRowColorInfo::GetBufferAverageLuminance(
          m_zoomFilterFx->GetLastFilterBufferColorInfo());

  if (currentBufferAverageLuminance < UTILS::MATH::SMALL_FLOAT)
  {
    // No point trying to handle zero luminance.
    return;
  }
  m_adaptiveExposure.UpdateAverageLuminance(currentBufferAverageLuminance);

  if (m_doExposureControl)
  {
    m_zoomFilterFx->SetZoomFilterBrightness(m_adaptiveExposure.GetCurrentExposure());
  }
}

inline auto GoomAllVisualFx::GetCurrentExposure() const -> float
{
  return m_adaptiveExposure.GetCurrentExposure();
}

inline auto GoomAllVisualFx::GetCurrentStateName() const -> std::string
{
  return GoomStateInfo::GetStateInfo(m_goomStateHandler.GetCurrentState()).name;
}

inline auto GoomAllVisualFx::CanDisplayLines() const -> bool
{
  return IsCurrentlyDrawable(GoomDrawables::LINES);
}

inline auto GoomAllVisualFx::IsScopeDrawable() const -> bool
{
  return IsCurrentlyDrawable(GoomDrawables::SCOPE);
}

inline auto GoomAllVisualFx::IsFarScopeDrawable() const -> bool
{
  return IsCurrentlyDrawable(GoomDrawables::FAR_SCOPE);
}

inline auto GoomAllVisualFx::CanResetDestGoomLines() const -> bool
{
  return m_goomLine1->CanResetDestLine() && m_goomLine2->CanResetDestLine();
}

inline void GoomAllVisualFx::ResetDestGoomLines(const GoomLineSettings& lineSettings)
{
  m_goomLine1->ResetDestLine(lineSettings.mode, lineSettings.params.line1, lineSettings.amplitude,
                             lineSettings.colors.line1);
  m_goomLine2->ResetDestLine(lineSettings.mode, lineSettings.params.line2, lineSettings.amplitude,
                             lineSettings.colors.line2);
}

inline auto GoomAllVisualFx::GetGoomLine1RandomColor() const -> Pixel
{
  return m_goomLine1->GetRandomLineColor();
}

inline auto GoomAllVisualFx::GetGoomLine2RandomColor() const -> Pixel
{
  return m_goomLine2->GetRandomLineColor();
}

} // namespace CONTROL
} // namespace GOOM
