module;

#include <cstddef>

export module Goom.Color.ColorMapBase;

import Goom.Color.ColorData.ColorMapEnums;
import Goom.Lib.GoomGraphic;

export namespace GOOM::COLOR
{

class IColorMap
{
public:
  IColorMap() noexcept                           = default;
  IColorMap(const IColorMap&) noexcept           = default;
  IColorMap(IColorMap&&) noexcept                = default;
  virtual ~IColorMap() noexcept                  = default;
  auto operator=(const IColorMap&) -> IColorMap& = default;
  auto operator=(IColorMap&&) -> IColorMap&      = default;

  [[nodiscard]] virtual auto GetNumStops() const -> size_t                  = 0;
  [[nodiscard]] virtual auto GetMapName() const -> COLOR_DATA::ColorMapName = 0;

  [[nodiscard]] virtual auto GetColor(float t) const -> Pixel = 0;
};

} // namespace GOOM::COLOR
