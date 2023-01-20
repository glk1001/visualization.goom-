#undef NO_LOGGING

#include "line_drawer.h"

#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_logger.h"
#include "line_draw_thick.h"
#include "line_draw_wu.h"
#include "utils/debugging_logger.h"

#include <cstdint>
#include <vector>

namespace GOOM::DRAW::SHAPE_DRAWERS
{

using THICK_LINES::DrawVariableThicknessLine;
using WU_LINES::WuLine;

} // namespace GOOM::DRAW::SHAPE_DRAWERS
