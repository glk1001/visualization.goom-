#pragma once

#include "goom/spimpl.h"
#include "goom_visual_fx.h"

#include <memory>
#include <string>
#include <vector>

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

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto Resume() noexcept -> void override;
  auto Suspend() noexcept -> void override;

  auto ApplyMultiple() noexcept -> void override;

private:
  class TentaclesImpl;
  spimpl::unique_impl_ptr<TentaclesImpl> m_pimpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
