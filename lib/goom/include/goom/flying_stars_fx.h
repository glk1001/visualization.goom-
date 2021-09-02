#ifndef VISUALIZATION_GOOM_FLYING_STARS_FX_H
#define VISUALIZATION_GOOM_FLYING_STARS_FX_H

#include "goom_visual_fx.h"
#include "goomutils/spimpl.h"

#include <string>

namespace GOOM
{

class IGoomDraw;
class PluginInfo;

namespace UTILS
{
class RandomColorMaps;
class SmallImageBitmaps;
} // namespace UTILS

class FlyingStarsFx : public IVisualFx
{
public:
  enum class ColorMode
  {
    _NULL = -1,
    MIX_COLORS,
    REVERSE_MIX_COLORS,
    SIMILAR_LOW_COLORS,
    SINE_MIX_COLORS,
  };

  FlyingStarsFx() noexcept = delete;
  explicit FlyingStarsFx(const IGoomDraw& draw,
                         const std::shared_ptr<const PluginInfo>& goomInfo,
                         const UTILS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void Start() override;

  void Resume() override;
  void Suspend() override;

  void SetWeightedColorMaps(std::shared_ptr<UTILS::RandomColorMaps> weightedMaps);
  void SetWeightedLowColorMaps(std::shared_ptr<UTILS::RandomColorMaps> weightedMaps);

  void ApplyMultiple();

  void Finish() override;

private:
  bool m_enabled = true;
  class FlyingStarsImpl;
  spimpl::unique_impl_ptr<FlyingStarsImpl> m_fxImpl;
};

} // namespace GOOM
#endif
