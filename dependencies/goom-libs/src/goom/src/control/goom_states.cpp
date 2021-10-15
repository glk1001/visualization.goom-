#include "goom_states.h"

#include "goom_graphic.h"

#include <utility>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

//@formatter:off
// clang-format off
GoomStates::WeightedStatesArray GoomStates::STATES{{
  {
    /*.name = */ "Image Tester",
    /*.weight = */1,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IMAGE,     /*.buffSettings = */{ /*.buffIntensity = */0.8F, /*.allowOverexposed = */true  } },
    }},
  },
  {
    /*.name = */ "Lines Tester",
    /*.weight = */1,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::LINES,     /*.buffSettings = */{ /*.buffIntensity = */0.8F, /*.allowOverexposed = */true  } },
    }},
  },
  {
    /*.name = */ "Tentacles Tester",
    /*.weight = */1,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::TENTACLES, /*.buffSettings = */{ /*.buffIntensity = */0.8F, /*.allowOverexposed = */true  } },
    }},
  },
  {
    /*.name = */ "IFS Tester",
    /*.weight = */1,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IFS,       /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */true  } },
    }},
  },
  {
    /*.name = */ "Image Tester",
    /*.weight = */1,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IMAGE,       /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */true  } },
    }},
  },
  {
   /*.name = */ "Tube Tester",
   /*.weight = */1,
   /*.drawables */{{
      { /*.fx = */GoomDrawable::TUBE,    /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */false  } },
    }},
  },
  {
    /*.name = */ "Dots Tester",
    /*.weight = */1,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::DOTS,      /*.buffSettings = */{ /*.buffIntensity = */1.0F, /*.allowOverexposed = */false  } },
    }},
  },
  {
    /*.name = */ "Stars Tester",
    /*.weight = */1,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::STARS,     /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */false } },
  }},
  },
  {
    /*.name = */ "Ifs and Dots",
    /*.weight = */200,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IFS,       /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::DOTS,      /*.buffSettings = */{ /*.buffIntensity = */0.1F, /*.allowOverexposed = */false } },
    }},
   },
   {
     /*.name = */ "Ifs and Image",
     /*.weight = */200,
     /*.drawables */{{
       { /*.fx = */GoomDrawable::IFS,       /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */true  } },
       { /*.fx = */GoomDrawable::IMAGE,     /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true } },
    }},
   },
   {
    /*.name = */ "Ifs and Stars",
    /*.weight = */200,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IFS,       /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::STARS,     /*.buffSettings = */{ /*.buffIntensity = */0.3F, /*.allowOverexposed = */false } },
    }},
   },
   {
    /*.name = */ "Image and Stars",
    /*.weight = */200,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IMAGE,     /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::STARS,     /*.buffSettings = */{ /*.buffIntensity = */0.3F, /*.allowOverexposed = */false } },
    }},
   },
   {
    /*.name = */ "Ifs and Tubes",
    /*.weight = */200,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IFS,       /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::TUBE,      /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */false } },
    }},
   },
   {
    /*.name = */ "Image and Tubes",
    /*.weight = */200,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IMAGE,     /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::TUBE,      /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */false } },
    }},
   },
   {
    /*.name = */ "Tentacles, Dots, Tubes",
    /*.weight = */200,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::TENTACLES, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::DOTS,      /*.buffSettings = */{ /*.buffIntensity = */0.6F, /*.allowOverexposed = */false } },
      { /*.fx = */GoomDrawable::TUBE,      /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true } },
    }},
   },
   {
    /*.name = */ "Tentacles and Lines",
    /*.weight = */200,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::TENTACLES, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::LINES,     /*.buffSettings = */{ /*.buffIntensity = */0.2F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::SCOPE,     /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::FAR_SCOPE, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
    }},
   },
   {
    /*.name = */ "Image and Lines",
    /*.weight = */200,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IMAGE,     /*.buffSettings = */{ /*.buffIntensity = */0.4F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::LINES,     /*.buffSettings = */{ /*.buffIntensity = */0.4F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::SCOPE,     /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::FAR_SCOPE, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
    }},
   },
   {
    /*.name = */ "IFS, Dots, Stars",
    /*.weight = */100,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IFS,       /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::DOTS,      /*.buffSettings = */{ /*.buffIntensity = */0.6F, /*.allowOverexposed = */false } },
      { /*.fx = */GoomDrawable::STARS,     /*.buffSettings = */{ /*.buffIntensity = */0.3F, /*.allowOverexposed = */false } },
    }},
   },
   {
    /*.name = */ "IFS, Tentacles, Stars",
    /*.weight = */20,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IFS,       /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::TENTACLES, /*.buffSettings = */{ /*.buffIntensity = */0.2F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::STARS,     /*.buffSettings = */{ /*.buffIntensity = */0.2F, /*.allowOverexposed = */false } },
    }},
  },
  {
    /*.name = */ "IFS, Lines, Stars",
    /*.weight = */60,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IFS,       /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::STARS,     /*.buffSettings = */{ /*.buffIntensity = */0.4F, /*.allowOverexposed = */false } },
      { /*.fx = */GoomDrawable::LINES,     /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::SCOPE,     /*.buffSettings = */{ /*.buffIntensity = */0.2F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::FAR_SCOPE, /*.buffSettings = */{ /*.buffIntensity = */0.2F, /*.allowOverexposed = */true  } },
    }},
  },
  {
    /*.name = */ "IFS, Tentacles, Tubes",
    /*.weight = */70,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IFS,       /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::TENTACLES, /*.buffSettings = */{ /*.buffIntensity = */0.2F, /*.allowOverexposed = */false } },
      { /*.fx = */GoomDrawable::TUBE,      /*.buffSettings = */{ /*.buffIntensity = */0.4F, /*.allowOverexposed = */false } },
    }},
  },
  {
    /*.name = */ "IFS and Tentacles",
    /*.weight = */70,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IFS,       /*.buffSettings = */{ /*.buffIntensity = */0.2F, /*.allowOverexposed = */true } },
      { /*.fx = */GoomDrawable::TENTACLES, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true } },
    }},
  },
  {
    /*.name = */ "Image and Tentacles",
    /*.weight = */70,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::IMAGE,     /*.buffSettings = */{ /*.buffIntensity = */0.2F, /*.allowOverexposed = */true } },
      { /*.fx = */GoomDrawable::TENTACLES, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true } },
    }},
  },
  {
    /*.name = */ "D, Te, S, L",
    /*.weight = */40,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::DOTS,      /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::TENTACLES, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::STARS,     /*.buffSettings = */{ /*.buffIntensity = */0.3F, /*.allowOverexposed = */false } },
      { /*.fx = */GoomDrawable::LINES,     /*.buffSettings = */{ /*.buffIntensity = */0.2F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::SCOPE,     /*.buffSettings = */{ /*.buffIntensity = */0.2F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::FAR_SCOPE, /*.buffSettings = */{ /*.buffIntensity = */0.2F, /*.allowOverexposed = */true  } },
    }},
  },
  {
    /*.name = */ "D, Te, L, Tu",
    /*.weight = */40,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::DOTS,      /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::TENTACLES, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::LINES,     /*.buffSettings = */{ /*.buffIntensity = */0.2F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::SCOPE,     /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::FAR_SCOPE, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::TUBE,      /*.buffSettings = */{ /*.buffIntensity = */0.4F, /*.allowOverexposed = */false } },
    }},
  },
  {
    /*.name = */ "D, Te, S, Tu",
    /*.weight = */100,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::DOTS,      /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::TENTACLES, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::STARS,     /*.buffSettings = */{ /*.buffIntensity = */0.4F, /*.allowOverexposed = */false } },
      { /*.fx = */GoomDrawable::TUBE,      /*.buffSettings = */{ /*.buffIntensity = */0.3F, /*.allowOverexposed = */false } },
    }},
  },
  {
    /*.name = */ "D, Te, Tu",
    /*.weight = */70,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::DOTS,      /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::TENTACLES, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::TUBE,      /*.buffSettings = */{ /*.buffIntensity = */0.3F, /*.allowOverexposed = */false } },
    }},
  },
  {
    /*.name = */ "Te, S, L, Im",
    /*.weight = */100,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::TENTACLES, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::STARS,     /*.buffSettings = */{ /*.buffIntensity = */0.4F, /*.allowOverexposed = */false } },
      { /*.fx = */GoomDrawable::LINES,     /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::FAR_SCOPE, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::IMAGE,     /*.buffSettings = */{ /*.buffIntensity = */0.3F, /*.allowOverexposed = */true  } },
    }},
  },
  {
    /*.name = */ "Stars and Lines",
    /*.weight = */60,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::STARS,     /*.buffSettings = */{ /*.buffIntensity = */0.4F, /*.allowOverexposed = */false } },
      { /*.fx = */GoomDrawable::LINES,     /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::SCOPE,     /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::FAR_SCOPE, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
    }},
  },
  {
    /*.name = */ "Dots and Stars",
    /*.weight = */60,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::DOTS,      /*.buffSettings = */{ /*.buffIntensity = */0.7F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::STARS,     /*.buffSettings = */{ /*.buffIntensity = */0.3F, /*.allowOverexposed = */false } },
      { /*.fx = */GoomDrawable::IMAGE,     /*.buffSettings = */{ /*.buffIntensity = */0.3F, /*.allowOverexposed = */true  } },
    }},
  },
  {
    /*.name = */ "Dots and Lines",
    /*.weight = */60,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::DOTS,      /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::LINES,     /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::SCOPE,     /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::FAR_SCOPE, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
    }},
  },
  {
    /*.name = */ "D, L, Tu",
    /*.weight = */60,
    /*.drawables */{{
      { /*.fx = */GoomDrawable::DOTS,      /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::LINES,     /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::SCOPE,     /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::FAR_SCOPE, /*.buffSettings = */{ /*.buffIntensity = */0.5F, /*.allowOverexposed = */true  } },
      { /*.fx = */GoomDrawable::TUBE,      /*.buffSettings = */{ /*.buffIntensity = */0.3F, /*.allowOverexposed = */true  } },
    }},
 },
}};
// clang-format on
//@formatter:on

