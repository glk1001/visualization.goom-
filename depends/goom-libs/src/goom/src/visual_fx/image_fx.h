#pragma once

#include "goom/spimpl.h"
#include "goom_visual_fx.h"

#include <memory>

namespace GOOM
{

namespace COLOR
{
class RandomColorMaps;
}

namespace UTILS
{
class Parallel;
}

namespace VISUAL_FX
{
class FxHelper;

class ImageFx : public IVisualFx
{
public:
  ImageFx(UTILS::Parallel& parallel,
          const FxHelper& fxHelper,
          const std::string& resourcesDirectory) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto Resume() noexcept -> void override;

  auto ApplyMultiple() noexcept -> void override;

private:
  class ImageFxImpl;
  spimpl::unique_impl_ptr<ImageFxImpl> m_pimpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
