#include "goom_random_state_handler.h"

#include "utils/enumutils.h"

#undef NDEBUG
#include <cassert>

namespace GOOM::CONTROL
{

using UTILS::NUM;
using UTILS::MATH::IGoomRand;


// For debugging:
constexpr bool USE_FORCED_GOOM_STATE = false;

constexpr GoomStates FORCED_GOOM_STATE = GoomStates::CIRCLES_ONLY;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::DOTS_IFS;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::DOTS_IFS_STARS;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::DOTS_IMAGE_STARS;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::DOTS_LINES;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::DOTS_LINES_STAR_TENTACLES;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::DOTS_LINES_TENTACLES_TUBES;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::DOTS_LINES_TUBES;
//constexpr GoomStates FORCED_GOOM_STATE = GoomStates::DOTS_ONLY;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::DOTS_STARS;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::DOTS_STARS_TENTACLES_TUBES;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::DOTS_TENTACLES_TUBES;
//constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IFS_IMAGE;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IFS_LINES_STARS;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IFS_ONLY;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IFS_STARS;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IFS_STARS_TENTACLES;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IFS_TENTACLES;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IFS_TENTACLES_TUBES;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IFS_TUBES;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IMAGE_LINES;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IMAGE_LINES_STARS_TENTACLES;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IMAGE_ONLY;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IMAGE_STARS;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IMAGE_TENTACLES;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::IMAGE_TUBES;
//constexpr GoomStates FORCED_GOOM_STATE = GoomStates::LINES_ONLY;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::LINES_STARS;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::LINES_TENTACLES;
// constexpr GoomStates FORCED_GOOM_STATE = GoomStates::STARS_ONLY;
//constexpr GoomStates FORCED_GOOM_STATE = GoomStates::TENTACLES_ONLY;
//constexpr GoomStates FORCED_GOOM_STATE = GoomStates::TUBES_ONLY;
// End debugging


// clang-format off
constexpr float CIRCLES_ONLY_WEIGHT                =    1.0F;
constexpr float CIRCLES_IFS_WEIGHT                 =  200.0F;
constexpr float CIRCLES_IMAGE_WEIGHT               =   50.0F;
constexpr float CIRCLES_IMAGE_STARS_WEIGHT         =  200.0F;
constexpr float CIRCLES_LINES_WEIGHT               =  200.0F;
constexpr float CIRCLES_STARS_TUBES_WEIGHT         =  100.0F;
constexpr float CIRCLES_TENTACLES_WEIGHT           =  200.0F;

constexpr float DOTS_IFS_WEIGHT                    = 200.0F;
constexpr float DOTS_IFS_STARS_WEIGHT              = 100.0F;
constexpr float DOTS_IMAGE_STARS_WEIGHT            =  50.0F;
constexpr float DOTS_LINES_WEIGHT                  =  50.0F;
constexpr float DOTS_LINES_STAR_TENTACLES_WEIGHT   =  40.0F;
constexpr float DOTS_LINES_TENTACLES_TUBES_WEIGHT  =  40.0F;
constexpr float DOTS_LINES_TUBES_WEIGHT            =  60.0F;
constexpr float DOTS_ONLY_WEIGHT                   =   1.0F;
constexpr float DOTS_STARS_WEIGHT                  = 100.0F;
constexpr float DOTS_STARS_TENTACLES_TUBES_WEIGHT  = 100.0F;
constexpr float DOTS_TENTACLES_TUBES_WEIGHT        = 200.0F;

constexpr float IFS_IMAGE_WEIGHT                   =  10.0F;
constexpr float IFS_LINES_STARS_WEIGHT             = 200.0F;
constexpr float IFS_ONLY_WEIGHT                    = 100.0F;
constexpr float IFS_STARS_WEIGHT                   = 200.0F;
constexpr float IFS_STARS_TENTACLES_WEIGHT         =  70.0F;
constexpr float IFS_TENTACLES_WEIGHT               =  70.0F;
constexpr float IFS_TENTACLES_TUBES_WEIGHT         =  70.0F;
constexpr float IFS_TUBES_WEIGHT                   = 200.0F;

constexpr float IMAGE_LINES_WEIGHT                 = 100.0F;
constexpr float IMAGE_LINES_STARS_TENTACLES_WEIGHT = 100.0F;
constexpr float IMAGE_ONLY_WEIGHT                  =   1.0F;
constexpr float IMAGE_STARS_WEIGHT                 = 200.0F;
constexpr float IMAGE_TENTACLES_WEIGHT             =  70.0F;
constexpr float IMAGE_TUBES_WEIGHT                 = 200.0F;

constexpr float LINES_ONLY_WEIGHT                  =   1.0F;
constexpr float LINES_STARS_WEIGHT                 = 100.0F;
constexpr float LINES_TENTACLES_WEIGHT             = 200.0F;

constexpr float STARS_ONLY_WEIGHT                  =   1.0F;
constexpr float TENTACLES_ONLY_WEIGHT              =   1.0F;
constexpr float TUBES_ONLY_WEIGHT                  =   1.0F;
// clang-format on


GoomRandomStateHandler::GoomRandomStateHandler(const IGoomRand& goomRand)
  // clang-format off
  : m_weightedStates{
        goomRand,
        {
            {GoomStates::CIRCLES_ONLY, CIRCLES_ONLY_WEIGHT},
            {GoomStates::CIRCLES_IFS, CIRCLES_IFS_WEIGHT},
            {GoomStates::CIRCLES_IMAGE, CIRCLES_IMAGE_WEIGHT},
            {GoomStates::CIRCLES_IMAGE_STARS, CIRCLES_IMAGE_STARS_WEIGHT},
            {GoomStates::CIRCLES_LINES, CIRCLES_LINES_WEIGHT},
            {GoomStates::CIRCLES_STARS_TUBES, CIRCLES_STARS_TUBES_WEIGHT},
            {GoomStates::CIRCLES_TENTACLES, CIRCLES_TENTACLES_WEIGHT},

            {GoomStates::DOTS_IFS, DOTS_IFS_WEIGHT},
            {GoomStates::DOTS_IFS_STARS, DOTS_IFS_STARS_WEIGHT},
            {GoomStates::DOTS_IMAGE_STARS, DOTS_IMAGE_STARS_WEIGHT},
            {GoomStates::DOTS_LINES, DOTS_LINES_WEIGHT},
            {GoomStates::DOTS_LINES_STARS_TENTACLES, DOTS_LINES_STAR_TENTACLES_WEIGHT},
            {GoomStates::DOTS_LINES_TENTACLES_TUBES, DOTS_LINES_TENTACLES_TUBES_WEIGHT},
            {GoomStates::DOTS_LINES_TUBES, DOTS_LINES_TUBES_WEIGHT},
            {GoomStates::DOTS_ONLY, DOTS_ONLY_WEIGHT},
            {GoomStates::DOTS_STARS, DOTS_STARS_WEIGHT},
            {GoomStates::DOTS_STARS_TENTACLES_TUBES, DOTS_STARS_TENTACLES_TUBES_WEIGHT},
            {GoomStates::DOTS_TENTACLES_TUBES, DOTS_TENTACLES_TUBES_WEIGHT},

            {GoomStates::IFS_IMAGE, IFS_IMAGE_WEIGHT},
            {GoomStates::IFS_LINES_STARS, IFS_LINES_STARS_WEIGHT},
            {GoomStates::IFS_ONLY, IFS_ONLY_WEIGHT},
            {GoomStates::IFS_STARS, IFS_STARS_WEIGHT},
            {GoomStates::IFS_STARS_TENTACLES, IFS_STARS_TENTACLES_WEIGHT},
            {GoomStates::IFS_TENTACLES, IFS_TENTACLES_WEIGHT},
            {GoomStates::IFS_TENTACLES_TUBES, IFS_TENTACLES_TUBES_WEIGHT},
            {GoomStates::IFS_TUBES, IFS_TUBES_WEIGHT},

            {GoomStates::IMAGE_LINES, IMAGE_LINES_WEIGHT},
            {GoomStates::IMAGE_LINES_STARS_TENTACLES, IMAGE_LINES_STARS_TENTACLES_WEIGHT},
            {GoomStates::IMAGE_ONLY, IMAGE_ONLY_WEIGHT},
            {GoomStates::IMAGE_STARS, IMAGE_STARS_WEIGHT},
            {GoomStates::IMAGE_TENTACLES, IMAGE_TENTACLES_WEIGHT},
            {GoomStates::IMAGE_TUBES, IMAGE_TUBES_WEIGHT},

            {GoomStates::LINES_ONLY, LINES_ONLY_WEIGHT},
            {GoomStates::LINES_STARS, LINES_STARS_WEIGHT},
            {GoomStates::LINES_TENTACLES, LINES_TENTACLES_WEIGHT},

            {GoomStates::STARS_ONLY, STARS_ONLY_WEIGHT},

            {GoomStates::TENTACLES_ONLY, TENTACLES_ONLY_WEIGHT},

            {GoomStates::TUBES_ONLY, TUBES_ONLY_WEIGHT},
        }
    }
// clang-format on
{
  assert(m_weightedStates.GetNumElements() == NUM<GoomStates>);
}

void GoomRandomStateHandler::ChangeToNextState()
{
  if constexpr (USE_FORCED_GOOM_STATE)
  {
    m_currentState = FORCED_GOOM_STATE;
    return;
  }

  m_currentState = m_weightedStates.GetRandomWeighted();
}

} // namespace GOOM::CONTROL
