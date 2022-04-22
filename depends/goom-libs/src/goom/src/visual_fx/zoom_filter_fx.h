#pragma once

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
class FilterBufferColorInfo;
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

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;
  auto SetBuffSettings(const FXBuffSettings& settings) noexcept -> void;

  auto Start() noexcept -> void override;

  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> int32_t;

  [[nodiscard]] auto GetFilterEffectsSettings() const noexcept
      -> const VISUAL_FX::FILTERS::ZoomFilterEffectsSettings&;
  auto UpdateFilterEffectsSettings(
      const VISUAL_FX::FILTERS::ZoomFilterEffectsSettings& filterEffectsSettings) noexcept -> void;
  auto UpdateFilterBufferSettings(
      const VISUAL_FX::FILTERS::ZoomFilterBufferSettings& filterBufferSettings) noexcept -> void;
  auto UpdateFilterColorSettings(
      const VISUAL_FX::FILTERS::ZoomFilterColorSettings& filterColorSettings) noexcept -> void;

  auto ZoomFilterFastRgb(const PixelBuffer& srceBuff, PixelBuffer& destBuff) noexcept -> void;
  auto SetZoomFilterBrightness(float brightness) noexcept -> void;
  [[nodiscard]] auto GetLastFilterBufferColorInfo() const noexcept
      -> const FILTERS::FilterBufferColorInfo&;
  [[nodiscard]] auto GetLastFilterBufferColorInfo() noexcept -> FILTERS::FilterBufferColorInfo&;

  auto Finish() noexcept -> void override;

  [[nodiscard]] auto GetNameValueParams() const noexcept -> UTILS::NameValuePairs;

private:
  class ZoomFilterImpl;
  spimpl::unique_impl_ptr<ZoomFilterImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
