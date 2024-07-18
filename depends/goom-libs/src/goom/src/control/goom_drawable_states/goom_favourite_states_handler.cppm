module;

#include <vector>

export module Goom.Control.GoomFavouriteStatesHandler;

import Goom.Control.GoomDrawables;
import Goom.Control.GoomStateHandler;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomTypes;

using GOOM::UTILS::EnumMap;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::Weights;

export namespace GOOM::CONTROL
{

class GoomFavouriteStatesHandler : public IGoomStateHandler
{
public:
  explicit GoomFavouriteStatesHandler(const GoomRand& goomRand);

  auto ChangeToNextState() -> void override;

  [[nodiscard]] auto GetCurrentState() const noexcept -> const GoomDrawablesState& override;

private:
  const GoomRand* m_goomRand;

  enum class Favourites : UnderlyingEnumType
  {
    DOTS_ONLY,
    DOTS_PARTICLES,
    DOTS_IFS,
    PARTICLES_ONLY,
    PARTICLES_IFS,
  };
  Favourites m_currentFavourite{};
  Weights<Favourites> m_weightedFavourites;
  static const EnumMap<Favourites, std::vector<GoomDrawables>> FAVOURITE_DRAWABLES;
  auto ChangeToFavourite(Favourites favourite) -> void;

  GoomDrawablesState m_currentDrawablesState;
  [[nodiscard]] auto GetGoomDrawablesState(Favourites favourite) const noexcept
      -> GoomDrawablesState;
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto GoomFavouriteStatesHandler::GetCurrentState() const noexcept
    -> const GoomDrawablesState&
{
  return m_currentDrawablesState;
}

} // namespace GOOM::CONTROL
