#pragma once

#include "color_data/color_map_enums.h"
#include "goom_graphic.h"
#include "utils/enum_utils.h"

#include <cstddef>

namespace GOOM::COLOR
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
