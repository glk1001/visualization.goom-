#include "goom_graphic.h"

namespace GOOM
{

#if __cplusplus <= 201402L
const Pixel Pixel::BLACK{/*.channels*/ {/*.r = */ 0, /*.g = */ 0, /*.b = */ 0, /*.a = */ 0}};

const Pixel Pixel::WHITE{/*.channels*/ {/*.r = */ MAX_COLOR_VAL, /*.g = */ MAX_COLOR_VAL,
                                        /*.b = */ MAX_COLOR_VAL, /*.a = */ MAX_COLOR_VAL}};
#endif

} // namespace GOOM
