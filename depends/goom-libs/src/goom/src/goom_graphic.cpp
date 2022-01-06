#include "goom_graphic.h"

namespace GOOM
{

const Pixel Pixel::BLACK{
    {/*.red = */ 0, /*.green = */ 0, /*.blue = */ 0, /*.alpha = */ 0}
};

const Pixel Pixel::WHITE{
    {/*.red = */ MAX_COLOR_VAL, /*.green = */ MAX_COLOR_VAL,
     /*.blue = */ MAX_COLOR_VAL, /*.alpha = */ MAX_ALPHA}
};

} // namespace GOOM
