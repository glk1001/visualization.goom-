#pragma once

#include "goom/spimpl.h"
#include "goom_visual_fx.h"

#include <memory>
#include <string>

namespace GOOM
{
class PluginInfo;
struct GoomShaderEffects;

namespace DRAW
{
class IGoomDraw;
} // namespace DRAW

namespace VISUAL_FX
{

class ShaderFx : public IVisualFx
{
public:
  ShaderFx(DRAW::IGoomDraw& draw, const PluginInfo& goomInfo) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void Start() override;

  void ApplyMultiple();

  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

  void Finish() override;

private:
  class ShaderFxImpl;
  spimpl::unique_impl_ptr<ShaderFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
