#pragma once

#include "goom_visual_fx.h"
#include "utils/spimpl.h"

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

class GoomDotsFx : public IVisualFx
{
public:
  GoomDotsFx() noexcept = delete;
  GoomDotsFx(const DRAW::IGoomDraw& draw,
             const PluginInfo& goomInfo,
             const UTILS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void Start() override;

  void Resume() override;
  void Suspend() override;

  static constexpr uint32_t NUM_DOT_TYPES = 5;
  void SetWeightedColorMaps(uint32_t dotNum, std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  void ApplySingle();
  void ApplyMultiple();

  void Finish() override;

private:
  class GoomDotsFxImpl;
  spimpl::unique_impl_ptr<GoomDotsFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM

