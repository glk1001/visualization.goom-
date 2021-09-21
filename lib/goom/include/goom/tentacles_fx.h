#ifndef VISUALIZATION_GOOM_TENTACLES_FX_H
#define VISUALIZATION_GOOM_TENTACLES_FX_H

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
} // namespace UTILS

class TentaclesFx : public IVisualFx
{
public:
  TentaclesFx() noexcept = delete;
  TentaclesFx(const IGoomDraw& draw, const PluginInfo& goomInfo) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void Start() override;

  void FreshStart();
  void Resume() override;
  void Suspend() override;

  void SetWeightedColorMaps(std::shared_ptr<UTILS::RandomColorMaps> weightedMaps);

  void ApplyNoDraw();
  void ApplyMultiple();

  void Finish() override;

private:
  class TentaclesImpl;
  spimpl::unique_impl_ptr<TentaclesImpl> m_fxImpl;
};

} // namespace GOOM
#endif
