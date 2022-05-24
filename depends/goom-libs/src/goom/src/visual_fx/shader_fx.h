#pragma once

#include "goom/spimpl.h"
#include "goom_visual_fx.h"
#include "utils/stopwatch.h"

#include <memory>

namespace GOOM
{
struct GoomShaderEffects;

namespace VISUAL_FX
{
class FxHelper;

class ShaderFx : public IVisualFx
{
public:
  explicit ShaderFx(const FxHelper& fxHelper) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void Start() override;

  void ChangeEffects();

  void ApplyMultiple();

  auto ApplyEndEffect(const UTILS::Stopwatch::TimeValues& timeValues) -> void;

  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

  void Finish() override;

private:
  class ShaderFxImpl;
  spimpl::unique_impl_ptr<ShaderFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
