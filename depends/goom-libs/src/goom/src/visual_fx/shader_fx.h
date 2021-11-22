#pragma once

#include "goom/spimpl.h"
#include "goom_visual_fx.h"

#include <memory>
#include <string>

namespace GOOM
{
struct GoomShaderEffects;

namespace VISUAL_FX
{
class FxHelpers;

class ShaderFx : public IVisualFx
{
public:
  explicit ShaderFx(const FxHelpers& fxHelpers) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void Start() override;

  void ChangeEffects();

  void ApplyMultiple();

  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

  void Finish() override;

private:
  class ShaderFxImpl;
  spimpl::unique_impl_ptr<ShaderFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
