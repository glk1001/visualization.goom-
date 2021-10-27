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

class TubeFx : public IVisualFx
{
public:
  TubeFx() noexcept = delete;
  TubeFx(DRAW::IGoomDraw& draw,
         const PluginInfo& goomInfo,
         const UTILS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void Start() override;

  void Resume() override;
  void Suspend() override;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);
  void SetWeightedLowColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  void SetZoomMidPoint(const V2dInt& zoomMidPoint) override;

  void ApplyNoDraw();
  void ApplyMultiple();

  void Finish() override;

private:
  class TubeFxImpl;
  spimpl::unique_impl_ptr<TubeFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
