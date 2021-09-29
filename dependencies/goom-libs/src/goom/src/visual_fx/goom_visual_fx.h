#pragma once

#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
class V2dInt;

namespace VISUAL_FX
{
#else
namespace GOOM::VISUAL_FX
{
#endif

class IVisualFx
{
public:
  IVisualFx() = default;
  IVisualFx(const IVisualFx&) noexcept = delete;
  IVisualFx(IVisualFx&&) noexcept = delete;
  virtual ~IVisualFx() = default;
  auto operator=(const IVisualFx&) -> IVisualFx& = delete;
  auto operator=(IVisualFx&&) -> IVisualFx& = delete;

  [[nodiscard]] virtual auto GetFxName() const -> std::string = 0;

  virtual void Start() = 0;

  virtual void Resume(){};
  virtual void Suspend(){};

  virtual void Refresh(){};
  virtual void PostStateUpdate([[maybe_unused]] const bool wasActiveInPreviousState){};
  virtual void SetZoomMidPoint([[maybe_unused]] const V2dInt& zoomMidPoint){};

  virtual void Finish() = 0;
};

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
