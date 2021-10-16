#pragma once

#include "goom_random_states.h"
#include "goom_states.h"
#include "visual_fx/lines_fx.h"
#include "visual_fx/zoom_filter_fx.h"

#include <functional>
#include <map>
#include <memory>
#include <unordered_set>

namespace GOOM
{
class AudioSamples;
class PluginInfo;
class PixelBuffer;

namespace UTILS
{
class Parallel;
class SmallImageBitmaps;
} // namespace UTILS

namespace DRAW
{
class IGoomDraw;
} // namespace DRAW

namespace FILTERS
{
class FilterBuffersService;
class FilterColorsService;
struct ZoomFilterSettings;
} // namespace FILTERS

namespace VISUAL_FX
{
class IVisualFx;
class FlyingStarsFx;
class GoomDotsFx;
class IfsDancersFx;
class ImageFx;
class TentaclesFx;
class TubeFx;
class ZoomFilterFx;
} // namespace VISUAL_FX

namespace CONTROL
{

class GoomAllVisualFx
{
public:
  GoomAllVisualFx() noexcept = delete;
  GoomAllVisualFx(UTILS::Parallel& parallel,
                  DRAW::IGoomDraw& draw,
                  const PluginInfo& goomInfo,
                  const UTILS::SmallImageBitmaps& smallBitmaps,
                  const std::string& resourcesDirectory,
                  std::unique_ptr<FILTERS::FilterBuffersService> filterBuffersService,
                  std::unique_ptr<FILTERS::FilterColorsService> filterColorsService) noexcept;

  void Start();
  void Finish();

  void SetNextState();
  [[nodiscard]] auto GetCurrentStateName() const -> std::string;
  [[nodiscard]] auto IsCurrentlyDrawable(GoomDrawables goomDrawable) const -> bool;

  void SetSingleBufferDots(bool value);
  using ResetDrawBuffSettingsFunc = std::function<void(const FXBuffSettings& settings)>;
  void SetResetDrawBuffSettingsFunc(const ResetDrawBuffSettingsFunc& f);

  void ChangeColorMaps();
  void RefreshAll();

  void ApplyCurrentStateToSingleBuffer();
  void ApplyCurrentStateToMultipleBuffers();

  void SetZoomFilterAllowOverexposed(bool allowOverexposed);
  void UpdateFilterSettings(const FILTERS::ZoomFilterSettings& filterSettings,
                            bool updateFilterEffects);
  void ApplyZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff);

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
  const std::shared_ptr<VISUAL_FX::ZoomFilterFx> m_zoomFilter_fx;
  const std::shared_ptr<VISUAL_FX::FlyingStarsFx> m_star_fx;
  const std::shared_ptr<VISUAL_FX::GoomDotsFx> m_goomDots_fx;
  const std::shared_ptr<VISUAL_FX::IfsDancersFx> m_ifs_fx;
  const std::shared_ptr<VISUAL_FX::ImageFx> m_image_fx;
  const std::shared_ptr<VISUAL_FX::TentaclesFx> m_tentacles_fx;
  const std::shared_ptr<VISUAL_FX::TubeFx> m_tube_fx;
  const std::shared_ptr<VISUAL_FX::LinesFx> m_goomLine1;
  const std::shared_ptr<VISUAL_FX::LinesFx> m_goomLine2;

  const std::vector<std::shared_ptr<VISUAL_FX::IVisualFx>> m_list;
  const std::map<GoomDrawables, std::shared_ptr<VISUAL_FX::IVisualFx>> m_drawablesMap;

  GoomRandomStates m_state{};
  std::unordered_set<GoomDrawables> m_currentGoomDrawables{};
  void ChangeState();
  void PostStateUpdate(const std::unordered_set<GoomDrawables>& oldGoomDrawables);
  [[nodiscard]] auto GetCurrentGoomDrawables() const -> std::unordered_set<GoomDrawables>;
  [[nodiscard]] auto CanDraw(GoomDrawables goomDrawable) const -> bool;
  [[nodiscard]] auto GetCurrentBuffSettings(GoomDrawables fx) const -> FXBuffSettings;

  bool m_singleBufferDots = true;
  static constexpr float INITIAL_SCREEN_HEIGHT_FRACTION_LINE1 = 0.4F;
  static constexpr float INITIAL_SCREEN_HEIGHT_FRACTION_LINE2 = 0.2F;

  void SuspendFx();
  void ResumeFx();
  void SetZoomMidPoint(const V2dInt& zoomMidPoint);

  void ApplyDotsIfRequired();
  void ApplyDotsToBothBuffersIfRequired();
  void ApplyIfsToBothBuffersIfRequired();
  void ApplyImageToBothBuffersIfRequired();
  void ApplyTentaclesToBothBuffersIfRequired();
  void ApplyStarsToBothBuffersIfRequired();
  void ApplyTubeToBothBuffersIfRequired();

  ResetDrawBuffSettingsFunc m_resetDrawBuffSettings{};
  void ResetDrawBuffSettings(const FXBuffSettings& settings);
};

inline void GoomAllVisualFx::SetNextState()
{
  const auto oldGDrawables = GetCurrentGoomDrawables();
  ChangeState();
  ChangeColorMaps();
  PostStateUpdate(oldGDrawables);
}

inline void GoomAllVisualFx::ApplyZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff)
{
  m_zoomFilter_fx->ZoomFilterFastRgb(srceBuff, destBuff);
}

inline void GoomAllVisualFx::SetZoomFilterAllowOverexposed(const bool allowOverexposed)
{
  m_zoomFilter_fx->SetBuffSettings({/*.buffIntensity = */ FXBuffSettings::INITIAL_BUFF_INTENSITY,
                                    /*.allowOverexposed = */ allowOverexposed});
}

inline void GoomAllVisualFx::SetResetDrawBuffSettingsFunc(const ResetDrawBuffSettingsFunc& f)
{
  m_resetDrawBuffSettings = f;
}

inline auto GoomAllVisualFx::GetCurrentStateName() const -> std::string
{
  return GoomStateInfo::GetStateInfo(m_state.GetCurrentState()).name;
}

inline auto GoomAllVisualFx::GetCurrentGoomDrawables() const -> std::unordered_set<GoomDrawables>
{
  return m_currentGoomDrawables;
}

inline auto GoomAllVisualFx::CanDraw(const GoomDrawables goomDrawable) const -> bool
{
  return m_drawablesMap.find(goomDrawable) != m_drawablesMap.end();
}

inline auto GoomAllVisualFx::IsCurrentlyDrawable(const GoomDrawables goomDrawable) const -> bool
{
#if __cplusplus <= 201703L
  return m_currentGoomDrawables.find(goomDrawable) != m_currentGoomDrawables.end();
#else
  return m_currentGoomDrawables.contains(goomDrawable);
#endif
}

inline void GoomAllVisualFx::SetSingleBufferDots(const bool value)
{
  m_singleBufferDots = value;
}

inline void GoomAllVisualFx::ApplyCurrentStateToSingleBuffer()
{
  // applyIfsIfRequired();
  ApplyDotsIfRequired();
}

inline void GoomAllVisualFx::ApplyCurrentStateToMultipleBuffers()
{
  ApplyDotsToBothBuffersIfRequired();
  ApplyIfsToBothBuffersIfRequired();
  ApplyImageToBothBuffersIfRequired();
  ApplyTentaclesToBothBuffersIfRequired();
  ApplyStarsToBothBuffersIfRequired();
  ApplyTubeToBothBuffersIfRequired();
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
