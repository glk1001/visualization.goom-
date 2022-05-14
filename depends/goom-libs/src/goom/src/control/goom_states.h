#pragma once

#include "goom_graphic.h"
#include "utils/enumutils.h"
#include "utils/math/goom_rand_base.h"

#include <array>
#include <string_view>
#include <vector>

namespace GOOM::CONTROL
{

enum class GoomStates : size_t
{
  CIRCLES_ONLY = 0,
  CIRCLES_IFS,
  CIRCLES_IMAGE,
  CIRCLES_IMAGE_STARS,
  CIRCLES_LINES,
  CIRCLES_LINES_SHAPES,
  CIRCLES_SHAPES,
  CIRCLES_STARS_TUBES,
  CIRCLES_TENTACLES,
  DOTS_IFS,
  DOTS_IFS_STARS,
  DOTS_IMAGE_STARS,
  DOTS_LINES,
  DOTS_LINES_STARS_TENTACLES,
  DOTS_LINES_TENTACLES_TUBES,
  DOTS_LINES_TUBES,
  DOTS_ONLY,
  DOTS_STARS,
  DOTS_STARS_TENTACLES_TUBES,
  DOTS_TENTACLES_TUBES,
  IFS_IMAGE,
  IFS_IMAGE_SHAPES,
  IFS_LINES_STARS,
  IFS_ONLY,
  IFS_SHAPES,
  IFS_STARS,
  IFS_STARS_TENTACLES,
  IFS_TENTACLES,
  IFS_TENTACLES_TUBES,
  IFS_TUBES,
  IMAGE_LINES,
  IMAGE_LINES_SHAPES,
  IMAGE_LINES_STARS_TENTACLES,
  IMAGE_ONLY,
  IMAGE_SHAPES_STARS,
  IMAGE_SHAPES_TUBES,
  IMAGE_STARS,
  IMAGE_TENTACLES,
  IMAGE_TUBES,
  LINES_ONLY,
  LINES_SHAPES_STARS,
  LINES_STARS,
  LINES_TENTACLES,
  SHAPES_ONLY,
  STARS_ONLY,
  TENTACLES_ONLY,
  TUBES_ONLY,
  _num // unused and must be last
};

enum class GoomDrawables
{
  CIRCLES = 0,
  DOTS,
  FAR_SCOPE,
  IFS,
  LINES,
  IMAGE,
  SCOPE,
  SHADER,
  SHAPES,
  STARS,
  TENTACLES,
  TUBES,
  _num // unused and must be last
};

using BuffIntensityRange = UTILS::MATH::IGoomRand::NumberRange<float>;

class GoomStateInfo
{
public:
  struct DrawableInfo
  {
    GoomDrawables fx{};
    BuffIntensityRange buffIntensityRange{};
  };
  struct StateInfo
  {
    std::string_view name;
    std::vector<DrawableInfo> drawablesInfo;
  };

  GoomStateInfo() noexcept = delete;

  [[nodiscard]] static auto GetStateInfo(GoomStates goomState) -> const StateInfo&;
  [[nodiscard]] static auto GetBuffIntensityRange(GoomStates goomState, GoomDrawables fx)
      -> BuffIntensityRange;
  [[nodiscard]] static auto IsMultiThreaded(GoomStates goomState) -> bool;

private:
  using StateInfoArray = std::array<StateInfo, UTILS::NUM<GoomStates>>;
  static const StateInfoArray STATE_INFO_ARRAY;
  [[nodiscard]] static auto GetStateInfoArray() noexcept -> StateInfoArray;
  [[nodiscard]] static auto GetDrawablesInfo(GoomStates goomState) -> std::vector<DrawableInfo>;
};

inline auto GoomStateInfo::GetStateInfo(const GoomStates goomState) -> const StateInfo&
{
  return STATE_INFO_ARRAY.at(UTILS::ToUType(goomState));
}

} // namespace GOOM::CONTROL
