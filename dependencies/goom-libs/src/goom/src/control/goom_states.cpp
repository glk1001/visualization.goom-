#include "goom_states.h"

#include "utils/enumutils.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

using UTILS::ToUType;

const GoomStateInfo::StateInfoArray GoomStateInfo::STATE_INFO_ARRAY = GetStateInfoArray();

void GoomStateInfo::NormaliseStates(StateInfoArray& stateInfoArray)
{
  for (auto& state : stateInfoArray)
  {
    float totalBuffIntensity = 0.0;
    for (const auto& drawableInfo : state.drawablesInfo)
    {
      totalBuffIntensity += drawableInfo.buffSettings.buffIntensity;
    }
    if (totalBuffIntensity <= 1.0F)
    {
      continue;
    }

    for (auto& drawableInfo : state.drawablesInfo)
    {
      drawableInfo.buffSettings.buffIntensity /= totalBuffIntensity;
    }
  }
}

auto GoomStateInfo::GetBuffSettings(const GoomStates goomState, const GoomDrawables fx)
    -> FXBuffSettings
{
  for (const auto& drawableInfo : GetStateInfo(goomState).drawablesInfo)
  {
    if (drawableInfo.fx == fx)
    {
      return drawableInfo.buffSettings;
    }
  }
  return FXBuffSettings{};
}

auto GoomStateInfo::GetStateInfoArray() -> StateInfoArray
{
  StateInfoArray statesArray{};

  statesArray[ToUType(GoomStates::DOTS_ONLY)] = {
      /*.name = */ "Dots Only",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::DOTS,
           /*.buffSettings = */ {/*.buffIntensity = */ 1.0F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::DOTS_IFS)] = {
      /*.name = */ "Dots and Ifs",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::DOTS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.1F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::IFS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::DOTS_IFS_STARS)] = {
      /*.name = */ "Dots, IFS, Stars",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::DOTS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.6F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::IFS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::STARS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.3F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::DOTS_IMAGE_STARS)] = {
      /*.name = */ "Dots, Image, Stars",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::DOTS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::IMAGE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.3F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::STARS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.3F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::DOTS_LINES)] = {
      /*.name = */ "Dots and Lines",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::DOTS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::LINES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::FAR_SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::DOTS_LINES_STAR_TENTACLES)] = {
      /*.name = */ "D, L, S, Te",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::DOTS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::LINES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.2F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::STARS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.3F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TENTACLES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.2F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::FAR_SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.2F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::DOTS_LINES_TENTACLES_TUBES)] = {
      /*.name = */ "D, L, Te, Tu",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::DOTS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::LINES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.2F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TENTACLES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TUBE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.4F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::FAR_SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::DOTS_LINES_TUBES)] = {
      /*.name = */ "D, L, Tu",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::DOTS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::LINES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TUBE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.3F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::FAR_SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::DOTS_STARS)] = {
      /*.name = */ "Dots and Stars",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::DOTS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::STARS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.3F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::DOTS_STARS_TENTACLES_TUBES)] = {
      /*.name = */ "D, S, Te, Tu",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::DOTS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::STARS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.4F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TENTACLES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TUBE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.3F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::DOTS_TENTACLES_TUBES)] = {
      /*.name = */ "Dots, Tentacles, Tubes",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::DOTS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.6F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TENTACLES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TUBE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.3F, /*.allowOverexposed = */ true}},
      },
  };

  statesArray[ToUType(GoomStates::IFS_ONLY)] = {
      /*.name = */ "IFS Only",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::IFS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::IFS_IMAGE)] = {
      /*.name = */ "Ifs and Image",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::IFS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::IMAGE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::IFS_LINES_STARS)] = {
      /*.name = */ "IFS, Lines, Stars",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::IFS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::STARS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.4F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::LINES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.2F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::FAR_SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.2F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::IFS_STARS)] = {
      /*.name = */ "Ifs and Stars",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::IFS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::STARS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.3F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::IFS_STARS_TENTACLES)] = {
      /*.name = */ "IFS, Stars, Tentacles",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::IFS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TENTACLES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.2F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::STARS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.2F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::IFS_TENTACLES)] = {
      /*.name = */ "IFS and Tentacles",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::IFS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.2F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TENTACLES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::IFS_TENTACLES_TUBES)] = {
      /*.name = */ "IFS, Tentacles, Tubes",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::IFS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TENTACLES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.2F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TUBE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.4F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::IFS_TUBES)] = {
      /*.name = */ "Ifs and Tubes",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::IFS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TUBE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
      },
  };

  statesArray[ToUType(GoomStates::IMAGE_ONLY)] = {
      /*.name = */ "Image Only",
      /*.drawablesInfo */ {
          {/*.fx = */ GoomDrawables::IMAGE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.8F,
                                 /*.allowOverexposed = */ true}},
      }};
  statesArray[ToUType(GoomStates::IMAGE_LINES)] = {
      /*.name = */ "Image and Lines",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::IMAGE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.4F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::LINES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.4F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::FAR_SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::IMAGE_LINES_STARS_TENTACLES)] = {
      /*.name = */ "Im, L, S, Te",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::IMAGE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.3F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::LINES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::STARS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.4F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TENTACLES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::FAR_SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::IMAGE_STARS)] = {
      /*.name = */ "Image and Stars",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::IMAGE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::STARS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.3F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::IMAGE_TENTACLES)] = {
      /*.name = */ "Image and Tentacles",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::IMAGE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.2F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TENTACLES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::IMAGE_TUBES)] = {
      /*.name = */ "Image and Tubes",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::IMAGE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TUBE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
      },
  };

  statesArray[ToUType(GoomStates::LINES_ONLY)] = {
      /*.name = */ "Lines Only",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::LINES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.8F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::LINES_STARS)] = {
      /*.name = */ "Lines and Stars",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::LINES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::STARS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.4F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::FAR_SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
      },
  };
  statesArray[ToUType(GoomStates::LINES_TENTACLES)] = {
      /*.name = */ "Tentacles and Lines",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::LINES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.2F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::TENTACLES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
          {/*.fx = */ GoomDrawables::FAR_SCOPE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.5F, /*.allowOverexposed = */ true}},
      },
  };

  statesArray[ToUType(GoomStates::STARS_ONLY)] = {
      /*.name = */ "Stars Only",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::STARS,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
      },
  };

  statesArray[ToUType(GoomStates::TENTACLES_ONLY)] = {
      /*.name = */ "Tentacles Only",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::TENTACLES,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.8F, /*.allowOverexposed = */ true}},
      },
  };

  statesArray[ToUType(GoomStates::TUBES_ONLY)] = {
      /*.name = */ "Tubes Only",
      /*.drawablesInfo */
      {
          {/*.fx = */ GoomDrawables::TUBE,
           /*.buffSettings = */ {/*.buffIntensity = */ 0.7F, /*.allowOverexposed = */ true}},
      },
  };

  NormaliseStates(statesArray);

  return statesArray;
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
