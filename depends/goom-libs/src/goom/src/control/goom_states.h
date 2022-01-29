#pragma once

#include "goom_graphic.h"
#include "utils/enumutils.h"

#include <array>
#include <string>
#include <vector>

namespace GOOM::CONTROL
{

enum class GoomStates : size_t
{
  DOTS_IFS = 0,
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
  IFS_LINES_STARS,
  IFS_ONLY,
  IFS_STARS,
  IFS_STARS_TENTACLES,
  IFS_TENTACLES,
  IFS_TENTACLES_TUBES,
  IFS_TUBES,
  IMAGE_LINES,
  IMAGE_LINES_STARS_TENTACLES,
  IMAGE_ONLY,
  IMAGE_STARS,
  IMAGE_TENTACLES,
  IMAGE_TUBES,
  LINES_ONLY,
  LINES_STARS,
  LINES_TENTACLES,
  STARS_ONLY,
  TENTACLES_ONLY,
  TUBES_ONLY,
  _NUM
};

enum class GoomDrawables
{
  IFS = 0,
  DOTS,
  TENTACLES,
  STARS,
  LINES,
  SCOPE,
  FAR_SCOPE,
  IMAGE,
  SHADER,
  TUBES,
  _NUM // unused and must be last
};

class GoomStateInfo
{
public:
  struct DrawableInfo
  {
    GoomDrawables fx{};
    FXBuffSettings buffSettings{};
  };
  struct StateInfo
  {
    std::string name;
    std::vector<DrawableInfo> drawablesInfo;
  };

  GoomStateInfo() noexcept = delete;

  [[nodiscard]] static auto GetStateInfo(GoomStates goomState) -> const StateInfo&;
  [[nodiscard]] static auto GetBuffSettings(GoomStates goomState, GoomDrawables fx)
      -> FXBuffSettings;

private:
  using StateInfoArray = std::array<StateInfo, UTILS::NUM<GoomStates>>;
  static const StateInfoArray STATE_INFO_ARRAY;
  [[nodiscard]] static auto GetStateInfoArray() noexcept -> StateInfoArray;
};

inline auto GoomStateInfo::GetStateInfo(const GoomStates goomState) -> const StateInfo&
{
  return STATE_INFO_ARRAY.at(UTILS::ToUType(goomState));
}

} // namespace GOOM::CONTROL
