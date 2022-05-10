#include "shape_paths.h"

#include "shape_parts.h"

namespace GOOM::VISUAL_FX::SHAPES
{

auto ShapePath::UpdateMainColorInfo(ShapePart& parentShapePart) const noexcept -> void
{
  parentShapePart.UpdateMainColorMapId(m_colorInfo.mainColorMapId);
}

auto ShapePath::UpdateLowColorInfo(ShapePart& parentShapePart) const noexcept -> void
{
  parentShapePart.UpdateLowColorMapId(m_colorInfo.lowColorMapId);
}

auto ShapePath::UpdateInnerColorInfo(ShapePart& parentShapePart) const noexcept -> void
{
  parentShapePart.UpdateInnerColorMapId(m_colorInfo.innerColorMapId);
}

} // namespace GOOM::VISUAL_FX::SHAPES
