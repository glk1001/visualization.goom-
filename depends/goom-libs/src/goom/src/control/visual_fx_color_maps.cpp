#include "visual_fx_color_maps.h"

#include "color/random_colormaps.h"
#include "utils/enumutils.h"
#include "utils/math/goom_rand_base.h"

#include <array>
#include <numeric>

namespace GOOM::CONTROL
{

using COLOR::MakeSharedAllMapsUnweighted;
using COLOR::MakeSharedAllSlimMaps;
using COLOR::MakeSharedAllStandardMaps;
using COLOR::MakeSharedBlueStandardMaps;
using COLOR::MakeSharedCitiesStandardMaps;
using COLOR::MakeSharedColdStandardMaps;
using COLOR::MakeSharedDivergingBlackStandardMaps;
using COLOR::MakeSharedGreenStandardMaps;
using COLOR::MakeSharedHeatStandardMaps;
using COLOR::MakeSharedMostlySequentialSlimMaps;
using COLOR::MakeSharedMostlySequentialStandardMaps;
using COLOR::MakeSharedOrangeStandardMaps;
using COLOR::MakeSharedPastelStandardMaps;
using COLOR::MakeSharedPurpleStandardMaps;
using COLOR::MakeSharedRedStandardMaps;
using COLOR::MakeSharedSeasonsStandardMaps;
using COLOR::MakeSharedSlightlyDivergingSlimMaps;
using COLOR::MakeSharedSlightlyDivergingStandardMaps;
using COLOR::MakeSharedWesAndersonMaps;
using COLOR::MakeSharedYellowStandardMaps;
using UTILS::NUM;
using UTILS::MATH::IGoomRand;

auto VisualFxColorMaps::MakeColorMatchedSetsMap() const -> ColorMatchedSetsMap
{
  // clang-format off
  return { 
    {ColorMatchedSets::RED_GREEN_STD_MAPS,      GetTwoGroupsColorMatchedSet(MakeSharedRedStandardMaps,    MakeSharedGreenStandardMaps)},
    {ColorMatchedSets::RED_BLUE_STD_MAPS,       GetTwoGroupsColorMatchedSet(MakeSharedRedStandardMaps,    MakeSharedBlueStandardMaps)},
    {ColorMatchedSets::YELLOW_BLUE_STD_MAPS,    GetTwoGroupsColorMatchedSet(MakeSharedYellowStandardMaps, MakeSharedBlueStandardMaps)},
    {ColorMatchedSets::YELLOW_PURPLE_STD_MAPS,  GetTwoGroupsColorMatchedSet(MakeSharedYellowStandardMaps, MakeSharedPurpleStandardMaps)},
    {ColorMatchedSets::ORANGE_GREEN_STD_MAPS,   GetTwoGroupsColorMatchedSet(MakeSharedOrangeStandardMaps, MakeSharedGreenStandardMaps)},
    {ColorMatchedSets::ORANGE_PURPLE_STD_MAPS,  GetTwoGroupsColorMatchedSet(MakeSharedOrangeStandardMaps, MakeSharedPurpleStandardMaps)},
    {ColorMatchedSets::ALL_ONLY_STD_MAPS,       GetOneGroupColorMatchedSet(MakeSharedAllStandardMaps)},
    {ColorMatchedSets::HEAT_ONLY_STD_MAPS,      GetOneGroupColorMatchedSet(MakeSharedHeatStandardMaps)},
    {ColorMatchedSets::COLD_ONLY_STD_MAPS,      GetOneGroupColorMatchedSet(MakeSharedColdStandardMaps)},
    {ColorMatchedSets::DIVERGING_ONLY_STD_MAPS, GetOneGroupColorMatchedSet(MakeSharedSlightlyDivergingSlimMaps)},
    {ColorMatchedSets::DIVERGING_BLACK_ONLY_STD_MAPS,
                                                GetOneGroupColorMatchedSet(MakeSharedDivergingBlackStandardMaps)},
    {ColorMatchedSets::WES_ANDERSON_ONLY_MAPS,  GetOneGroupColorMatchedSet(MakeSharedWesAndersonMaps)},
    {ColorMatchedSets::PASTEL_ONLY_MAPS,        GetOneGroupColorMatchedSet(MakeSharedPastelStandardMaps)},
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
  matchedSet.at(GoomEffect::LINES1) = MakeSharedMostlySequentialStandardMaps;
  matchedSet.at(GoomEffect::LINES2) = MakeSharedSlightlyDivergingStandardMaps;
  matchedSet.at(GoomEffect::IMAGE) = MakeSharedAllSlimMaps;

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
  matchedSet.at(GoomEffect::LINES1) = MakeSharedMostlySequentialStandardMaps;
  matchedSet.at(GoomEffect::LINES2) = MakeSharedSlightlyDivergingStandardMaps;
  matchedSet.at(GoomEffect::IMAGE) = MakeSharedAllSlimMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetPrimaryColorDots(ColorMatchedSet& matchedSet) -> void
{
  matchedSet.at(GoomEffect::DOTS0) = MakeSharedRedStandardMaps;
  matchedSet.at(GoomEffect::DOTS1) = MakeSharedBlueStandardMaps;
  matchedSet.at(GoomEffect::DOTS2) = MakeSharedGreenStandardMaps;
  matchedSet.at(GoomEffect::DOTS3) = MakeSharedYellowStandardMaps;
  matchedSet.at(GoomEffect::DOTS4) = MakeSharedPurpleStandardMaps;
}

auto VisualFxColorMaps::GetColorMatchedSet1() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES, MakeSharedSlightlyDivergingStandardMaps},
      {         GoomEffect::CIRCLES_LOW, MakeSharedSlightlyDivergingStandardMaps},
      {               GoomEffect::DOTS0,               MakeSharedRedStandardMaps},
      {               GoomEffect::DOTS1,              MakeSharedBlueStandardMaps},
      {               GoomEffect::DOTS2,             MakeSharedGreenStandardMaps},
      {               GoomEffect::DOTS3,            MakeSharedYellowStandardMaps},
      {               GoomEffect::DOTS4,            MakeSharedPurpleStandardMaps},
      {                 GoomEffect::IFS, MakeSharedSlightlyDivergingStandardMaps},
      {               GoomEffect::IMAGE, MakeSharedSlightlyDivergingStandardMaps},
      {              GoomEffect::LINES1,  MakeSharedMostlySequentialStandardMaps},
      {              GoomEffect::LINES2, MakeSharedSlightlyDivergingStandardMaps},
      {         GoomEffect::SHAPES_MAIN,      MakeSharedMostlySequentialSlimMaps},
      {          GoomEffect::SHAPES_LOW, MakeSharedSlightlyDivergingStandardMaps},
      {        GoomEffect::SHAPES_INNER,      MakeSharedMostlySequentialSlimMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,      MakeSharedMostlySequentialSlimMaps},
      { GoomEffect::STARS_LOW_FIREWORKS, MakeSharedSlightlyDivergingStandardMaps},
      {     GoomEffect::STARS_MAIN_RAIN,           MakeSharedSeasonsStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,            MakeSharedCitiesStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,              MakeSharedHeatStandardMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,              MakeSharedColdStandardMaps},
      {           GoomEffect::TENTACLES,     MakeSharedSlightlyDivergingSlimMaps},
      {                GoomEffect::TUBE, MakeSharedSlightlyDivergingStandardMaps},
      {            GoomEffect::TUBE_LOW,     MakeSharedSlightlyDivergingSlimMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet2() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,     MakeSharedSlightlyDivergingSlimMaps},
      {         GoomEffect::CIRCLES_LOW,     MakeSharedSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,            MakeSharedOrangeStandardMaps},
      {               GoomEffect::DOTS1,            MakeSharedPurpleStandardMaps},
      {               GoomEffect::DOTS2,             MakeSharedGreenStandardMaps},
      {               GoomEffect::DOTS3,            MakeSharedYellowStandardMaps},
      {               GoomEffect::DOTS4,               MakeSharedRedStandardMaps},
      {                 GoomEffect::IFS,     MakeSharedSlightlyDivergingSlimMaps},
      {               GoomEffect::IMAGE,     MakeSharedSlightlyDivergingSlimMaps},
      {              GoomEffect::LINES1,     MakeSharedSlightlyDivergingSlimMaps},
      {              GoomEffect::LINES2, MakeSharedSlightlyDivergingStandardMaps},
      {         GoomEffect::SHAPES_MAIN,              MakeSharedHeatStandardMaps},
      {          GoomEffect::SHAPES_LOW,           MakeSharedSeasonsStandardMaps},
      {        GoomEffect::SHAPES_INNER,              MakeSharedColdStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,              MakeSharedHeatStandardMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,                   MakeSharedAllSlimMaps},
      {     GoomEffect::STARS_MAIN_RAIN,              MakeSharedColdStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,           MakeSharedSeasonsStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,              MakeSharedBlueStandardMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,            MakeSharedYellowStandardMaps},
      {           GoomEffect::TENTACLES,            MakeSharedYellowStandardMaps},
      {                GoomEffect::TUBE,            MakeSharedYellowStandardMaps},
      {            GoomEffect::TUBE_LOW,              MakeSharedBlueStandardMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet3() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,              MakeSharedColdStandardMaps},
      {         GoomEffect::CIRCLES_LOW,     MakeSharedSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,               MakeSharedRedStandardMaps},
      {               GoomEffect::DOTS1,              MakeSharedBlueStandardMaps},
      {               GoomEffect::DOTS2,            MakeSharedOrangeStandardMaps},
      {               GoomEffect::DOTS3,            MakeSharedYellowStandardMaps},
      {               GoomEffect::DOTS4,             MakeSharedGreenStandardMaps},
      {                 GoomEffect::IFS,              MakeSharedColdStandardMaps},
      {               GoomEffect::IMAGE,  MakeSharedMostlySequentialStandardMaps},
      {              GoomEffect::LINES1,                   MakeSharedAllSlimMaps},
      {              GoomEffect::LINES2,              MakeSharedBlueStandardMaps},
      {         GoomEffect::SHAPES_MAIN,            MakeSharedPastelStandardMaps},
      {          GoomEffect::SHAPES_LOW,    MakeSharedDivergingBlackStandardMaps},
      {        GoomEffect::SHAPES_INNER, MakeSharedSlightlyDivergingStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,     MakeSharedSlightlyDivergingSlimMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,              MakeSharedBlueStandardMaps},
      {     GoomEffect::STARS_MAIN_RAIN,               MakeSharedWesAndersonMaps},
      {      GoomEffect::STARS_LOW_RAIN,           MakeSharedSeasonsStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,                   MakeSharedAllSlimMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,            MakeSharedPastelStandardMaps},
      {           GoomEffect::TENTACLES,  MakeSharedMostlySequentialStandardMaps},
      {                GoomEffect::TUBE,  MakeSharedMostlySequentialStandardMaps},
      {            GoomEffect::TUBE_LOW,              MakeSharedHeatStandardMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet4() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,            MakeSharedCitiesStandardMaps},
      {         GoomEffect::CIRCLES_LOW,     MakeSharedSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,               MakeSharedWesAndersonMaps},
      {               GoomEffect::DOTS1,            MakeSharedCitiesStandardMaps},
      {               GoomEffect::DOTS2,           MakeSharedSeasonsStandardMaps},
      {               GoomEffect::DOTS3,              MakeSharedHeatStandardMaps},
      {               GoomEffect::DOTS4,              MakeSharedColdStandardMaps},
      {                 GoomEffect::IFS,            MakeSharedCitiesStandardMaps},
      {               GoomEffect::IMAGE,            MakeSharedCitiesStandardMaps},
      {              GoomEffect::LINES1, MakeSharedSlightlyDivergingStandardMaps},
      {              GoomEffect::LINES2,               MakeSharedRedStandardMaps},
      {         GoomEffect::SHAPES_MAIN,            MakeSharedPurpleStandardMaps},
      {          GoomEffect::SHAPES_LOW,             MakeSharedGreenStandardMaps},
      {        GoomEffect::SHAPES_INNER,            MakeSharedYellowStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,              MakeSharedBlueStandardMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,  MakeSharedMostlySequentialStandardMaps},
      {     GoomEffect::STARS_MAIN_RAIN,            MakeSharedCitiesStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,    MakeSharedDivergingBlackStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,                   MakeSharedAllSlimMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,           MakeSharedSeasonsStandardMaps},
      {           GoomEffect::TENTACLES,            MakeSharedPurpleStandardMaps},
      {                GoomEffect::TUBE,            MakeSharedPurpleStandardMaps},
      {            GoomEffect::TUBE_LOW,            MakeSharedPastelStandardMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet5() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,            MakeSharedPastelStandardMaps},
      {         GoomEffect::CIRCLES_LOW,     MakeSharedSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,    MakeSharedDivergingBlackStandardMaps},
      {               GoomEffect::DOTS1, MakeSharedSlightlyDivergingStandardMaps},
      {               GoomEffect::DOTS2,            MakeSharedPastelStandardMaps},
      {               GoomEffect::DOTS3,               MakeSharedWesAndersonMaps},
      {               GoomEffect::DOTS4,              MakeSharedHeatStandardMaps},
      {                 GoomEffect::IFS,            MakeSharedPastelStandardMaps},
      {               GoomEffect::IMAGE,            MakeSharedPastelStandardMaps},
      {              GoomEffect::LINES1, MakeSharedSlightlyDivergingStandardMaps},
      {              GoomEffect::LINES2,               MakeSharedRedStandardMaps},
      {         GoomEffect::SHAPES_MAIN,            MakeSharedPastelStandardMaps},
      {          GoomEffect::SHAPES_LOW, MakeSharedSlightlyDivergingStandardMaps},
      {        GoomEffect::SHAPES_INNER,    MakeSharedDivergingBlackStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,            MakeSharedPastelStandardMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,  MakeSharedMostlySequentialStandardMaps},
      {     GoomEffect::STARS_MAIN_RAIN, MakeSharedSlightlyDivergingStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,    MakeSharedDivergingBlackStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,               MakeSharedRedStandardMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,             MakeSharedGreenStandardMaps},
      {           GoomEffect::TENTACLES,           MakeSharedSeasonsStandardMaps},
      {                GoomEffect::TUBE,           MakeSharedSeasonsStandardMaps},
      {            GoomEffect::TUBE_LOW,              MakeSharedColdStandardMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet6() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,            MakeSharedPastelStandardMaps},
      {         GoomEffect::CIRCLES_LOW,     MakeSharedSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,               MakeSharedRedStandardMaps},
      {               GoomEffect::DOTS1,              MakeSharedBlueStandardMaps},
      {               GoomEffect::DOTS2,             MakeSharedGreenStandardMaps},
      {               GoomEffect::DOTS3,            MakeSharedYellowStandardMaps},
      {               GoomEffect::DOTS4,              MakeSharedHeatStandardMaps},
      {                 GoomEffect::IFS,            MakeSharedPastelStandardMaps},
      {               GoomEffect::IMAGE,            MakeSharedPastelStandardMaps},
      {              GoomEffect::LINES1, MakeSharedSlightlyDivergingStandardMaps},
      {              GoomEffect::LINES2,               MakeSharedRedStandardMaps},
      {         GoomEffect::SHAPES_MAIN,               MakeSharedAllStandardMaps},
      {          GoomEffect::SHAPES_LOW,            MakeSharedPastelStandardMaps},
      {        GoomEffect::SHAPES_INNER,              MakeSharedHeatStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,            MakeSharedPastelStandardMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,              MakeSharedColdStandardMaps},
      {     GoomEffect::STARS_MAIN_RAIN,              MakeSharedColdStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,            MakeSharedPastelStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,            MakeSharedPurpleStandardMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,           MakeSharedSeasonsStandardMaps},
      {           GoomEffect::TENTACLES,           MakeSharedSeasonsStandardMaps},
      {                GoomEffect::TUBE,           MakeSharedSeasonsStandardMaps},
      {            GoomEffect::TUBE_LOW,            MakeSharedCitiesStandardMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet7() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,            MakeSharedPastelStandardMaps},
      {         GoomEffect::CIRCLES_LOW,     MakeSharedSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,               MakeSharedRedStandardMaps},
      {               GoomEffect::DOTS1,              MakeSharedBlueStandardMaps},
      {               GoomEffect::DOTS2,             MakeSharedGreenStandardMaps},
      {               GoomEffect::DOTS3,            MakeSharedYellowStandardMaps},
      {               GoomEffect::DOTS4,              MakeSharedHeatStandardMaps},
      {                 GoomEffect::IFS,            MakeSharedPastelStandardMaps},
      {               GoomEffect::IMAGE,            MakeSharedPastelStandardMaps},
      {              GoomEffect::LINES1, MakeSharedSlightlyDivergingStandardMaps},
      {              GoomEffect::LINES2,               MakeSharedRedStandardMaps},
      {         GoomEffect::SHAPES_MAIN,               MakeSharedRedStandardMaps},
      {          GoomEffect::SHAPES_LOW,              MakeSharedBlueStandardMaps},
      {        GoomEffect::SHAPES_INNER,             MakeSharedGreenStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,            MakeSharedPastelStandardMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,             MakeSharedAllMapsUnweighted},
      {     GoomEffect::STARS_MAIN_RAIN,              MakeSharedHeatStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,      MakeSharedMostlySequentialSlimMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,              MakeSharedColdStandardMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,              MakeSharedColdStandardMaps},
      {           GoomEffect::TENTACLES,             MakeSharedGreenStandardMaps},
      {                GoomEffect::TUBE,             MakeSharedAllMapsUnweighted},
      {            GoomEffect::TUBE_LOW,                   MakeSharedAllSlimMaps},
  };
}

