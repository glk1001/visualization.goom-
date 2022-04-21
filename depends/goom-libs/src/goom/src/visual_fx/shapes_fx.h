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

namespace VISUAL_FX
{
class FxHelper;

class ShapesFx : public IVisualFx
{
public:
  static constexpr uint32_t NUM_SHAPES = 5;

  explicit ShapesFx(const FxHelper& fxHelper) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void Start() override;
  void Finish() override;

  void SetWeightedColorMaps(uint32_t shapeNum,
                            std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);
  void SetWeightedLowColorMaps(uint32_t shapeNum,
                               std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  void ApplyMultiple();

private:
  class ShapesFxImpl;
  spimpl::unique_impl_ptr<ShapesFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
