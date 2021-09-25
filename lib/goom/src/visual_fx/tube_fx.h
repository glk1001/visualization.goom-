#pragma once

#include "goom_visual_fx.h"
#include "goomutils/spimpl.h"

#include <memory>
#include <string>

namespace GOOM
{

class PluginInfo;

namespace UTILS
{
class RandomColorMaps;
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
  TubeFx(const DRAW::IGoomDraw& draw,
         const PluginInfo& goomInfo,
         const UTILS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void Start() override;

  void Resume() override;
  void Suspend() override;

  void SetWeightedColorMaps(std::shared_ptr<UTILS::RandomColorMaps> weightedMaps);
  void SetWeightedLowColorMaps(std::shared_ptr<UTILS::RandomColorMaps> weightedMaps);

  void ApplyNoDraw();
  void ApplyMultiple();

  void Finish() override;

private:
  class TubeFxImpl;
  spimpl::unique_impl_ptr<TubeFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM

