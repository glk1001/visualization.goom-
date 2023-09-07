#include "stars.h"

#include "goom/point2d.h"
#include "star_colors.h"

namespace GOOM::VISUAL_FX::FLYING_STARS
{

Star::Star(const Params& params, const StarColors& starColors) noexcept
  : m_params{params}, m_starColors{starColors}
{
}

/**
 * Met a jour la position et vitesse d'une particule.
 */
auto Star::Update() noexcept -> void
{
  m_params.currentPosition = Translate(m_params.currentPosition, m_params.velocity);
  m_params.velocity        = m_params.velocity + m_params.acceleration;
  m_params.tAge += m_params.tAgeInc;
}

} // namespace GOOM::VISUAL_FX::FLYING_STARS
