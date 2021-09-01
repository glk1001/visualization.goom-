#ifndef VISUALIZATION_GOOM_GOOM_DOTS_FX_H
#define VISUALIZATION_GOOM_GOOM_DOTS_FX_H

#include "goom_visual_fx.h"
#include "goomutils/spimpl.h"

#include <memory>
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

class GoomDotsFx : public IVisualFx
{
public:
  GoomDotsFx() noexcept = delete;
  explicit GoomDotsFx(const IGoomDraw& draw,
                      const std::shared_ptr<const PluginInfo>& goomInfo,
                      const UTILS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void Start() override;

  void Resume() override;
  void Suspend() override;

  static constexpr uint32_t NUM_DOT_TYPES = 5;
  void SetWeightedColorMaps(uint32_t dotNum, std::shared_ptr<UTILS::RandomColorMaps> weightedMaps);

  void ApplySingle();
  void ApplyMultiple();

  void Finish() override;

private:
  bool m_enabled = true;
  class GoomDotsFxImpl;
  spimpl::unique_impl_ptr<GoomDotsFxImpl> m_fxImpl;
};

} // namespace GOOM

#endif /* VISUALIZATION_GOOM_GOOM_DOTS_FX_H */
