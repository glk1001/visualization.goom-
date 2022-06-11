#pragma once

#include "utils/enumutils.h"
#include "utils/math/goom_rand_base.h"

#include <array>
#include <functional>
#include <memory>

namespace GOOM
{

namespace COLOR
{
class RandomColorMaps;
}

namespace CONTROL
{

enum class GoomEffect
{
  CIRCLES = 0,
  CIRCLES_LOW,
  DOTS0,
  DOTS1,
  DOTS2,
  DOTS3,
  DOTS4,
  IFS,
  IMAGE,
  LINES1,
  LINES2,
  SHAPES0_MAIN,
  SHAPES0_LOW,
  SHAPES0_INNER,
  /**
  SHAPES1_MAIN,
  SHAPES1_LOW,
  SHAPES1_INNER,
  SHAPES2_MAIN,
  SHAPES2_LOW,
  SHAPES2_INNER,
   **/
  STARS_MAIN_FIREWORKS,
  STARS_LOW_FIREWORKS,
  STARS_MAIN_RAIN,
  STARS_LOW_RAIN,
  STARS_MAIN_FOUNTAIN,
  STARS_LOW_FOUNTAIN,
  TENTACLES,
  TUBE,
  TUBE_LOW,
  _num // unused and must be last
};

static inline constexpr auto EXPECTED_NUM_DOT_TYPES =
    1 + (static_cast<uint32_t>(GoomEffect::DOTS4) - static_cast<uint32_t>(GoomEffect::DOTS0));
static inline constexpr auto EXPECTED_NUM_SHAPES =
    1 + ((static_cast<uint32_t>(GoomEffect::SHAPES0_MAIN) -
          static_cast<uint32_t>(GoomEffect::SHAPES0_MAIN)) /
         3);
static inline constexpr auto EXPECTED_NUM_STAR_MODES =
    1 + ((static_cast<uint32_t>(GoomEffect::STARS_LOW_FOUNTAIN) -
          static_cast<uint32_t>(GoomEffect::STARS_MAIN_FIREWORKS)) /
         2);

class VisualFxColorMaps
{
public:
  explicit VisualFxColorMaps(const UTILS::MATH::IGoomRand& goomRand);

  auto SetNextColorMapSet() -> void;

  [[nodiscard]] auto GetColorMaps(GoomEffect goomEffect) const
      -> std::shared_ptr<COLOR::RandomColorMaps>;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;

  using ColorMapsFunc = std::function<std::shared_ptr<COLOR::RandomColorMaps>(
      const UTILS::MATH::IGoomRand& goomRand)>;
  using ColorMatchedSet = std::array<ColorMapsFunc, UTILS::NUM<GoomEffect>>;
  enum class ColorMatchedSets
  {
    RED_GREEN_STANDARD_MAPS,
    RED_BLUE_STANDARD_MAPS,
    YELLOW_BLUE_STANDARD_MAPS,
    YELLOW_PURPLE_STANDARD_MAPS,
    ORANGE_GREEN_STANDARD_MAPS,
    ORANGE_PURPLE_STANDARD_MAPS,
    ALL_ONLY_STANDARD_MAPS,
    HEAT_ONLY_STANDARD_MAPS,
    COLD_ONLY_STANDARD_MAPS,
    DIVERGING_ONLY_STANDARD_MAPS,
    DIVERGING_BLACK_ONLY_STANDARD_MAPS,
    WES_ANDERSON_ONLY_MAPS,
    PASTEL_ONLY_MAPS,
    COLOR_MATCHED_SET1,
    COLOR_MATCHED_SET2,
    COLOR_MATCHED_SET3,
    COLOR_MATCHED_SET4,
    COLOR_MATCHED_SET5,
    COLOR_MATCHED_SET6,
    COLOR_MATCHED_SET7,
    COLOR_MATCHED_SET8,
    _num // unused and must be last
  };
  using ColorMatchedSetsArray = std::array<ColorMatchedSet, UTILS::NUM<ColorMatchedSets>>;
  [[nodiscard]] auto GetColorMatchedSetsArray() const -> ColorMatchedSetsArray;
  const ColorMatchedSetsArray m_colorMatchedSets{GetColorMatchedSetsArray()};
  const UTILS::MATH::Weights<ColorMatchedSets> m_colorMatchedSetsWeights;
  [[nodiscard]] auto GetNextColorMatchedSet() const -> const ColorMatchedSet&;
  const ColorMatchedSet* m_currentColorMatchedSet{&GetNextColorMatchedSet()};

  [[nodiscard]] static auto GetOneGroupColorMatchedSet(const ColorMapsFunc& func)
      -> ColorMatchedSet;
  [[nodiscard]] auto GetTwoGroupsColorMatchedSet(const ColorMapsFunc& func1,
                                                 const ColorMapsFunc& func2) const
      -> ColorMatchedSet;

  static auto GetPrimaryColorDots(ColorMatchedSet& matchedSet) -> void;

  [[nodiscard]] static auto GetColorMatchedSet1() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet2() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet3() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet4() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet5() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet6() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet7() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet8() -> ColorMatchedSet;
};

inline auto VisualFxColorMaps::SetNextColorMapSet() -> void
{
  m_currentColorMatchedSet = &GetNextColorMatchedSet();
}

inline auto VisualFxColorMaps::GetColorMaps(const GoomEffect goomEffect) const
    -> std::shared_ptr<COLOR::RandomColorMaps>
{
  return (*m_currentColorMatchedSet)[static_cast<size_t>(goomEffect)](m_goomRand);
}

} // namespace CONTROL
} // namespace GOOM
