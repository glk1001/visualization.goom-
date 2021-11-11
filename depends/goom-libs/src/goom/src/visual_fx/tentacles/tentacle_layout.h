#pragma once

#include "tentacle3d.h"

#include <cstdint>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace TENTACLES
{
#else
namespace GOOM::VISUAL_FX::TENTACLES
{
#endif

class ITentacleLayout
{
public:
  ITentacleLayout() noexcept = default;
  ITentacleLayout(const ITentacleLayout&) noexcept = default;
  ITentacleLayout(ITentacleLayout&&) noexcept = delete;
  virtual ~ITentacleLayout() noexcept = default;
  auto operator=(const ITentacleLayout&) -> ITentacleLayout& = delete;
  auto operator=(ITentacleLayout&&) -> ITentacleLayout& = delete;

  [[nodiscard]] virtual auto GetNumPoints() const -> size_t = 0;
  [[nodiscard]] virtual auto GetPoints() const -> const std::vector<V3dFlt>& = 0;
};

#if __cplusplus <= 201402L
} // namespace TENTACLES
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::TENTACLES
#endif
