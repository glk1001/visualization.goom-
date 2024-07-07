module;

#include <ranges>
#include <vector>

module Goom.Control.GoomFavouriteStatesHandler;

import Goom.Control.GoomDrawables;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;

namespace GOOM::CONTROL
{

using UTILS::EnumMap;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::NumberRange;

using enum GoomDrawables;

static constexpr auto BUFF_INTENSITY_RANGES = EnumMap<GoomDrawables, NumberRange<float>>{{{
    {GoomDrawables::CIRCLES, {0.50F, 0.80F}},
    {GoomDrawables::DOTS, {0.40F, 0.70F}},
    {GoomDrawables::IFS, {0.40F, 0.70F}},
    {GoomDrawables::L_SYSTEM, {0.70F, 0.80F}},
    {GoomDrawables::LINES, {0.50F, 0.70F}},
    {GoomDrawables::IMAGE, {0.05F, 0.30F}},
    {GoomDrawables::PARTICLES, {0.50F, 0.80F}},
    {GoomDrawables::RAINDROPS, {0.60F, 0.80F}},
    {GoomDrawables::SHAPES, {0.50F, 0.80F}},
    {GoomDrawables::STARS, {0.50F, 0.60F}},
    {GoomDrawables::TENTACLES, {0.30F, 0.50F}},
    {GoomDrawables::TUBES, {0.70F, 0.80F}},
}}};

const EnumMap<GoomFavouriteStatesHandler::Favourites, std::vector<GoomDrawables>>
    // NOLINTNEXTLINE(cert-err58-cpp)
    GoomFavouriteStatesHandler::FAVOURITE_DRAWABLES{{{
        {Favourites::DOTS_ONLY, {DOTS}},
        {Favourites::DOTS_PARTICLES, {DOTS, PARTICLES}},
        {Favourites::DOTS_IFS, {DOTS, IFS}},
        {Favourites::PARTICLES_ONLY, {PARTICLES}},
        {Favourites::PARTICLES_IFS, {PARTICLES, IFS}},
    }}};

static constexpr auto DOTS_ONLY_WEIGHT      = 1.0F;
static constexpr auto DOTS_PARTICLES_WEIGHT = 1.0F;
static constexpr auto DOTS_IFS_WEIGHT       = 1.0F;
static constexpr auto PARTICLES_ONLY_WEIGHT = 1.0F;
static constexpr auto PARTICLES_IFS_WEIGHT  = 1.0F;

GoomFavouriteStatesHandler::GoomFavouriteStatesHandler(const IGoomRand& goomRand)
:  m_goomRand{&goomRand},
   m_weightedFavourites{
        goomRand,
        {
          {Favourites::DOTS_ONLY,      DOTS_ONLY_WEIGHT},
          {Favourites::DOTS_PARTICLES, DOTS_PARTICLES_WEIGHT},
          {Favourites::DOTS_IFS,       DOTS_IFS_WEIGHT},
          {Favourites::PARTICLES_ONLY, PARTICLES_ONLY_WEIGHT},
          {Favourites::PARTICLES_IFS,  PARTICLES_IFS_WEIGHT},
        }
  }
{
  ChangeToFavourite(m_weightedFavourites.GetRandomWeighted());
}

auto GoomFavouriteStatesHandler::ChangeToNextState() -> void
{
  auto favourite                  = m_weightedFavourites.GetRandomWeighted();
  static constexpr auto MAX_TRIES = 10U;
  for (auto i = 0U; i < MAX_TRIES; ++i)
  {
    if (m_currentFavourite != favourite)
    {
      break;
    }
    favourite = m_weightedFavourites.GetRandomWeighted();
  }

  ChangeToFavourite(favourite);
}

auto GoomFavouriteStatesHandler::ChangeToFavourite(const Favourites favourite) -> void
{
  m_currentFavourite = favourite;

  const auto& drawables      = FAVOURITE_DRAWABLES[favourite];
  const auto buffIntensities = GetBuffIntensities(drawables);
  m_currentDrawablesState    = GoomDrawablesState{drawables, buffIntensities};
}

auto GoomFavouriteStatesHandler::GetBuffIntensities(
    const std::vector<GoomDrawables>& drawables) const noexcept -> std::vector<float>
{
  return std::ranges::to<std::vector<float>>(
      drawables | std::views::transform(
                      [this](const auto drawable)
                      { return m_goomRand->GetRandInRange(BUFF_INTENSITY_RANGES[drawable]); }));
}

} // namespace GOOM::CONTROL
