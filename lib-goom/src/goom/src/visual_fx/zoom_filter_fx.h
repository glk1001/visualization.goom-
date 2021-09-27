#pragma once

#include "goom_visual_fx.h"
#include "goomutils/name_value_pairs.h"
#include "goomutils/spimpl.h"

#include <cstdint>
#include <memory>
#include <string>

namespace GOOM
{

namespace FILTERS
{
class FilterBuffersService;
class FilterColorsService;
struct ZoomFilterBufferSettings;
struct ZoomFilterEffectsSettings;
struct ZoomFilterColorSettings;
} // namespace FILTERS

namespace UTILS
{
class Parallel;
} // namespace UTILS

struct FXBuffSettings;
class PixelBuffer;
class PluginInfo;

namespace VISUAL_FX
{

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

  auto GetTranLerpFactor() const -> int32_t;

  void UpdateFilterEffectsSettings(const FILTERS::ZoomFilterEffectsSettings& filterEffectsSettings);
  void UpdateFilterBufferSettings(const FILTERS::ZoomFilterBufferSettings& filterBufferSettings);
  void UpdateFilterColorSettings(const FILTERS::ZoomFilterColorSettings& filterColorSettings);

  void ZoomFilterFastRgb(const PixelBuffer& srceBuff, PixelBuffer& destBuff);

  void Finish() override;

  [[nodiscard]] auto GetNameValueParams() const -> UTILS::NameValuePairs;

private:
  class ZoomFilterImpl;
  spimpl::unique_impl_ptr<ZoomFilterImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM

