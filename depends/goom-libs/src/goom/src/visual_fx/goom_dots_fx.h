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

class GoomDotsFx : public IVisualFx
{
public:
  GoomDotsFx() noexcept = delete;
  GoomDotsFx(const FxHelper& fxHelper,
             const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void Start() override;

  void Resume() override;
  void Suspend() override;

  static constexpr uint32_t NUM_DOT_TYPES = 5;
  void SetWeightedColorMaps(uint32_t dotNum, std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  void ApplySingle();
  void ApplyMultiple();

  void Finish() override;

private:
  class GoomDotsFxImpl;
  spimpl::unique_impl_ptr<GoomDotsFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