GoomStates::GoomStates() : m_weightedStates{GetWeightedStates(STATES)}
{
  NormaliseStates();
  DoRandomStateChange();
}

auto GoomStates::GetCurrentDrawables() const -> GoomStates::DrawablesState
{
  GoomStates::DrawablesState currentDrawables{};
  for (const auto drawable : STATES[m_currentStateIndex].drawables)
  {
    currentDrawables.insert(drawable.fx);
  }
  return currentDrawables;
}

auto GoomStates::GetCurrentBuffSettings(const GoomDrawable theFx) const -> FXBuffSettings
{
  for (const auto& drawable : STATES[m_currentStateIndex].drawables)
  {
    if (drawable.fx == theFx)
    {
      return drawable.buffSettings;
    }
  }
  return FXBuffSettings{};
}

void GoomStates::DoRandomStateChange()
{
  m_currentStateIndex = static_cast<size_t>(m_weightedStates.GetRandomWeighted());
}

void GoomStates::NormaliseStates()
{
  for (auto& state : STATES)
  {
    float totalBuffIntensity = 0.0;
    for (const auto& drawableInfo : state.drawables)
    {
      totalBuffIntensity += drawableInfo.buffSettings.buffIntensity;
    }
    if (totalBuffIntensity <= 1.0F)
    {
      continue;
    }

    for (auto& drawableInfo : state.drawables)
    {
      drawableInfo.buffSettings.buffIntensity /= totalBuffIntensity;
    }
  }
}

auto GoomStates::GetWeightedStates(const GoomStates::WeightedStatesArray& theStates)
    -> std::vector<std::pair<uint16_t, size_t>>
{
  std::vector<std::pair<uint16_t, size_t>> weightedVals(theStates.size());
  for (size_t i = 0; i < theStates.size(); i++)
  {
    weightedVals[i] = std::make_pair(i, theStates[i].weight);
  }
  return weightedVals;
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
