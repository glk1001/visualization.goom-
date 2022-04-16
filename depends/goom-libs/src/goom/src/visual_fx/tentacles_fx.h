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
class FxHelper;

class TentaclesFx : public IVisualFx
{
public:
  TentaclesFx() noexcept = delete;
  TentaclesFx(const FxHelper& fxHelper,
              const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  auto SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps) -> void;

  auto Start() -> void override;

  auto Resume() -> void override;
  auto Suspend() -> void override;

  auto ApplyMultiple() -> void;

  auto Finish() -> void override;

private:
  class TentaclesImpl;
  spimpl::unique_impl_ptr<TentaclesImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
