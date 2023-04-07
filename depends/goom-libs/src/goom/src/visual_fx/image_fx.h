#pragma once

#include "goom_visual_fx.h"
#include "spimpl.h"

#include <string>
#include <vector>

namespace GOOM::COLOR
{
class RandomColorMaps;
}
namespace GOOM::UTILS
{
class Parallel;
}

namespace GOOM::VISUAL_FX
{
struct FxHelper;

class ImageFx : public IVisualFx
{
public:
  ImageFx(UTILS::Parallel& parallel,
          const FxHelper& fxHelper,
          const std::string& resourcesDirectory) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto Resume() noexcept -> void override;

  auto ChangePixelBlender() noexcept -> void override;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override;

  auto ApplyMultiple() noexcept -> void override;

private:
  class ImageFxImpl;
  spimpl::unique_impl_ptr<ImageFxImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX
