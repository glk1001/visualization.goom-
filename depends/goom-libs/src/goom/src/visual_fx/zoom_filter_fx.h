#pragma once

#include "filters/filter_buffer_row_color_info.h"
#include "goom/spimpl.h"
#include "goom_visual_fx.h"
#include "utils/name_value_pairs.h"

#include <cstdint>
#include <memory>

namespace GOOM
{

namespace UTILS
{
class Parallel;
}

struct FXBuffSettings;
class PixelBuffer;
class PluginInfo;

namespace VISUAL_FX
{

namespace FILTERS
{
class FilterBuffersService;
class FilterColorsService;
struct ZoomFilterBufferSettings;
struct ZoomFilterEffectsSettings;
struct ZoomFilterColorSettings;
}

class ZoomFilterFx : public IVisualFx
{
public:
  ZoomFilterFx() noexcept = delete;
  ZoomFilterFx(UTILS::Parallel& parallel,
               const PluginInfo& goomInfo,
               std::unique_ptr<FILTERS::FilterBuffersService> filterBuffersService,
               std::unique_ptr<FILTERS::FilterColorsService> filterColorsService) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;
  void SetBuffSettings(const FXBuffSettings& settings);

  void Start() override;

  [[nodiscard]] auto GetTranLerpFactor() const -> int32_t;

  void UpdateFilterEffectsSettings(
      const VISUAL_FX::FILTERS::ZoomFilterEffectsSettings& filterEffectsSettings);
  void UpdateFilterBufferSettings(
      const VISUAL_FX::FILTERS::ZoomFilterBufferSettings& filterBufferSettings);
  void UpdateFilterColorSettings(
      const VISUAL_FX::FILTERS::ZoomFilterColorSettings& filterColorSettings);

  void ZoomFilterFastRgb(const PixelBuffer& srceBuff, PixelBuffer& destBuff);
  void SetZoomFilterBrightness(float brightness);
  [[nodiscard]] auto GetLastFilterBufferColorInfo() const
      -> const std::vector<FILTERS::FilterBufferRowColorInfo>&;

  void Finish() override;

  [[nodiscard]] auto GetNameValueParams() const -> UTILS::NameValuePairs;

private:
  class ZoomFilterImpl;
  spimpl::unique_impl_ptr<ZoomFilterImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM

