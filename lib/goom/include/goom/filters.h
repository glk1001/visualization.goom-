#ifndef VISUALIZATION_GOOM_FILTERS_H
#define VISUALIZATION_GOOM_FILTERS_H

#include "goom_stats.h"
#include "goom_visual_fx.h"
#include "goomutils/spimpl.h"

#include <cstdint>
#include <memory>
#include <string>

namespace GOOM
{

namespace FILTERS
{
class IZoomVector;
} // namespace FILTERS

namespace UTILS
{
class Parallel;
} // namespace UTILS

struct FXBuffSettings;
class PixelBuffer;
class PluginInfo;
struct ZoomFilterData;

class ZoomFilterFx : public IVisualFx
{
public:
  ZoomFilterFx() noexcept = delete;
  ZoomFilterFx(UTILS::Parallel& p,
               const std::shared_ptr<const PluginInfo>& goomInfo,
               FILTERS::IZoomVector& zoomVector) noexcept;

  [[nodiscard]] auto GetResourcesDirectory() const -> const std::string& override;
  void SetResourcesDirectory(const std::string& dirName) override;

  [[nodiscard]] auto GetFxName() const -> std::string override;
  void SetBuffSettings(const FXBuffSettings& settings);

  auto GetZoomVector() const -> FILTERS::IZoomVector&;

  void Start() override;

  auto GetFilterSettings() const -> const ZoomFilterData&;
  auto GetFilterSettingsArePending() const -> bool;

  auto GetTranLerpFactor() const -> int32_t;

  void SetInitialFilterSettings(const ZoomFilterData& filterSettings);
  void ChangeFilterSettings(const ZoomFilterData& filterSettings);

  void ZoomFilterFastRgb(const PixelBuffer& pix1,
                         PixelBuffer& pix2,
                         int switchIncr,
                         float switchMult,
                         uint32_t& numClipped);

  void Log(const GoomStats::LogStatsValueFunc& logValueFunc) const override;
  void Finish() override;

private:
  class ZoomFilterImpl;
  spimpl::unique_impl_ptr<ZoomFilterImpl> m_fxImpl;
};

} // namespace GOOM
#endif
