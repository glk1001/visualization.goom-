#ifndef VISUALIZATION_GOOM_FILTERS_H
#define VISUALIZATION_GOOM_FILTERS_H

#include "goom_visual_fx.h"
#include "goomutils/spimpl.h"

#include <cstdint>
#include <string>

namespace GOOM
{

namespace FILTERS
{
class ZoomFilterBuffersService;
class ZoomFilterColors;
} // namespace FILTERS

namespace UTILS
{
class Parallel;
} // namespace UTILS

struct FXBuffSettings;
class PixelBuffer;
class PluginInfo;
struct ZoomFilterSettings;

class ZoomFilterFx : public IVisualFx
{
public:
  ZoomFilterFx() noexcept = delete;
  ZoomFilterFx(UTILS::Parallel& p,
               const std::shared_ptr<const PluginInfo>& goomInfo,
               FILTERS::ZoomFilterBuffersService& filterBuffersService,
               FILTERS::ZoomFilterColors& filterColors) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;
  void SetBuffSettings(const FXBuffSettings& settings);

  void Start() override;

  void ZoomFilterFastRgb(const PixelBuffer& srceBuff,
                         PixelBuffer& destBuff,
                         int switchIncr,
                         float switchMult);

  void Finish() override;

private:
  class ZoomFilterImpl;
  spimpl::unique_impl_ptr<ZoomFilterImpl> m_fxImpl;
};

} // namespace GOOM
#endif
