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
class FxHelpers;

class ImageFx : public IVisualFx
{
public:
  ImageFx(UTILS::Parallel& parallel,
          const FxHelpers& fxHelpers,
          const std::string& resourcesDirectory) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  void Start() override;

  void ApplyMultiple();

  void Finish() override;

private:
  class ImageFxImpl;
  spimpl::unique_impl_ptr<ImageFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
