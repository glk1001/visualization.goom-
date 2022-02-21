#pragma once

#include <string>

namespace GOOM
{
struct Point2dInt;

namespace VISUAL_FX
{

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

  virtual void Resume(){/* default does nothing */};
  virtual void Suspend(){/* default does nothing */};

  virtual void Refresh(){/* default does nothing */};
  virtual void PostStateUpdate([[maybe_unused]] const bool wasActiveInPreviousState){
      /* default does nothing */};
  virtual void SetZoomMidpoint([[maybe_unused]] const Point2dInt& zoomMidpoint){
      /* default does nothing */};

  virtual void Finish() = 0;
};

} // namespace VISUAL_FX
} // namespace GOOM
