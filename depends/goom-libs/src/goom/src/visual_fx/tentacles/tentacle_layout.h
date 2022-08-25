#pragma once

#include "tentacle3d.h"

#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

class ITentacleLayout
{
public:
  ITentacleLayout() noexcept                                 = default;
  ITentacleLayout(const ITentacleLayout&) noexcept           = default;
  ITentacleLayout(ITentacleLayout&&) noexcept                = delete;
  virtual ~ITentacleLayout() noexcept                        = default;
  auto operator=(const ITentacleLayout&) -> ITentacleLayout& = delete;
  auto operator=(ITentacleLayout&&) -> ITentacleLayout&      = delete;

  [[nodiscard]] virtual auto GetNumPoints() const -> size_t                       = 0;
  [[nodiscard]] virtual auto GetStartPoints() const -> const std::vector<V3dFlt>& = 0;
  [[nodiscard]] virtual auto GetEndPoints() const -> const std::vector<V3dFlt>&   = 0;
};

} // namespace GOOM::VISUAL_FX::TENTACLES
