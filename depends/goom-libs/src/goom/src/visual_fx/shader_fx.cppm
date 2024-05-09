module;

#include "goom/spimpl.h"

#include <string>
#include <vector>

export module Goom.VisualFx.ShaderFx;

import Goom.Utils.Stopwatch;
import Goom.VisualFx.VisualFxBase;
import Goom.Lib.FrameData;

export namespace GOOM::VISUAL_FX
{
class FxHelper;

class ShaderFx : public IVisualFx
{
public:
  explicit ShaderFx(const FxHelper& fxHelper) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void override;
  auto ChangeEffects() noexcept -> void;

  auto SetFrameMiscData(MiscData& miscData) noexcept -> void override;
  auto ApplyToImageBuffers() noexcept -> void override;
  auto ApplyEndEffect(const UTILS::Stopwatch::TimeValues& timeValues) noexcept -> void;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override
  {
    return {};
  }

private:
  class ShaderFxImpl;
  spimpl::unique_impl_ptr<ShaderFxImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX
