module Goom.Control.GoomRandomStateHandler;

import Goom.Control.GoomStates;
import Goom.Control.StateAndFilterConsts;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.AssertUtils;

namespace GOOM::CONTROL
{

using CONTROL::USE_FORCED_GOOM_STATE;
using UTILS::NUM;
using UTILS::MATH::IGoomRand;


// For debugging:

//static constexpr auto FORCED_GOOM_STATE = GoomStates::CIRCLES_ONLY;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::DOTS_IFS;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::DOTS_IFS_RAINDROPS;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::DOTS_IFS_STARS;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::DOTS_IMAGE_STARS;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::DOTS_LINES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::DOTS_LINES_STAR_TENTACLES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::DOTS_LINES_TENTACLES_TUBES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::DOTS_LINES_TUBES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::DOTS_ONLY;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::DOTS_STARS;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::DOTS_STARS_TENTACLES_TUBES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::DOTS_TENTACLES_TUBES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IFS_IMAGE;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IFS_LINES_STARS;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IFS_ONLY;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IFS_PARTICLES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IFS_STARS;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IFS_STARS_TENTACLES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IFS_TENTACLES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IFS_TENTACLES_TUBES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IFS_TUBES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IMAGE_LINES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IMAGE_LINES_STARS_TENTACLES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IMAGE_ONLY;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IMAGE_RAINDROPS;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IMAGE_RAINDROPS_SHAPES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IMAGE_STARS;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IMAGE_TENTACLES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::IMAGE_TUBES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::LINES_ONLY;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::LINES_PARTICLES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::LINES_STARS;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::LINES_TENTACLES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::L_SYSTEM_ONLY;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::PARTICLES_ONLY;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::PARTICLES_TENTACLES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::PARTICLES_TUBES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::RAINDROPS_ONLY;
static constexpr auto FORCED_GOOM_STATE = GoomStates::SHAPES_ONLY;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::SHAPES_STARS_LINES;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::STARS_ONLY;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::TENTACLES_ONLY;
//static constexpr auto FORCED_GOOM_STATE = GoomStates::TUBES_ONLY;

// End debugging


static constexpr auto CIRCLES_ONLY_WEIGHT                 = 050.0F;
static constexpr auto CIRCLES_IFS_WEIGHT                  = 200.0F;
static constexpr auto CIRCLES_IMAGE_WEIGHT                = 050.0F;
static constexpr auto CIRCLES_IMAGE_STARS_WEIGHT          = 200.0F;
static constexpr auto CIRCLES_IMAGE_STARS_L_SYSTEM_WEIGHT = 300.0F;
static constexpr auto CIRCLES_LINES_WEIGHT                = 200.0F;
static constexpr auto CIRCLES_STARS_TUBES_WEIGHT          = 100.0F;
static constexpr auto CIRCLES_TENTACLES_WEIGHT            = 200.0F;
static constexpr auto DOTS_IFS_WEIGHT                     = 300.0F;
static constexpr auto DOTS_IFS_RAINDROPS_WEIGHT           = 100.0F;
static constexpr auto DOTS_IFS_STARS_WEIGHT               = 300.0F;
static constexpr auto DOTS_IMAGE_RAINDROPS_WEIGHT         = 050.0F;
static constexpr auto DOTS_IMAGE_RAINDROPS_STARS_WEIGHT   = 100.0F;
static constexpr auto DOTS_IMAGE_STARS_WEIGHT             = 200.0F;
static constexpr auto DOTS_IMAGE_STARS_L_SYSTEM_WEIGHT    = 001.0F;
static constexpr auto DOTS_LINES_WEIGHT                   = 200.0F;
static constexpr auto DOTS_LINES_RAINDROPS_STARS_WEIGHT   = 040.0F;
static constexpr auto DOTS_LINES_STARS_TENTACLES_WEIGHT   = 001.0F;
static constexpr auto DOTS_LINES_TENTACLES_TUBES_WEIGHT   = 001.0F;
static constexpr auto DOTS_LINES_TUBES_WEIGHT             = 200.0F;
static constexpr auto DOTS_ONLY_WEIGHT                    = 300.0F;
static constexpr auto DOTS_RAINDROPS_STARS_WEIGHT         = 100.0F;
static constexpr auto DOTS_STARS_WEIGHT                   = 100.0F;
static constexpr auto DOTS_STARS_L_SYSTEM_WEIGHT          = 050.0F;
static constexpr auto DOTS_STARS_TENTACLES_TUBES_WEIGHT   = 001.0F;
static constexpr auto DOTS_TENTACLES_TUBES_WEIGHT         = 001.0F;
static constexpr auto IFS_IMAGE_WEIGHT                    = 100.0F;
static constexpr auto IFS_IMAGE_RAINDROPS_SHAPES_WEIGHT   = 100.0F;
static constexpr auto IFS_IMAGE_SHAPES_WEIGHT             = 100.0F;
static constexpr auto IFS_LINES_RAINDROPS_STARS_WEIGHT    = 200.0F;
static constexpr auto IFS_LINES_STARS_WEIGHT              = 200.0F;
static constexpr auto IFS_ONLY_WEIGHT                     = 100.0F;
static constexpr auto IFS_PARTICLES_WEIGHT                = 300.0F;
static constexpr auto IFS_RAINDROPS_WEIGHT                = 100.0F;
static constexpr auto IFS_RAINDROPS_SHAPES_WEIGHT         = 100.0F;
static constexpr auto IFS_RAINDROPS_STARS_WEIGHT          = 100.0F;
static constexpr auto IFS_SHAPES_WEIGHT                   = 100.0F;
static constexpr auto IFS_STARS_WEIGHT                    = 200.0F;
static constexpr auto IFS_STARS_TENTACLES_WEIGHT          = 070.0F;
static constexpr auto IFS_TENTACLES_WEIGHT                = 070.0F;
static constexpr auto IFS_TENTACLES_TUBES_WEIGHT          = 070.0F;
static constexpr auto IFS_TUBES_WEIGHT                    = 200.0F;
static constexpr auto IMAGE_LINES_WEIGHT                  = 100.0F;
static constexpr auto IMAGE_LINES_RAINDROPS_WEIGHT        = 100.0F;
static constexpr auto IMAGE_LINES_SHAPES_WEIGHT           = 100.0F;
static constexpr auto IMAGE_LINES_STARS_TENTACLES_WEIGHT  = 100.0F;
static constexpr auto IMAGE_ONLY_WEIGHT                   = 001.0F;
static constexpr auto IMAGE_RAINDROPS_WEIGHT              = 100.0F;
static constexpr auto IMAGE_RAINDROPS_SHAPES_WEIGHT       = 100.0F;
static constexpr auto IMAGE_SHAPES_WEIGHT                 = 100.0F;
static constexpr auto IMAGE_SHAPES_L_SYSTEM_WEIGHT        = 100.0F;
static constexpr auto IMAGE_SHAPES_STARS_WEIGHT           = 100.0F;
static constexpr auto IMAGE_SHAPES_TUBES_WEIGHT           = 100.0F;
static constexpr auto IMAGE_STARS_WEIGHT                  = 200.0F;
static constexpr auto IMAGE_STARS_L_SYSTEM_WEIGHT         = 250.0F;
static constexpr auto IMAGE_TENTACLES_WEIGHT              = 070.0F;
static constexpr auto IMAGE_TUBES_WEIGHT                  = 200.0F;
static constexpr auto L_SYSTEM_ONLY_WEIGHT                = 100.0F;
static constexpr auto LINES_ONLY_WEIGHT                   = 100.0F;
static constexpr auto LINES_PARTICLES_WEIGHT              = 300.0F;
static constexpr auto LINES_RAINDROPS_WEIGHT              = 100.0F;
static constexpr auto LINES_SHAPES_STARS_WEIGHT           = 200.0F;
static constexpr auto LINES_STARS_WEIGHT                  = 100.0F;
static constexpr auto LINES_TENTACLES_WEIGHT              = 200.0F;
static constexpr auto PARTICLES_ONLY_WEIGHT               = 100.0F;
static constexpr auto PARTICLES_TENTACLES_WEIGHT          = 300.0F;
static constexpr auto PARTICLES_TUBES_WEIGHT              = 300.0F;
static constexpr auto RAINDROPS_ONLY_WEIGHT               = 010.0F;
static constexpr auto SHAPES_ONLY_WEIGHT                  = 001.0F;
static constexpr auto SHAPES_STARS_WEIGHT                 = 200.0F;
static constexpr auto SHAPES_TUBES_WEIGHT                 = 200.0F;
static constexpr auto STARS_ONLY_WEIGHT                   = 001.0F;
static constexpr auto TENTACLES_ONLY_WEIGHT               = 001.0F;
static constexpr auto TUBES_ONLY_WEIGHT                   = 001.0F;


GoomRandomStateHandler::GoomRandomStateHandler(const IGoomRand& goomRand)
  : m_weightedStates{
        goomRand,
        {
          {GoomStates::CIRCLES_ONLY, CIRCLES_ONLY_WEIGHT},
          {GoomStates::CIRCLES_IFS, CIRCLES_IFS_WEIGHT},
          {GoomStates::CIRCLES_IMAGE, CIRCLES_IMAGE_WEIGHT},
          {GoomStates::CIRCLES_IMAGE_STARS, CIRCLES_IMAGE_STARS_WEIGHT},
          {GoomStates::CIRCLES_IMAGE_STARS_L_SYSTEM, CIRCLES_IMAGE_STARS_L_SYSTEM_WEIGHT},
          {GoomStates::CIRCLES_LINES, CIRCLES_LINES_WEIGHT},
          {GoomStates::CIRCLES_STARS_TUBES, CIRCLES_STARS_TUBES_WEIGHT},
          {GoomStates::CIRCLES_TENTACLES, CIRCLES_TENTACLES_WEIGHT},

          {GoomStates::DOTS_IFS, DOTS_IFS_WEIGHT},
          {GoomStates::DOTS_IFS_RAINDROPS, DOTS_IFS_RAINDROPS_WEIGHT},
          {GoomStates::DOTS_IFS_STARS, DOTS_IFS_STARS_WEIGHT},
          {GoomStates::DOTS_IMAGE_RAINDROPS, DOTS_IMAGE_RAINDROPS_WEIGHT},
          {GoomStates::DOTS_IMAGE_RAINDROPS_STARS, DOTS_IMAGE_RAINDROPS_STARS_WEIGHT},
          {GoomStates::DOTS_IMAGE_STARS, DOTS_IMAGE_STARS_WEIGHT},
          {GoomStates::DOTS_IMAGE_STARS_L_SYSTEM, DOTS_IMAGE_STARS_L_SYSTEM_WEIGHT},
          {GoomStates::DOTS_LINES, DOTS_LINES_WEIGHT},
          {GoomStates::DOTS_LINES_RAINDROPS_STARS, DOTS_LINES_RAINDROPS_STARS_WEIGHT},
          {GoomStates::DOTS_LINES_STARS_TENTACLES, DOTS_LINES_STARS_TENTACLES_WEIGHT},
          {GoomStates::DOTS_LINES_TENTACLES_TUBES, DOTS_LINES_TENTACLES_TUBES_WEIGHT},
          {GoomStates::DOTS_LINES_TUBES, DOTS_LINES_TUBES_WEIGHT},
          {GoomStates::DOTS_ONLY, DOTS_ONLY_WEIGHT},
          {GoomStates::DOTS_RAINDROPS_STARS, DOTS_RAINDROPS_STARS_WEIGHT},
          {GoomStates::DOTS_STARS, DOTS_STARS_WEIGHT},
          {GoomStates::DOTS_STARS_L_SYSTEM, DOTS_STARS_L_SYSTEM_WEIGHT},
          {GoomStates::DOTS_STARS_TENTACLES_TUBES, DOTS_STARS_TENTACLES_TUBES_WEIGHT},
          {GoomStates::DOTS_TENTACLES_TUBES, DOTS_TENTACLES_TUBES_WEIGHT},

          {GoomStates::IFS_IMAGE, IFS_IMAGE_WEIGHT},
          {GoomStates::IFS_IMAGE_RAINDROPS_SHAPES, IFS_IMAGE_RAINDROPS_SHAPES_WEIGHT},
          {GoomStates::IFS_IMAGE_SHAPES, IFS_IMAGE_SHAPES_WEIGHT},
          {GoomStates::IFS_LINES_RAINDROPS_STARS, IFS_LINES_RAINDROPS_STARS_WEIGHT},
          {GoomStates::IFS_LINES_STARS, IFS_LINES_STARS_WEIGHT},
          {GoomStates::IFS_ONLY, IFS_ONLY_WEIGHT},
          {GoomStates::IFS_PARTICLES, IFS_PARTICLES_WEIGHT},
          {GoomStates::IFS_RAINDROPS, IFS_RAINDROPS_WEIGHT},
          {GoomStates::IFS_RAINDROPS_SHAPES, IFS_RAINDROPS_SHAPES_WEIGHT},
          {GoomStates::IFS_RAINDROPS_STARS, IFS_RAINDROPS_STARS_WEIGHT},
          {GoomStates::IFS_SHAPES, IFS_SHAPES_WEIGHT},
          {GoomStates::IFS_STARS, IFS_STARS_WEIGHT},
          {GoomStates::IFS_STARS_TENTACLES, IFS_STARS_TENTACLES_WEIGHT},
          {GoomStates::IFS_TENTACLES, IFS_TENTACLES_WEIGHT},
          {GoomStates::IFS_TENTACLES_TUBES, IFS_TENTACLES_TUBES_WEIGHT},
          {GoomStates::IFS_TUBES, IFS_TUBES_WEIGHT},

          {GoomStates::IMAGE_LINES, IMAGE_LINES_WEIGHT},
          {GoomStates::IMAGE_LINES_RAINDROPS, IMAGE_LINES_RAINDROPS_WEIGHT},
          {GoomStates::IMAGE_LINES_SHAPES, IMAGE_LINES_SHAPES_WEIGHT},
          {GoomStates::IMAGE_LINES_STARS_TENTACLES, IMAGE_LINES_STARS_TENTACLES_WEIGHT},
          {GoomStates::IMAGE_ONLY, IMAGE_ONLY_WEIGHT},
          {GoomStates::IMAGE_RAINDROPS, IMAGE_RAINDROPS_WEIGHT},
          {GoomStates::IMAGE_RAINDROPS_SHAPES, IMAGE_RAINDROPS_SHAPES_WEIGHT},
          {GoomStates::IMAGE_SHAPES, IMAGE_SHAPES_WEIGHT},
          {GoomStates::IMAGE_SHAPES_L_SYSTEM, IMAGE_SHAPES_L_SYSTEM_WEIGHT},
          {GoomStates::IMAGE_SHAPES_STARS, IMAGE_SHAPES_STARS_WEIGHT},
          {GoomStates::IMAGE_SHAPES_TUBES, IMAGE_SHAPES_TUBES_WEIGHT},
          {GoomStates::IMAGE_STARS, IMAGE_STARS_WEIGHT},
          {GoomStates::IMAGE_STARS_L_SYSTEM, IMAGE_STARS_L_SYSTEM_WEIGHT},
          {GoomStates::IMAGE_TENTACLES, IMAGE_TENTACLES_WEIGHT},
          {GoomStates::IMAGE_TUBES, IMAGE_TUBES_WEIGHT},

          {GoomStates::L_SYSTEM_ONLY, L_SYSTEM_ONLY_WEIGHT},

          {GoomStates::LINES_ONLY, LINES_ONLY_WEIGHT},
          {GoomStates::LINES_PARTICLES, LINES_PARTICLES_WEIGHT},
          {GoomStates::LINES_RAINDROPS, LINES_RAINDROPS_WEIGHT},
          {GoomStates::LINES_SHAPES_STARS, LINES_SHAPES_STARS_WEIGHT},
          {GoomStates::LINES_STARS, LINES_STARS_WEIGHT},
          {GoomStates::LINES_TENTACLES, LINES_TENTACLES_WEIGHT},

          {GoomStates::PARTICLES_ONLY, PARTICLES_ONLY_WEIGHT},
          {GoomStates::PARTICLES_TENTACLES, PARTICLES_TENTACLES_WEIGHT},
          {GoomStates::PARTICLES_TUBES, PARTICLES_TUBES_WEIGHT},

          {GoomStates::RAINDROPS_ONLY, RAINDROPS_ONLY_WEIGHT},
          {GoomStates::SHAPES_ONLY, SHAPES_ONLY_WEIGHT},
          {GoomStates::SHAPES_STARS, SHAPES_STARS_WEIGHT},
          {GoomStates::SHAPES_TUBES, SHAPES_TUBES_WEIGHT},

          {GoomStates::STARS_ONLY, STARS_ONLY_WEIGHT},

          {GoomStates::TENTACLES_ONLY, TENTACLES_ONLY_WEIGHT},

          {GoomStates::TUBES_ONLY, TUBES_ONLY_WEIGHT},
          }
}
{
  Ensures(m_weightedStates.GetNumElements() == NUM<GoomStates>);
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
