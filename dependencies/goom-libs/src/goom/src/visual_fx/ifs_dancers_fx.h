#pragma once

#include "goom/spimpl.h"
#include "goom_visual_fx.h"

#include <memory>
#include <string>

namespace GOOM
{

class PluginInfo;

namespace COLOR
{
class RandomColorMaps;
} // namespace COLOR

namespace UTILS
{
class SmallImageBitmaps;
} // namespace UTILS

namespace DRAW
{
class IGoomDraw;
} // namespace DRAW

namespace VISUAL_FX
{

class IfsDancersFx : public IVisualFx
{
public:
  enum class ColorMode
  {
    _NULL = -1,
    MAP_COLORS,
    MIX_COLORS,
    REVERSE_MIX_COLORS,
    MEGA_MAP_COLOR_CHANGE,
    MEGA_MIX_COLOR_CHANGE,
    SINGLE_COLORS,
    SINE_MIX_COLORS,
    SINE_MAP_COLORS,
    _NUM // unused and must be last
  };

  IfsDancersFx() noexcept = delete;
  explicit IfsDancersFx(DRAW::IGoomDraw& draw,
                        const PluginInfo& goomInfo,
                        const UTILS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  // If colorMode is not set or set to '_null', then returns
  // random weighted color mode.
  [[nodiscard]] auto GetColorMode() const -> ColorMode;
  void SetColorMode(ColorMode colorMode);

  void Start() override;

  void Resume() override;
  void Suspend() override;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);
  void Refresh() override;
  void PostStateUpdate(bool wasActiveInPreviousState) override;

  void ApplyNoDraw();
  void ApplyMultiple();

  void Finish() override;

private:
  class IfsDancersFxImpl;
  spimpl::unique_impl_ptr<IfsDancersFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
