#pragma once

#include "color/colormaps.h"
#include "goom_graphic.h"

#include <cstdint>

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

class ITentacleColorizer
{
public:
  ITentacleColorizer() noexcept = default;
  ITentacleColorizer(const ITentacleColorizer&) noexcept = delete;
  ITentacleColorizer(ITentacleColorizer&&) noexcept = delete;
  virtual ~ITentacleColorizer() noexcept = default;
  auto operator=(const ITentacleColorizer&) noexcept -> ITentacleColorizer& = delete;
  auto operator=(ITentacleColorizer&&) noexcept -> ITentacleColorizer& = delete;

  virtual void SetColorMapGroup(COLOR::ColorMapGroup colorMapGroup) = 0;
  virtual void ChangeColorMap() = 0;
  [[nodiscard]] virtual auto GetColor(size_t nodeNum) const -> Pixel = 0;
};

#if __cplusplus <= 201402L
} // namespace TENTACLES
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::TENTACLES
#endif
