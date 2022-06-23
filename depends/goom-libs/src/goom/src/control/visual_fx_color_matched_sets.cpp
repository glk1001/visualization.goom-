#include "visual_fx_color_matched_sets.h"

#include "color/random_color_maps_groups.h"
#include "utils/enumutils.h"
#include "utils/math/goom_rand_base.h"

#include <array>
#include <map>
#include <numeric>

namespace GOOM::CONTROL
{

using COLOR::RandomColorMapsGroups;
using UTILS::NUM;
using UTILS::MATH::IGoomRand;

using Groups = RandomColorMapsGroups::Groups;

auto VisualFxColorMatchedSets::MakeColorMatchedSetsMap() const noexcept -> ColorMatchedSetsMap
{
  // clang-format off
  return {
    {ColorMatchedSets::RED_GREEN_STD_MAPS,      GetTwoGroupsColorMatchedSet(Groups::RED_STANDARD_MAPS,    Groups::GREEN_STANDARD_MAPS)},
    {ColorMatchedSets::RED_BLUE_STD_MAPS,       GetTwoGroupsColorMatchedSet(Groups::RED_STANDARD_MAPS,    Groups::BLUE_STANDARD_MAPS)},
    {ColorMatchedSets::YELLOW_BLUE_STD_MAPS,    GetTwoGroupsColorMatchedSet(Groups::YELLOW_STANDARD_MAPS, Groups::BLUE_STANDARD_MAPS)},
    {ColorMatchedSets::YELLOW_PURPLE_STD_MAPS,  GetTwoGroupsColorMatchedSet(Groups::YELLOW_STANDARD_MAPS, Groups::PURPLE_STANDARD_MAPS)},
    {ColorMatchedSets::ORANGE_GREEN_STD_MAPS,   GetTwoGroupsColorMatchedSet(Groups::ORANGE_STANDARD_MAPS, Groups::GREEN_STANDARD_MAPS)},
    {ColorMatchedSets::ORANGE_PURPLE_STD_MAPS,  GetTwoGroupsColorMatchedSet(Groups::ORANGE_STANDARD_MAPS, Groups::PURPLE_STANDARD_MAPS)},
    {ColorMatchedSets::ALL_ONLY_STD_MAPS,       GetOneGroupColorMatchedSet(Groups::ALL_STANDARD_MAPS)},
    {ColorMatchedSets::HEAT_ONLY_STD_MAPS,      GetOneGroupColorMatchedSet(Groups::HEAT_STANDARD_MAPS)},
    {ColorMatchedSets::COLD_ONLY_STD_MAPS,      GetOneGroupColorMatchedSet(Groups::COLD_STANDARD_MAPS)},
    {ColorMatchedSets::DIVERGING_ONLY_STD_MAPS, GetOneGroupColorMatchedSet(Groups::SLIGHTLY_DIVERGING_SLIM_MAPS)},
    {ColorMatchedSets::DIVERGING_BLACK_ONLY_STD_MAPS,
                                                GetOneGroupColorMatchedSet(Groups::DIVERGING_BLACK_STANDARD_MAPS)},
    {ColorMatchedSets::WES_ANDERSON_ONLY_MAPS,  GetOneGroupColorMatchedSet(Groups::WES_ANDERSON_MAPS)},
    {ColorMatchedSets::PASTEL_ONLY_MAPS,        GetOneGroupColorMatchedSet(Groups::PASTEL_STANDARD_MAPS)},
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


VisualFxColorMatchedSets::VisualFxColorMatchedSets(const IGoomRand& goomRand) noexcept
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

auto VisualFxColorMatchedSets::GetNextRandomColorMatchedSet() const noexcept
    -> const ColorMatchedSet&
{
  return m_colorMatchedSetsMap.at(m_colorMatchedSetsWeights.GetRandomWeighted());
}

// TODO: CLion is giving false positives for unreachable functions.
// For example, 'GetOneGroupColorMatchedSet'.
auto VisualFxColorMatchedSets::GetOneGroupColorMatchedSet(const Groups group) noexcept
    -> ColorMatchedSet
{
  ColorMatchedSet matchedSet{};

  for (uint32_t i = 0; i < NUM<GoomEffect>; ++i)
  {
    matchedSet.try_emplace(static_cast<GoomEffect>(i), group);
  }

  GetPrimaryColorDots(matchedSet);
  matchedSet.at(GoomEffect::LINES1) = Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS;
  matchedSet.at(GoomEffect::LINES2) = Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS;
  matchedSet.at(GoomEffect::IMAGE) = Groups::ALL_SLIM_MAPS;

  return matchedSet;
}

auto VisualFxColorMatchedSets::GetTwoGroupsColorMatchedSet(const Groups group1,
                                                           const Groups group2) const noexcept
    -> ColorMatchedSet
{
  ColorMatchedSet matchedSet = GetOneGroupColorMatchedSet(group1);

  // Change every second map entry to 'func2'.
  std::array<size_t, NUM<GoomEffect>> indexes{};
  std::iota(begin(indexes), end(indexes), 0);
  m_goomRand.Shuffle(begin(indexes), end(indexes));
  static constexpr size_t INC_BY_2 = 2;
  for (size_t i = 0; i < NUM<GoomEffect>; i += INC_BY_2)
  {
    matchedSet.at(static_cast<GoomEffect>(indexes.at(i))) = group2;
  }

  GetPrimaryColorDots(matchedSet);
  matchedSet.at(GoomEffect::LINES1) = Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS;
  matchedSet.at(GoomEffect::LINES2) = Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS;
  matchedSet.at(GoomEffect::IMAGE) = Groups::ALL_SLIM_MAPS;

  return matchedSet;
}

auto VisualFxColorMatchedSets::GetPrimaryColorDots(ColorMatchedSet& matchedSet) noexcept -> void
{
  matchedSet.at(GoomEffect::DOTS0) = Groups::RED_STANDARD_MAPS;
  matchedSet.at(GoomEffect::DOTS1) = Groups::BLUE_STANDARD_MAPS;
  matchedSet.at(GoomEffect::DOTS2) = Groups::GREEN_STANDARD_MAPS;
  matchedSet.at(GoomEffect::DOTS3) = Groups::YELLOW_STANDARD_MAPS;
  matchedSet.at(GoomEffect::DOTS4) = Groups::PURPLE_STANDARD_MAPS;
}

auto VisualFxColorMatchedSets::GetColorMatchedSet1() noexcept -> ColorMatchedSet
{
  return {
      {        GoomEffect::CIRCLES_MAIN,                Groups::ALL_STANDARD_MAPS},
      {         GoomEffect::CIRCLES_LOW, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {               GoomEffect::DOTS0,                Groups::RED_STANDARD_MAPS},
      {               GoomEffect::DOTS1,               Groups::BLUE_STANDARD_MAPS},
      {               GoomEffect::DOTS2,              Groups::GREEN_STANDARD_MAPS},
      {               GoomEffect::DOTS3,             Groups::YELLOW_STANDARD_MAPS},
      {               GoomEffect::DOTS4,             Groups::PURPLE_STANDARD_MAPS},
      {                 GoomEffect::IFS, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {               GoomEffect::IMAGE,                Groups::ALL_STANDARD_MAPS},
      {              GoomEffect::LINES1,  Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS},
      {              GoomEffect::LINES2, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {         GoomEffect::SHAPES_MAIN,      Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS},
      {          GoomEffect::SHAPES_LOW, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {        GoomEffect::SHAPES_INNER,             Groups::CITIES_STANDARD_MAPS},
      {GoomEffect::STARS_MAIN_FIREWORKS,      Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS},
      { GoomEffect::STARS_LOW_FIREWORKS, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {     GoomEffect::STARS_MAIN_RAIN,            Groups::SEASONS_STANDARD_MAPS},
      {      GoomEffect::STARS_LOW_RAIN,             Groups::CITIES_STANDARD_MAPS},
      { GoomEffect::STARS_MAIN_FOUNTAIN,               Groups::HEAT_STANDARD_MAPS},
      {  GoomEffect::STARS_LOW_FOUNTAIN,               Groups::COLD_STANDARD_MAPS},
      {           GoomEffect::TENTACLES,                Groups::ALL_STANDARD_MAPS},
      {           GoomEffect::TUBE_MAIN, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {            GoomEffect::TUBE_LOW,     Groups::SLIGHTLY_DIVERGING_SLIM_MAPS},
  };
}

auto VisualFxColorMatchedSets::GetColorMatchedSet2() noexcept -> ColorMatchedSet
{
  return {
      {        GoomEffect::CIRCLES_MAIN,            Groups::ALL_STANDARD_MAPS},
      {         GoomEffect::CIRCLES_LOW, Groups::SLIGHTLY_DIVERGING_SLIM_MAPS},
      {               GoomEffect::DOTS0,         Groups::ORANGE_STANDARD_MAPS},
      {               GoomEffect::DOTS1,         Groups::PURPLE_STANDARD_MAPS},
      {               GoomEffect::DOTS2,          Groups::GREEN_STANDARD_MAPS},
      {               GoomEffect::DOTS3,         Groups::YELLOW_STANDARD_MAPS},
      {               GoomEffect::DOTS4,            Groups::RED_STANDARD_MAPS},
      {                 GoomEffect::IFS,  Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS},
      {               GoomEffect::IMAGE, Groups::SLIGHTLY_DIVERGING_SLIM_MAPS},
      {              GoomEffect::LINES1, Groups::SLIGHTLY_DIVERGING_SLIM_MAPS},
      {              GoomEffect::LINES2,           Groups::COLD_STANDARD_MAPS},
      {         GoomEffect::SHAPES_MAIN,           Groups::HEAT_STANDARD_MAPS},
      {          GoomEffect::SHAPES_LOW,        Groups::SEASONS_STANDARD_MAPS},
      {        GoomEffect::SHAPES_INNER,           Groups::COLD_STANDARD_MAPS},
      {GoomEffect::STARS_MAIN_FIREWORKS,           Groups::HEAT_STANDARD_MAPS},
      { GoomEffect::STARS_LOW_FIREWORKS,                Groups::ALL_SLIM_MAPS},
      {     GoomEffect::STARS_MAIN_RAIN,           Groups::COLD_STANDARD_MAPS},
      {      GoomEffect::STARS_LOW_RAIN,        Groups::SEASONS_STANDARD_MAPS},
      { GoomEffect::STARS_MAIN_FOUNTAIN,           Groups::BLUE_STANDARD_MAPS},
      {  GoomEffect::STARS_LOW_FOUNTAIN,         Groups::YELLOW_STANDARD_MAPS},
      {           GoomEffect::TENTACLES,         Groups::YELLOW_STANDARD_MAPS},
      {           GoomEffect::TUBE_MAIN,          Groups::GREEN_STANDARD_MAPS},
      {            GoomEffect::TUBE_LOW,           Groups::BLUE_STANDARD_MAPS},
  };
}

auto VisualFxColorMatchedSets::GetColorMatchedSet3() noexcept -> ColorMatchedSet
{
  return {
      {        GoomEffect::CIRCLES_MAIN,               Groups::COLD_STANDARD_MAPS},
      {         GoomEffect::CIRCLES_LOW,     Groups::SLIGHTLY_DIVERGING_SLIM_MAPS},
      {               GoomEffect::DOTS0,                Groups::RED_STANDARD_MAPS},
      {               GoomEffect::DOTS1,               Groups::BLUE_STANDARD_MAPS},
      {               GoomEffect::DOTS2,             Groups::ORANGE_STANDARD_MAPS},
      {               GoomEffect::DOTS3,             Groups::YELLOW_STANDARD_MAPS},
      {               GoomEffect::DOTS4,              Groups::GREEN_STANDARD_MAPS},
      {                 GoomEffect::IFS,               Groups::COLD_STANDARD_MAPS},
      {               GoomEffect::IMAGE,  Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS},
      {              GoomEffect::LINES1,                    Groups::ALL_SLIM_MAPS},
      {              GoomEffect::LINES2,               Groups::BLUE_STANDARD_MAPS},
      {         GoomEffect::SHAPES_MAIN,             Groups::PASTEL_STANDARD_MAPS},
      {          GoomEffect::SHAPES_LOW,    Groups::DIVERGING_BLACK_STANDARD_MAPS},
      {        GoomEffect::SHAPES_INNER, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {GoomEffect::STARS_MAIN_FIREWORKS,     Groups::SLIGHTLY_DIVERGING_SLIM_MAPS},
      { GoomEffect::STARS_LOW_FIREWORKS,               Groups::BLUE_STANDARD_MAPS},
      {     GoomEffect::STARS_MAIN_RAIN,                Groups::WES_ANDERSON_MAPS},
      {      GoomEffect::STARS_LOW_RAIN,            Groups::SEASONS_STANDARD_MAPS},
      { GoomEffect::STARS_MAIN_FOUNTAIN,                    Groups::ALL_SLIM_MAPS},
      {  GoomEffect::STARS_LOW_FOUNTAIN,             Groups::PASTEL_STANDARD_MAPS},
      {           GoomEffect::TENTACLES,  Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS},
      {           GoomEffect::TUBE_MAIN,            Groups::SEASONS_STANDARD_MAPS},
      {            GoomEffect::TUBE_LOW,               Groups::HEAT_STANDARD_MAPS},
  };
}

auto VisualFxColorMatchedSets::GetColorMatchedSet4() noexcept -> ColorMatchedSet
{
  return {
      {        GoomEffect::CIRCLES_MAIN,             Groups::CITIES_STANDARD_MAPS},
      {         GoomEffect::CIRCLES_LOW,     Groups::SLIGHTLY_DIVERGING_SLIM_MAPS},
      {               GoomEffect::DOTS0,                Groups::WES_ANDERSON_MAPS},
      {               GoomEffect::DOTS1,             Groups::CITIES_STANDARD_MAPS},
      {               GoomEffect::DOTS2,            Groups::SEASONS_STANDARD_MAPS},
      {               GoomEffect::DOTS3,               Groups::HEAT_STANDARD_MAPS},
      {               GoomEffect::DOTS4,               Groups::COLD_STANDARD_MAPS},
      {                 GoomEffect::IFS,             Groups::CITIES_STANDARD_MAPS},
      {               GoomEffect::IMAGE,              Groups::ALL_MAPS_UNWEIGHTED},
      {              GoomEffect::LINES1, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {              GoomEffect::LINES2,                Groups::RED_STANDARD_MAPS},
      {         GoomEffect::SHAPES_MAIN,             Groups::PURPLE_STANDARD_MAPS},
      {          GoomEffect::SHAPES_LOW,                Groups::WES_ANDERSON_MAPS},
      {        GoomEffect::SHAPES_INNER,             Groups::YELLOW_STANDARD_MAPS},
      {GoomEffect::STARS_MAIN_FIREWORKS,               Groups::BLUE_STANDARD_MAPS},
      { GoomEffect::STARS_LOW_FIREWORKS,  Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS},
      {     GoomEffect::STARS_MAIN_RAIN,             Groups::CITIES_STANDARD_MAPS},
      {      GoomEffect::STARS_LOW_RAIN,    Groups::DIVERGING_BLACK_STANDARD_MAPS},
      { GoomEffect::STARS_MAIN_FOUNTAIN,                    Groups::ALL_SLIM_MAPS},
      {  GoomEffect::STARS_LOW_FOUNTAIN,            Groups::SEASONS_STANDARD_MAPS},
      {           GoomEffect::TENTACLES,             Groups::PURPLE_STANDARD_MAPS},
      {           GoomEffect::TUBE_MAIN,                    Groups::ALL_SLIM_MAPS},
      {            GoomEffect::TUBE_LOW,             Groups::PASTEL_STANDARD_MAPS},
  };
}

auto VisualFxColorMatchedSets::GetColorMatchedSet5() noexcept -> ColorMatchedSet
{
  return {
      {        GoomEffect::CIRCLES_MAIN,             Groups::PASTEL_STANDARD_MAPS},
      {         GoomEffect::CIRCLES_LOW,     Groups::SLIGHTLY_DIVERGING_SLIM_MAPS},
      {               GoomEffect::DOTS0,    Groups::DIVERGING_BLACK_STANDARD_MAPS},
      {               GoomEffect::DOTS1, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {               GoomEffect::DOTS2,             Groups::PASTEL_STANDARD_MAPS},
      {               GoomEffect::DOTS3,                Groups::WES_ANDERSON_MAPS},
      {               GoomEffect::DOTS4,               Groups::HEAT_STANDARD_MAPS},
      {                 GoomEffect::IFS,             Groups::PASTEL_STANDARD_MAPS},
      {               GoomEffect::IMAGE,      Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS},
      {              GoomEffect::LINES1, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {              GoomEffect::LINES2,                Groups::RED_STANDARD_MAPS},
      {         GoomEffect::SHAPES_MAIN,             Groups::PASTEL_STANDARD_MAPS},
      {          GoomEffect::SHAPES_LOW, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {        GoomEffect::SHAPES_INNER,    Groups::DIVERGING_BLACK_STANDARD_MAPS},
      {GoomEffect::STARS_MAIN_FIREWORKS,             Groups::PASTEL_STANDARD_MAPS},
      { GoomEffect::STARS_LOW_FIREWORKS,  Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS},
      {     GoomEffect::STARS_MAIN_RAIN, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {      GoomEffect::STARS_LOW_RAIN,    Groups::DIVERGING_BLACK_STANDARD_MAPS},
      { GoomEffect::STARS_MAIN_FOUNTAIN,                Groups::RED_STANDARD_MAPS},
      {  GoomEffect::STARS_LOW_FOUNTAIN,              Groups::GREEN_STANDARD_MAPS},
      {           GoomEffect::TENTACLES,            Groups::SEASONS_STANDARD_MAPS},
      {           GoomEffect::TUBE_MAIN,                Groups::WES_ANDERSON_MAPS},
      {            GoomEffect::TUBE_LOW,               Groups::COLD_STANDARD_MAPS},
  };
}

auto VisualFxColorMatchedSets::GetColorMatchedSet6() noexcept -> ColorMatchedSet
{
  return {
      {        GoomEffect::CIRCLES_MAIN,             Groups::PASTEL_STANDARD_MAPS},
      {         GoomEffect::CIRCLES_LOW,     Groups::SLIGHTLY_DIVERGING_SLIM_MAPS},
      {               GoomEffect::DOTS0,                Groups::RED_STANDARD_MAPS},
      {               GoomEffect::DOTS1,               Groups::BLUE_STANDARD_MAPS},
      {               GoomEffect::DOTS2,              Groups::GREEN_STANDARD_MAPS},
      {               GoomEffect::DOTS3,             Groups::YELLOW_STANDARD_MAPS},
      {               GoomEffect::DOTS4,               Groups::HEAT_STANDARD_MAPS},
      {                 GoomEffect::IFS,             Groups::PASTEL_STANDARD_MAPS},
      {               GoomEffect::IMAGE,             Groups::CITIES_STANDARD_MAPS},
      {              GoomEffect::LINES1, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {              GoomEffect::LINES2,                Groups::RED_STANDARD_MAPS},
      {         GoomEffect::SHAPES_MAIN,                Groups::ALL_STANDARD_MAPS},
      {          GoomEffect::SHAPES_LOW,             Groups::PASTEL_STANDARD_MAPS},
      {        GoomEffect::SHAPES_INNER,               Groups::HEAT_STANDARD_MAPS},
      {GoomEffect::STARS_MAIN_FIREWORKS,             Groups::PASTEL_STANDARD_MAPS},
      { GoomEffect::STARS_LOW_FIREWORKS,               Groups::COLD_STANDARD_MAPS},
      {     GoomEffect::STARS_MAIN_RAIN,            Groups::SEASONS_STANDARD_MAPS},
      {      GoomEffect::STARS_LOW_RAIN,             Groups::PASTEL_STANDARD_MAPS},
      { GoomEffect::STARS_MAIN_FOUNTAIN,             Groups::PURPLE_STANDARD_MAPS},
      {  GoomEffect::STARS_LOW_FOUNTAIN,            Groups::SEASONS_STANDARD_MAPS},
      {           GoomEffect::TENTACLES,            Groups::SEASONS_STANDARD_MAPS},
      {           GoomEffect::TUBE_MAIN,    Groups::DIVERGING_BLACK_STANDARD_MAPS},
      {            GoomEffect::TUBE_LOW,             Groups::CITIES_STANDARD_MAPS},
  };
}

auto VisualFxColorMatchedSets::GetColorMatchedSet7() noexcept -> ColorMatchedSet
{
  return {
      {        GoomEffect::CIRCLES_MAIN,             Groups::PASTEL_STANDARD_MAPS},
      {         GoomEffect::CIRCLES_LOW,     Groups::SLIGHTLY_DIVERGING_SLIM_MAPS},
      {               GoomEffect::DOTS0,                Groups::RED_STANDARD_MAPS},
      {               GoomEffect::DOTS1,               Groups::BLUE_STANDARD_MAPS},
      {               GoomEffect::DOTS2,              Groups::GREEN_STANDARD_MAPS},
      {               GoomEffect::DOTS3,             Groups::YELLOW_STANDARD_MAPS},
      {               GoomEffect::DOTS4,               Groups::HEAT_STANDARD_MAPS},
      {                 GoomEffect::IFS,    Groups::DIVERGING_BLACK_STANDARD_MAPS},
      {               GoomEffect::IMAGE,                Groups::ALL_STANDARD_MAPS},
      {              GoomEffect::LINES1, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {              GoomEffect::LINES2,                Groups::RED_STANDARD_MAPS},
      {         GoomEffect::SHAPES_MAIN,                Groups::RED_STANDARD_MAPS},
      {          GoomEffect::SHAPES_LOW,      Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS},
      {        GoomEffect::SHAPES_INNER,              Groups::GREEN_STANDARD_MAPS},
      {GoomEffect::STARS_MAIN_FIREWORKS,             Groups::PASTEL_STANDARD_MAPS},
      { GoomEffect::STARS_LOW_FIREWORKS,              Groups::ALL_MAPS_UNWEIGHTED},
      {     GoomEffect::STARS_MAIN_RAIN,               Groups::HEAT_STANDARD_MAPS},
      {      GoomEffect::STARS_LOW_RAIN,      Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS},
      { GoomEffect::STARS_MAIN_FOUNTAIN,               Groups::COLD_STANDARD_MAPS},
      {  GoomEffect::STARS_LOW_FOUNTAIN,               Groups::COLD_STANDARD_MAPS},
      {           GoomEffect::TENTACLES,              Groups::GREEN_STANDARD_MAPS},
      {           GoomEffect::TUBE_MAIN,              Groups::ALL_MAPS_UNWEIGHTED},
      {            GoomEffect::TUBE_LOW,                    Groups::ALL_SLIM_MAPS},
  };
}

auto VisualFxColorMatchedSets::GetColorMatchedSet8() noexcept -> ColorMatchedSet
{
  return {
      {        GoomEffect::CIRCLES_MAIN,               Groups::HEAT_STANDARD_MAPS},
      {         GoomEffect::CIRCLES_LOW,     Groups::SLIGHTLY_DIVERGING_SLIM_MAPS},
      {               GoomEffect::DOTS0,                Groups::RED_STANDARD_MAPS},
      {               GoomEffect::DOTS1,               Groups::BLUE_STANDARD_MAPS},
      {               GoomEffect::DOTS2,              Groups::GREEN_STANDARD_MAPS},
      {               GoomEffect::DOTS3,             Groups::YELLOW_STANDARD_MAPS},
      {               GoomEffect::DOTS4,               Groups::HEAT_STANDARD_MAPS},
      {                 GoomEffect::IFS,                Groups::RED_STANDARD_MAPS},
      {               GoomEffect::IMAGE,                Groups::ALL_STANDARD_MAPS},
      {              GoomEffect::LINES1,                Groups::ALL_STANDARD_MAPS},
      {              GoomEffect::LINES2,               Groups::COLD_STANDARD_MAPS},
      {         GoomEffect::SHAPES_MAIN,                Groups::WES_ANDERSON_MAPS},
      {          GoomEffect::SHAPES_LOW,                Groups::RED_STANDARD_MAPS},
      {        GoomEffect::SHAPES_INNER, Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS},
      {GoomEffect::STARS_MAIN_FIREWORKS,               Groups::BLUE_STANDARD_MAPS},
      { GoomEffect::STARS_LOW_FIREWORKS,               Groups::BLUE_STANDARD_MAPS},
      {     GoomEffect::STARS_MAIN_RAIN,                Groups::RED_STANDARD_MAPS},
      {      GoomEffect::STARS_LOW_RAIN,             Groups::YELLOW_STANDARD_MAPS},
      { GoomEffect::STARS_MAIN_FOUNTAIN,              Groups::GREEN_STANDARD_MAPS},
      {  GoomEffect::STARS_LOW_FOUNTAIN,             Groups::PURPLE_STANDARD_MAPS},
      {           GoomEffect::TENTACLES,             Groups::YELLOW_STANDARD_MAPS},
      {           GoomEffect::TUBE_MAIN,              Groups::GREEN_STANDARD_MAPS},
      {            GoomEffect::TUBE_LOW,               Groups::HEAT_STANDARD_MAPS},
  };
}

} // namespace GOOM::CONTROL
