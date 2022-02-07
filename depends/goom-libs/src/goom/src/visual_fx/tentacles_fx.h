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
namespace UTILS::GRAPHICS
{
class SmallImageBitmaps;
}

namespace VISUAL_FX
{
class FxHelpers;

class TentaclesFx : public IVisualFx
{
public:
  TentaclesFx() noexcept = delete;
  TentaclesFx(const FxHelpers& fxHelpers,
              const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  void Start() override;

  void Resume() override;
  void Suspend() override;

  void ApplyMultiple();

  void Finish() override;

private:
  class TentaclesImpl;
  spimpl::unique_impl_ptr<TentaclesImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
