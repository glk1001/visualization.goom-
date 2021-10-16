#include "goom_random_states.h"

#include "utils/enumutils.h"

#undef NDEBUG
#include <cassert>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

using UTILS::NUM;
using UTILS::Weights;

const Weights<GoomStates> GoomRandomStates::WEIGHTED_STATES{{
    {GoomStates::DOTS_IFS, 200},
    {GoomStates::DOTS_IFS_STARS, 100},
    {GoomStates::DOTS_IMAGE_STARS, 50},
    {GoomStates::DOTS_LINES, 60},
    {GoomStates::DOTS_LINES_STAR_TENTACLES, 40},
    {GoomStates::DOTS_LINES_TENTACLES_TUBES, 40},
    {GoomStates::DOTS_LINES_TUBES, 60},
    {GoomStates::DOTS_ONLY, 1},
    {GoomStates::DOTS_STARS, 60},
    {GoomStates::DOTS_STARS_TENTACLES_TUBES, 100},
    {GoomStates::DOTS_TENTACLES_TUBES, 200},

    {GoomStates::IFS_IMAGE, 200},
    {GoomStates::IFS_LINES_STARS, 60},
    {GoomStates::IFS_ONLY, 1},
    {GoomStates::IFS_STARS, 200},
    {GoomStates::IFS_STARS_TENTACLES, 20},
    {GoomStates::IFS_TENTACLES, 70},
    {GoomStates::IFS_TENTACLES_TUBES, 70},
    {GoomStates::IFS_TUBES, 200},

    {GoomStates::IMAGE_LINES, 200},
    {GoomStates::IMAGE_LINES_STARS_TENTACLES, 100},
    {GoomStates::IMAGE_ONLY, 1},
    {GoomStates::IMAGE_STARS, 200},
    {GoomStates::IMAGE_TENTACLES, 70},
    {GoomStates::IMAGE_TUBES, 200},

    {GoomStates::LINES_ONLY, 1},
    {GoomStates::LINES_STARS, 60},
    {GoomStates::LINES_TENTACLES, 200},

    {GoomStates::STARS_ONLY, 1},

    {GoomStates::TENTACLES_ONLY, 1},

    {GoomStates::TUBES_ONLY, 1},
}};

GoomRandomStates::GoomRandomStates()
{
 assert(WEIGHTED_STATES.GetNumElements() == NUM<GoomStates>);
}

void GoomRandomStates::DoRandomStateChange()
{
  m_currentState = WEIGHTED_STATES.GetRandomWeighted();
}

auto GoomRandomStates::GetCurrentDrawables() const -> GoomRandomStates::DrawablesState
{
  GoomRandomStates::DrawablesState currentDrawables{};
  for (const auto drawableInfo : GoomStateInfo::GetStateInfo(m_currentState).drawablesInfo)
  {
    currentDrawables.insert(drawableInfo.fx);
  }
  return currentDrawables;
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
