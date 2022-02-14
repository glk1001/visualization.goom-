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

class CirclesFx : public IVisualFx
{
public:
  CirclesFx(const FxHelper& fxHelper,
            const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps,
                            std::shared_ptr<COLOR::RandomColorMaps> weightedLowMaps);

  void SetZoomMidpoint(const Point2dInt& zoomMidpoint) override;

  void Start() override;

  void ApplyMultiple();

  void Finish() override;

private:
  class CirclesFxImpl;
  spimpl::unique_impl_ptr<CirclesFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
