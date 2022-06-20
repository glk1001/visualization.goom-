#include "visual_fx_color_maps.h"

#include "color/random_colormaps.h"
#include "utils/enumutils.h"
#include "utils/math/goom_rand_base.h"

#include <array>
#include <numeric>

namespace GOOM::CONTROL
{

using COLOR::GetAllMapsUnweighted;
using COLOR::GetAllSlimMaps;
using COLOR::GetAllStandardMaps;
using COLOR::GetBlueStandardMaps;
using COLOR::GetCitiesStandardMaps;
using COLOR::GetColdStandardMaps;
using COLOR::GetDivergingBlackStandardMaps;
using COLOR::GetGreenStandardMaps;
using COLOR::GetHeatStandardMaps;
using COLOR::GetMostlySequentialSlimMaps;
using COLOR::GetMostlySequentialStandardMaps;
using COLOR::GetOrangeStandardMaps;
using COLOR::GetPastelStandardMaps;
using COLOR::GetPurpleStandardMaps;
using COLOR::GetRedStandardMaps;
using COLOR::GetSeasonsStandardMaps;
using COLOR::GetSlightlyDivergingSlimMaps;
using COLOR::GetSlightlyDivergingStandardMaps;
using COLOR::GetWesAndersonMaps;
using COLOR::GetYellowStandardMaps;
using UTILS::NUM;
using UTILS::MATH::IGoomRand;

auto VisualFxColorMaps::MakeColorMatchedSetsMap() const -> ColorMatchedSetsMap
{
  // clang-format off
  return { 
    {ColorMatchedSets::RED_GREEN_STD_MAPS,      GetTwoGroupsColorMatchedSet(GetRedStandardMaps,    GetGreenStandardMaps)},
    {ColorMatchedSets::RED_BLUE_STD_MAPS,       GetTwoGroupsColorMatchedSet(GetRedStandardMaps,    GetBlueStandardMaps)},
    {ColorMatchedSets::YELLOW_BLUE_STD_MAPS,    GetTwoGroupsColorMatchedSet(GetYellowStandardMaps, GetBlueStandardMaps)},
    {ColorMatchedSets::YELLOW_PURPLE_STD_MAPS,  GetTwoGroupsColorMatchedSet(GetYellowStandardMaps, GetPurpleStandardMaps)},
    {ColorMatchedSets::ORANGE_GREEN_STD_MAPS,   GetTwoGroupsColorMatchedSet(GetOrangeStandardMaps, GetGreenStandardMaps)},
    {ColorMatchedSets::ORANGE_PURPLE_STD_MAPS,  GetTwoGroupsColorMatchedSet(GetOrangeStandardMaps, GetPurpleStandardMaps)},
    {ColorMatchedSets::ALL_ONLY_STD_MAPS,       GetOneGroupColorMatchedSet(GetAllStandardMaps)},
    {ColorMatchedSets::HEAT_ONLY_STD_MAPS,      GetOneGroupColorMatchedSet(GetHeatStandardMaps)},
    {ColorMatchedSets::COLD_ONLY_STD_MAPS,      GetOneGroupColorMatchedSet(GetColdStandardMaps)},
    {ColorMatchedSets::DIVERGING_ONLY_STD_MAPS, GetOneGroupColorMatchedSet(GetSlightlyDivergingSlimMaps)},
    {ColorMatchedSets::DIVERGING_BLACK_ONLY_STD_MAPS,
                                                GetOneGroupColorMatchedSet(GetDivergingBlackStandardMaps)},
    {ColorMatchedSets::WES_ANDERSON_ONLY_MAPS,  GetOneGroupColorMatchedSet(GetWesAndersonMaps)},
    {ColorMatchedSets::PASTEL_ONLY_MAPS,        GetOneGroupColorMatchedSet(GetPastelStandardMaps)},
    {ColorMatchedSets::COLOR_MATCHED_SET1,      GetColorMatchedSet1()},
    {ColorMatchedSets::COLOR_MATCHED_SET2,      GetColorMatchedSet2()},
    {ColorMatchedSets::COLOR_MATCHED_SET3,      GetColorMatchedSet3()},
    {ColorMatchedSets::COLOR_MATCHED_SET4,      GetColorMatchedSet4()},
    {ColorMatchedSets::COLOR_MATCHED_SET5,      GetColorMatchedSet5()},
    {ColorMatchedSets::COLOR_MATCHED_SET6,      GetColorMatchedSet6()},
    {ColorMatchedSets::COLOR_MATCHED_SET7,      GetColorMatchedSet7()},
    {ColorMatchedSets::COLOR_MATCHED_SET8,      GetColorMatchedSet8()},
  };
  // clang-format on
}

// clang-format off
static constexpr float RED_GREEN_STD_MAPS_WEIGHT            =   5.0F;
static constexpr float RED_BLUE_STD_MAPS_WEIGHT             =   5.0F;
static constexpr float YELLOW_BLUE_STD_MAPS_WEIGHT          =   5.0F;
static constexpr float YELLOW_PURPLE_STD_MAPS_WEIGHT        =   5.0F;
static constexpr float ORANGE_GREEN_STD_MAPS_WEIGHT         =   5.0F;
static constexpr float ORANGE_PURPLE_STD_MAPS_WEIGHT        =   5.0F;
static constexpr float ALL_ONLY_STD_MAPS_WEIGHT             =  20.0F;
static constexpr float HEAT_ONLY_STD_MAPS_WEIGHT            =  35.0F;
static constexpr float COLD_ONLY_STD_MAPS_WEIGHT            =  35.0F;
static constexpr float DIVERGING_ONLY_STD_MAPS_WEIGHT       =  40.0F;
static constexpr float DIVERGING_BLACK_ONLY_STD_MAPS_WEIGHT =  40.0F;
static constexpr float WES_ANDERSON_ONLY_MAPS_WEIGHT        =  40.0F;
static constexpr float PASTEL_ONLY_MAPS_WEIGHT              =  40.0F;
static constexpr float COLOR_MATCHED_SET1_WEIGHT            =  90.0F;
static constexpr float COLOR_MATCHED_SET2_WEIGHT            =  90.0F;
static constexpr float COLOR_MATCHED_SET3_WEIGHT            =  90.0F;
static constexpr float COLOR_MATCHED_SET4_WEIGHT            =  90.0F;
static constexpr float COLOR_MATCHED_SET5_WEIGHT            =  90.0F;
static constexpr float COLOR_MATCHED_SET6_WEIGHT            =  90.0F;
static constexpr float COLOR_MATCHED_SET7_WEIGHT            =  90.0F;
static constexpr float COLOR_MATCHED_SET8_WEIGHT            =  90.0F;
// clang-format on

VisualFxColorMaps::VisualFxColorMaps(const IGoomRand& goomRand)
  : m_goomRand{goomRand},
    // clang-format off
    m_colorMatchedSetsWeights
    {
        m_goomRand,
        {
            {ColorMatchedSets::RED_GREEN_STD_MAPS,            RED_GREEN_STD_MAPS_WEIGHT},
            {ColorMatchedSets::RED_BLUE_STD_MAPS,             RED_BLUE_STD_MAPS_WEIGHT},
            {ColorMatchedSets::YELLOW_BLUE_STD_MAPS,          YELLOW_BLUE_STD_MAPS_WEIGHT},
            {ColorMatchedSets::YELLOW_PURPLE_STD_MAPS,        YELLOW_PURPLE_STD_MAPS_WEIGHT},
            {ColorMatchedSets::ORANGE_GREEN_STD_MAPS,         ORANGE_GREEN_STD_MAPS_WEIGHT},
            {ColorMatchedSets::ORANGE_PURPLE_STD_MAPS,        ORANGE_PURPLE_STD_MAPS_WEIGHT},
            {ColorMatchedSets::ALL_ONLY_STD_MAPS,             ALL_ONLY_STD_MAPS_WEIGHT},
            {ColorMatchedSets::HEAT_ONLY_STD_MAPS,            HEAT_ONLY_STD_MAPS_WEIGHT},
            {ColorMatchedSets::COLD_ONLY_STD_MAPS,            COLD_ONLY_STD_MAPS_WEIGHT},
            {ColorMatchedSets::DIVERGING_ONLY_STD_MAPS,       DIVERGING_ONLY_STD_MAPS_WEIGHT},
            {ColorMatchedSets::DIVERGING_BLACK_ONLY_STD_MAPS, DIVERGING_BLACK_ONLY_STD_MAPS_WEIGHT},
            {ColorMatchedSets::WES_ANDERSON_ONLY_MAPS,        WES_ANDERSON_ONLY_MAPS_WEIGHT},
            {ColorMatchedSets::PASTEL_ONLY_MAPS,              PASTEL_ONLY_MAPS_WEIGHT},
            {ColorMatchedSets::COLOR_MATCHED_SET1,            COLOR_MATCHED_SET1_WEIGHT},
            {ColorMatchedSets::COLOR_MATCHED_SET2,            COLOR_MATCHED_SET2_WEIGHT},
            {ColorMatchedSets::COLOR_MATCHED_SET3,            COLOR_MATCHED_SET3_WEIGHT},
            {ColorMatchedSets::COLOR_MATCHED_SET4,            COLOR_MATCHED_SET4_WEIGHT},
            {ColorMatchedSets::COLOR_MATCHED_SET5,            COLOR_MATCHED_SET5_WEIGHT},
            {ColorMatchedSets::COLOR_MATCHED_SET6,            COLOR_MATCHED_SET6_WEIGHT},
            {ColorMatchedSets::COLOR_MATCHED_SET7,            COLOR_MATCHED_SET7_WEIGHT},
            {ColorMatchedSets::COLOR_MATCHED_SET8,            COLOR_MATCHED_SET8_WEIGHT},
        }
    }
// clang-format on
{
}

auto VisualFxColorMaps::GetNextColorMatchedSet() const -> const ColorMatchedSet&
{
  return m_colorMatchedSetsMap.at(m_colorMatchedSetsWeights.GetRandomWeighted());
}

// TODO: CLion is giving false positives for unreachable functions.
// For example, 'GetOneGroupColorMatchedSet'.
auto VisualFxColorMaps::GetOneGroupColorMatchedSet(const GetRandomColorMapsFunc& func)
    -> ColorMatchedSet
{
  ColorMatchedSet matchedSet{};

  for (uint32_t i = 0; i < NUM<GoomEffect>; ++i)
  {
    matchedSet.try_emplace(static_cast<GoomEffect>(i), func);
  }

  GetPrimaryColorDots(matchedSet);
  matchedSet.at(GoomEffect::LINES1) = GetMostlySequentialStandardMaps;
  matchedSet.at(GoomEffect::LINES2) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(GoomEffect::IMAGE) = GetAllSlimMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetTwoGroupsColorMatchedSet(const GetRandomColorMapsFunc& func1,
                                                    const GetRandomColorMapsFunc& func2) const
    -> ColorMatchedSet
{
  ColorMatchedSet matchedSet = GetOneGroupColorMatchedSet(func1);

  // Change every second map entry to 'func2'.
  std::array<size_t, NUM<GoomEffect>> indexes{};
  std::iota(begin(indexes), end(indexes), 0);
  m_goomRand.Shuffle(begin(indexes), end(indexes));
  static constexpr size_t INC_BY_2 = 2;
  for (size_t i = 0; i < NUM<GoomEffect>; i += INC_BY_2)
  {
    matchedSet.at(static_cast<GoomEffect>(indexes.at(i))) = func2;
  }

  GetPrimaryColorDots(matchedSet);
  matchedSet.at(GoomEffect::LINES1) = GetMostlySequentialStandardMaps;
  matchedSet.at(GoomEffect::LINES2) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(GoomEffect::IMAGE) = GetAllSlimMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetPrimaryColorDots(ColorMatchedSet& matchedSet) -> void
{
  matchedSet.at(GoomEffect::DOTS0) = GetRedStandardMaps;
  matchedSet.at(GoomEffect::DOTS1) = GetBlueStandardMaps;
  matchedSet.at(GoomEffect::DOTS2) = GetGreenStandardMaps;
  matchedSet.at(GoomEffect::DOTS3) = GetYellowStandardMaps;
  matchedSet.at(GoomEffect::DOTS4) = GetPurpleStandardMaps;
}

auto VisualFxColorMaps::GetColorMatchedSet1() -> ColorMatchedSet
{
  return {
      {               GoomEffect::DOTS0,               GetRedStandardMaps},
      {               GoomEffect::DOTS1,              GetBlueStandardMaps},
      {               GoomEffect::DOTS2,             GetGreenStandardMaps},
      {               GoomEffect::DOTS3,            GetYellowStandardMaps},
      {               GoomEffect::DOTS4,            GetPurpleStandardMaps},
      {             GoomEffect::CIRCLES, GetSlightlyDivergingStandardMaps},
      {         GoomEffect::CIRCLES_LOW, GetSlightlyDivergingStandardMaps},
      {                 GoomEffect::IFS, GetSlightlyDivergingStandardMaps},
      {               GoomEffect::IMAGE, GetSlightlyDivergingStandardMaps},
      {              GoomEffect::LINES1,  GetMostlySequentialStandardMaps},
      {              GoomEffect::LINES2, GetSlightlyDivergingStandardMaps},
      {         GoomEffect::SHAPES_MAIN,      GetMostlySequentialSlimMaps},
      {          GoomEffect::SHAPES_LOW, GetSlightlyDivergingStandardMaps},
      {        GoomEffect::SHAPES_INNER,      GetMostlySequentialSlimMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,      GetMostlySequentialSlimMaps},
      { GoomEffect::STARS_LOW_FIREWORKS, GetSlightlyDivergingStandardMaps},
      {     GoomEffect::STARS_MAIN_RAIN,           GetSeasonsStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,            GetCitiesStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,              GetHeatStandardMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,              GetColdStandardMaps},
      {           GoomEffect::TENTACLES,     GetSlightlyDivergingSlimMaps},
      {                GoomEffect::TUBE, GetSlightlyDivergingStandardMaps},
      {            GoomEffect::TUBE_LOW,     GetSlightlyDivergingSlimMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet2() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,     GetSlightlyDivergingSlimMaps},
      {         GoomEffect::CIRCLES_LOW,     GetSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,            GetOrangeStandardMaps},
      {               GoomEffect::DOTS1,            GetPurpleStandardMaps},
      {               GoomEffect::DOTS2,             GetGreenStandardMaps},
      {               GoomEffect::DOTS3,            GetYellowStandardMaps},
      {               GoomEffect::DOTS4,               GetRedStandardMaps},
      {                 GoomEffect::IFS,     GetSlightlyDivergingSlimMaps},
      {               GoomEffect::IMAGE,     GetSlightlyDivergingSlimMaps},
      {              GoomEffect::LINES1,     GetSlightlyDivergingSlimMaps},
      {              GoomEffect::LINES2, GetSlightlyDivergingStandardMaps},
      {         GoomEffect::SHAPES_MAIN,              GetHeatStandardMaps},
      {          GoomEffect::SHAPES_LOW,           GetSeasonsStandardMaps},
      {        GoomEffect::SHAPES_INNER,              GetColdStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,              GetHeatStandardMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,                   GetAllSlimMaps},
      {     GoomEffect::STARS_MAIN_RAIN,              GetColdStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,           GetSeasonsStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,              GetBlueStandardMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,            GetYellowStandardMaps},
      {           GoomEffect::TENTACLES,            GetYellowStandardMaps},
      {                GoomEffect::TUBE,            GetYellowStandardMaps},
      {            GoomEffect::TUBE_LOW,              GetBlueStandardMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet3() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,              GetColdStandardMaps},
      {         GoomEffect::CIRCLES_LOW,     GetSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,               GetRedStandardMaps},
      {               GoomEffect::DOTS1,              GetBlueStandardMaps},
      {               GoomEffect::DOTS2,            GetOrangeStandardMaps},
      {               GoomEffect::DOTS3,            GetYellowStandardMaps},
      {               GoomEffect::DOTS4,             GetGreenStandardMaps},
      {                 GoomEffect::IFS,              GetColdStandardMaps},
      {               GoomEffect::IMAGE,  GetMostlySequentialStandardMaps},
      {              GoomEffect::LINES1,                   GetAllSlimMaps},
      {              GoomEffect::LINES2,              GetBlueStandardMaps},
      {         GoomEffect::SHAPES_MAIN,            GetPastelStandardMaps},
      {          GoomEffect::SHAPES_LOW,    GetDivergingBlackStandardMaps},
      {        GoomEffect::SHAPES_INNER, GetSlightlyDivergingStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,     GetSlightlyDivergingSlimMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,              GetBlueStandardMaps},
      {     GoomEffect::STARS_MAIN_RAIN,               GetWesAndersonMaps},
      {      GoomEffect::STARS_LOW_RAIN,           GetSeasonsStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,                   GetAllSlimMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,            GetPastelStandardMaps},
      {           GoomEffect::TENTACLES,  GetMostlySequentialStandardMaps},
      {                GoomEffect::TUBE,  GetMostlySequentialStandardMaps},
      {            GoomEffect::TUBE_LOW,              GetHeatStandardMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet4() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,            GetCitiesStandardMaps},
      {         GoomEffect::CIRCLES_LOW,     GetSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,               GetWesAndersonMaps},
      {               GoomEffect::DOTS1,            GetCitiesStandardMaps},
      {               GoomEffect::DOTS2,           GetSeasonsStandardMaps},
      {               GoomEffect::DOTS3,              GetHeatStandardMaps},
      {               GoomEffect::DOTS4,              GetColdStandardMaps},
      {                 GoomEffect::IFS,            GetCitiesStandardMaps},
      {               GoomEffect::IMAGE,            GetCitiesStandardMaps},
      {              GoomEffect::LINES1, GetSlightlyDivergingStandardMaps},
      {              GoomEffect::LINES2,               GetRedStandardMaps},
      {         GoomEffect::SHAPES_MAIN,            GetPurpleStandardMaps},
      {          GoomEffect::SHAPES_LOW,             GetGreenStandardMaps},
      {        GoomEffect::SHAPES_INNER,            GetYellowStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,              GetBlueStandardMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,  GetMostlySequentialStandardMaps},
      {     GoomEffect::STARS_MAIN_RAIN,            GetCitiesStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,    GetDivergingBlackStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,                   GetAllSlimMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,           GetSeasonsStandardMaps},
      {           GoomEffect::TENTACLES,            GetPurpleStandardMaps},
      {                GoomEffect::TUBE,            GetPurpleStandardMaps},
      {            GoomEffect::TUBE_LOW,            GetPastelStandardMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet5() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,            GetPastelStandardMaps},
      {         GoomEffect::CIRCLES_LOW,     GetSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,    GetDivergingBlackStandardMaps},
      {               GoomEffect::DOTS1, GetSlightlyDivergingStandardMaps},
      {               GoomEffect::DOTS2,            GetPastelStandardMaps},
      {               GoomEffect::DOTS3,               GetWesAndersonMaps},
      {               GoomEffect::DOTS4,              GetHeatStandardMaps},
      {                 GoomEffect::IFS,            GetPastelStandardMaps},
      {               GoomEffect::IMAGE,            GetPastelStandardMaps},
      {              GoomEffect::LINES1, GetSlightlyDivergingStandardMaps},
      {              GoomEffect::LINES2,               GetRedStandardMaps},
      {         GoomEffect::SHAPES_MAIN,            GetPastelStandardMaps},
      {          GoomEffect::SHAPES_LOW, GetSlightlyDivergingStandardMaps},
      {        GoomEffect::SHAPES_INNER,    GetDivergingBlackStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,            GetPastelStandardMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,  GetMostlySequentialStandardMaps},
      {     GoomEffect::STARS_MAIN_RAIN, GetSlightlyDivergingStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,    GetDivergingBlackStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,               GetRedStandardMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,             GetGreenStandardMaps},
      {           GoomEffect::TENTACLES,           GetSeasonsStandardMaps},
      {                GoomEffect::TUBE,           GetSeasonsStandardMaps},
      {            GoomEffect::TUBE_LOW,              GetColdStandardMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet6() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,            GetPastelStandardMaps},
      {         GoomEffect::CIRCLES_LOW,     GetSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,               GetRedStandardMaps},
      {               GoomEffect::DOTS1,              GetBlueStandardMaps},
      {               GoomEffect::DOTS2,             GetGreenStandardMaps},
      {               GoomEffect::DOTS3,            GetYellowStandardMaps},
      {               GoomEffect::DOTS4,              GetHeatStandardMaps},
      {                 GoomEffect::IFS,            GetPastelStandardMaps},
      {               GoomEffect::IMAGE,            GetPastelStandardMaps},
      {              GoomEffect::LINES1, GetSlightlyDivergingStandardMaps},
      {              GoomEffect::LINES2,               GetRedStandardMaps},
      {         GoomEffect::SHAPES_MAIN,               GetAllStandardMaps},
      {          GoomEffect::SHAPES_LOW,            GetPastelStandardMaps},
      {        GoomEffect::SHAPES_INNER,              GetHeatStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,            GetPastelStandardMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,              GetColdStandardMaps},
      {     GoomEffect::STARS_MAIN_RAIN,              GetColdStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,            GetPastelStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,            GetPurpleStandardMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,           GetSeasonsStandardMaps},
      {           GoomEffect::TENTACLES,           GetSeasonsStandardMaps},
      {                GoomEffect::TUBE,           GetSeasonsStandardMaps},
      {            GoomEffect::TUBE_LOW,            GetCitiesStandardMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet7() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,            GetPastelStandardMaps},
      {         GoomEffect::CIRCLES_LOW,     GetSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,               GetRedStandardMaps},
      {               GoomEffect::DOTS1,              GetBlueStandardMaps},
      {               GoomEffect::DOTS2,             GetGreenStandardMaps},
      {               GoomEffect::DOTS3,            GetYellowStandardMaps},
      {               GoomEffect::DOTS4,              GetHeatStandardMaps},
      {                 GoomEffect::IFS,            GetPastelStandardMaps},
      {               GoomEffect::IMAGE,            GetPastelStandardMaps},
      {              GoomEffect::LINES1, GetSlightlyDivergingStandardMaps},
      {              GoomEffect::LINES2,               GetRedStandardMaps},
      {         GoomEffect::SHAPES_MAIN,               GetRedStandardMaps},
      {          GoomEffect::SHAPES_LOW,              GetBlueStandardMaps},
      {        GoomEffect::SHAPES_INNER,             GetGreenStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,            GetPastelStandardMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,             GetAllMapsUnweighted},
      {     GoomEffect::STARS_MAIN_RAIN,              GetHeatStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,      GetMostlySequentialSlimMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,              GetColdStandardMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,              GetColdStandardMaps},
      {           GoomEffect::TENTACLES,             GetGreenStandardMaps},
      {                GoomEffect::TUBE,             GetAllMapsUnweighted},
      {            GoomEffect::TUBE_LOW,                   GetAllSlimMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet8() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,              GetHeatStandardMaps},
      {         GoomEffect::CIRCLES_LOW,     GetSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,               GetRedStandardMaps},
      {               GoomEffect::DOTS1,              GetBlueStandardMaps},
      {               GoomEffect::DOTS2,             GetGreenStandardMaps},
      {               GoomEffect::DOTS3,            GetYellowStandardMaps},
      {               GoomEffect::DOTS4,              GetHeatStandardMaps},
      {                 GoomEffect::IFS,               GetRedStandardMaps},
      {               GoomEffect::IMAGE,               GetAllStandardMaps},
      {              GoomEffect::LINES1,               GetAllStandardMaps},
      {              GoomEffect::LINES2,               GetAllStandardMaps},
      {         GoomEffect::SHAPES_MAIN,               GetWesAndersonMaps},
      {          GoomEffect::SHAPES_LOW,               GetRedStandardMaps},
      {        GoomEffect::SHAPES_INNER, GetSlightlyDivergingStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,              GetBlueStandardMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,              GetBlueStandardMaps},
      {     GoomEffect::STARS_MAIN_RAIN,               GetRedStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,            GetYellowStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,             GetGreenStandardMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,            GetPurpleStandardMaps},
      {           GoomEffect::TENTACLES,            GetYellowStandardMaps},
      {                GoomEffect::TUBE,             GetGreenStandardMaps},
      {            GoomEffect::TUBE_LOW,              GetHeatStandardMaps},
  };
}

} // namespace GOOM::CONTROL