auto VisualFxColorMaps::GetColorMatchedSet8() -> ColorMatchedSet
{
  return {
      {             GoomEffect::CIRCLES,              MakeSharedHeatStandardMaps},
      {         GoomEffect::CIRCLES_LOW,     MakeSharedSlightlyDivergingSlimMaps},
      {               GoomEffect::DOTS0,               MakeSharedRedStandardMaps},
      {               GoomEffect::DOTS1,              MakeSharedBlueStandardMaps},
      {               GoomEffect::DOTS2,             MakeSharedGreenStandardMaps},
      {               GoomEffect::DOTS3,            MakeSharedYellowStandardMaps},
      {               GoomEffect::DOTS4,              MakeSharedHeatStandardMaps},
      {                 GoomEffect::IFS,               MakeSharedRedStandardMaps},
      {               GoomEffect::IMAGE,               MakeSharedAllStandardMaps},
      {              GoomEffect::LINES1,               MakeSharedAllStandardMaps},
      {              GoomEffect::LINES2,               MakeSharedAllStandardMaps},
      {         GoomEffect::SHAPES_MAIN,               MakeSharedWesAndersonMaps},
      {          GoomEffect::SHAPES_LOW,               MakeSharedRedStandardMaps},
      {        GoomEffect::SHAPES_INNER, MakeSharedSlightlyDivergingStandardMaps},
      {GoomEffect::STARS_MAIN_FIREWORKS,              MakeSharedBlueStandardMaps},
      { GoomEffect::STARS_LOW_FIREWORKS,              MakeSharedBlueStandardMaps},
      {     GoomEffect::STARS_MAIN_RAIN,               MakeSharedRedStandardMaps},
      {      GoomEffect::STARS_LOW_RAIN,            MakeSharedYellowStandardMaps},
      { GoomEffect::STARS_MAIN_FOUNTAIN,             MakeSharedGreenStandardMaps},
      {  GoomEffect::STARS_LOW_FOUNTAIN,            MakeSharedPurpleStandardMaps},
      {           GoomEffect::TENTACLES,            MakeSharedYellowStandardMaps},
      {                GoomEffect::TUBE,             MakeSharedGreenStandardMaps},
      {            GoomEffect::TUBE_LOW,              MakeSharedHeatStandardMaps},
  };
}

} // namespace GOOM::CONTROL
