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
} // namespace UTILS

namespace DRAW
{
class IGoomDraw;
} // namespace DRAW

namespace VISUAL_FX
{

class TentaclesFx : public IVisualFx
{
public:
  TentaclesFx() noexcept = delete;
  TentaclesFx(const DRAW::IGoomDraw& draw, const PluginInfo& goomInfo) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void Start() override;

  void FreshStart();
  void Resume() override;
  void Suspend() override;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  void ApplyNoDraw();
  void ApplyMultiple();

  void Finish() override;

private:
  class TentaclesImpl;
  spimpl::unique_impl_ptr<TentaclesImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM

