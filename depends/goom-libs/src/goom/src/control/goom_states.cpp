#include "goom_states.h"

#include "utils/enumutils.h"

namespace GOOM::CONTROL
{

using UTILS::NUM;
using UTILS::ToUType;

struct RawStateInfo
{
  GoomStates state;
  const char* name;
  std::vector<GoomStateInfo::DrawableInfo> drawableInfo;
};


// TODO - When we get to use C++20 we can simplify things with
//        constexpr std::vector.
// clang-format off
//  Drawable                                              BuffIntensity
static const std::vector DOTS_ONLY_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::DOTS,      {0.5F}},
};
static const std::vector DOTS_IFS_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::DOTS,      {0.1F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::IFS,       {0.3F}},
};
static const std::vector DOTS_IFS_STARS_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::DOTS,      {0.2F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::IFS,       {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::STARS,     {0.2F}},
};
static const std::vector DOTS_IMAGE_STARS_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::DOTS,      {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::IMAGE,     {0.1F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::STARS,     {0.2F}},
};
static const std::vector DOTS_LINES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::DOTS,      {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::LINES,     {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::SCOPE,     {0.0F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::FAR_SCOPE, {0.0F}},
};
static const std::vector DOTS_LINES_STAR_TENTACLES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::DOTS,      {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::LINES,     {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::STARS,     {0.2F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TENTACLES, {0.2F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::SCOPE,     {0.0F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::FAR_SCOPE, {0.0F}},
};
static const std::vector DOTS_LINES_TENTACLES_TUBES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::DOTS,      {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::LINES,     {0.1F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TENTACLES, {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TUBE,      {0.4F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::SCOPE,     {0.0F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::FAR_SCOPE, {0.0F}},
};
static const std::vector DOTS_LINES_TUBES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::DOTS,      {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::LINES,     {0.2F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TUBE,      {0.2F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::SCOPE,     {0.0F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::FAR_SCOPE, {0.0F}},
};
static const std::vector DOTS_STARS_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::DOTS,      {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::STARS,     {0.2F}},
};
static const std::vector DOTS_STARS_TENTACLES_TUBES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::DOTS,      {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::STARS,     {0.2F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TENTACLES, {0.2F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TUBE,      {0.2F}},
};
static const std::vector DOTS_TENTACLES_TUBES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::DOTS,      {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TENTACLES, {0.2F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TUBE,      {0.2F}},
};
static const std::vector IFS_ONLY_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IFS,       {0.5F}},
};
static const std::vector IFS_IMAGE_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IFS,       {0.4F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::IMAGE,     {0.1F}},
};
static const std::vector IFS_LINES_STARS_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IFS,       {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::STARS,     {0.1F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::LINES,     {0.2F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::SCOPE,     {0.0F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::FAR_SCOPE, {0.0F}},
};
static const std::vector IFS_STARS_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IFS,       {0.4F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::STARS,     {0.1F}},
};
static const std::vector IFS_STARS_TENTACLES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IFS,       {0.4F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::STARS,     {0.1F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TENTACLES, {0.1F}},
};
static const std::vector IFS_TENTACLES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IFS,       {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TENTACLES, {0.1F}},
};
static const std::vector IFS_TENTACLES_TUBES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IFS,       {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TENTACLES, {0.1F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TUBE,      {0.2F}},
};
static const std::vector IFS_TUBES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IFS,       {0.5F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TUBE,      {0.3F}},
};
static const std::vector IMAGE_ONLY_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IMAGE,     {0.2F}},
};
static const std::vector IMAGE_LINES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IMAGE,     {0.1F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::LINES,     {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::SCOPE,     {0.0F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::FAR_SCOPE, {0.0F}},
};
static const std::vector IMAGE_LINES_STARS_TENTACLES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IMAGE,     {0.1F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::LINES,     {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::STARS,     {0.2F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TENTACLES, {0.2F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::FAR_SCOPE, {0.0F}},
};
static const std::vector IMAGE_STARS_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IMAGE,     {0.1F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::STARS,     {0.3F}},
};
static const std::vector IMAGE_TENTACLES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IMAGE,     {0.1F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TENTACLES, {0.3F}},
};
static const std::vector IMAGE_TUBES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::IMAGE,     {0.1F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TUBE,      {0.3F}},
};
static const std::vector LINES_ONLY_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::LINES,     {0.3F}},
};
static const std::vector LINES_STARS_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::LINES,     {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::STARS,     {0.2F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::SCOPE,     {0.0F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::FAR_SCOPE, {0.0F}},
};
static const std::vector LINES_TENTACLES_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::LINES,     {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::TENTACLES, {0.3F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::SCOPE,     {0.0F}},
    GoomStateInfo::DrawableInfo{GoomDrawables::FAR_SCOPE, {0.0F}},
};
static const std::vector STARS_ONLY_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::STARS,     {0.3F}},
};
static const std::vector TENTACLES_ONLY_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::TENTACLES, {0.3F}},
};
static const std::vector TUBES_ONLY_DRAWABLES_INFO {
    GoomStateInfo::DrawableInfo{GoomDrawables::TUBE,      {0.3F}},
};

static const std::array RAW_STATE_INFO_ARRAY{
    RawStateInfo{GoomStates::DOTS_ONLY,                   "Dots Only",              DOTS_ONLY_DRAWABLES_INFO},
    RawStateInfo{GoomStates::DOTS_IFS,                    "Dots and Ifs",           DOTS_IFS_DRAWABLES_INFO},
    RawStateInfo{GoomStates::DOTS_IFS_STARS,              "Dots, IFS, Stars",       DOTS_IFS_STARS_DRAWABLES_INFO},
    RawStateInfo{GoomStates::DOTS_IMAGE_STARS,            "Dots, Image, Stars",     DOTS_IMAGE_STARS_DRAWABLES_INFO},
    RawStateInfo{GoomStates::DOTS_LINES,                  "Dots and Lines",         DOTS_LINES_DRAWABLES_INFO},
    RawStateInfo{GoomStates::DOTS_LINES_STAR_TENTACLES,   "D, L, S, Te",            DOTS_LINES_STAR_TENTACLES_DRAWABLES_INFO},
    RawStateInfo{GoomStates::DOTS_LINES_TENTACLES_TUBES,  "D, L, Te, Tu",           DOTS_LINES_TENTACLES_TUBES_DRAWABLES_INFO},
    RawStateInfo{GoomStates::DOTS_LINES_TUBES,            "D, L, Tu",               DOTS_LINES_TUBES_DRAWABLES_INFO},
    RawStateInfo{GoomStates::DOTS_STARS,                  "Dots and Stars",         DOTS_STARS_DRAWABLES_INFO},
    RawStateInfo{GoomStates::DOTS_STARS_TENTACLES_TUBES,  "D, S, Te, Tu",           DOTS_STARS_TENTACLES_TUBES_DRAWABLES_INFO},
    RawStateInfo{GoomStates::DOTS_TENTACLES_TUBES,        "Dots, Tentacles, Tubes", DOTS_TENTACLES_TUBES_DRAWABLES_INFO},

    RawStateInfo{GoomStates::IFS_ONLY,                    "IFS Only",               IFS_ONLY_DRAWABLES_INFO},
    RawStateInfo{GoomStates::IFS_IMAGE,                   "IFS and Image",          IFS_IMAGE_DRAWABLES_INFO},
    RawStateInfo{GoomStates::IFS_LINES_STARS,             "IFS, Lines, Stars",      IFS_LINES_STARS_DRAWABLES_INFO},
    RawStateInfo{GoomStates::IFS_STARS,                   "IFS and Stars",          IFS_STARS_DRAWABLES_INFO},
    RawStateInfo{GoomStates::IFS_STARS_TENTACLES,         "IFS, Stars, Tentacles",  IFS_STARS_TENTACLES_DRAWABLES_INFO},
    RawStateInfo{GoomStates::IFS_TENTACLES,               "IFS and Tentacles",      IFS_TENTACLES_DRAWABLES_INFO},
    RawStateInfo{GoomStates::IFS_TENTACLES_TUBES,         "IFS, Tentacles, Tubes",  IFS_TENTACLES_TUBES_DRAWABLES_INFO},
    RawStateInfo{GoomStates::IFS_TUBES,                   "Ifs and Tubes",          IFS_TUBES_DRAWABLES_INFO},

    RawStateInfo{GoomStates::IMAGE_ONLY,                  "Image Only",             IMAGE_ONLY_DRAWABLES_INFO},
    RawStateInfo{GoomStates::IMAGE_LINES,                 "Image and Lines",        IMAGE_LINES_DRAWABLES_INFO},
    RawStateInfo{GoomStates::IMAGE_LINES_STARS_TENTACLES, "Im, L, S, Te",           IMAGE_LINES_STARS_TENTACLES_DRAWABLES_INFO},
    RawStateInfo{GoomStates::IMAGE_STARS,                 "Image and Stars",        IMAGE_STARS_DRAWABLES_INFO},
    RawStateInfo{GoomStates::IMAGE_TENTACLES,             "Image and Tentacles",    IMAGE_TENTACLES_DRAWABLES_INFO},
    RawStateInfo{GoomStates::IMAGE_TUBES,                 "Image and Tubes",        IMAGE_TUBES_DRAWABLES_INFO},

    RawStateInfo{GoomStates::LINES_ONLY,                  "Lines Only",             LINES_ONLY_DRAWABLES_INFO},
    RawStateInfo{GoomStates::LINES_STARS,                 "Lines and Stars",        LINES_STARS_DRAWABLES_INFO},
    RawStateInfo{GoomStates::LINES_TENTACLES,             "Tentacles and Lines",    LINES_TENTACLES_DRAWABLES_INFO},

    RawStateInfo{GoomStates::STARS_ONLY,                  "Stars Only",             STARS_ONLY_DRAWABLES_INFO},
    RawStateInfo{GoomStates::TENTACLES_ONLY,              "Tentacles Only",         TENTACLES_ONLY_DRAWABLES_INFO},
    RawStateInfo{GoomStates::TUBES_ONLY,                  "Tubes Only",             TUBES_ONLY_DRAWABLES_INFO},
};
// clang-format on

static_assert(RAW_STATE_INFO_ARRAY.size() == NUM<GoomStates>);

const GoomStateInfo::StateInfoArray GoomStateInfo::STATE_INFO_ARRAY = GetStateInfoArray();

auto GoomStateInfo::GetStateInfoArray() noexcept -> StateInfoArray
{
  StateInfoArray statesArray{};

  for (const auto& stateInfo : RAW_STATE_INFO_ARRAY)
  {
    statesArray[ToUType(stateInfo.state)] = {std::string(stateInfo.name), stateInfo.drawableInfo};
  }

  return statesArray;
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

} // namespace GOOM::CONTROL
