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
  SHAPES0,
  SHAPES1,
  SHAPES2,
  SHAPES3,
  SHAPES4,
  SHAPES5,
  SHAPES6,
  SHAPES7,
  SHAPES8,
  SHAPES9,
  STARS,
  STARS_LOW,
  TENTACLES,
  TUBE,
  TUBE_LOW,
  _num // unused and must be last
};

static inline constexpr auto EXPECTED_NUM_DOT_TYPES =
    (static_cast<uint32_t>(GoomEffect::DOTS4) - static_cast<uint32_t>(GoomEffect::DOTS0)) + 1;
static inline constexpr auto EXPECTED_NUM_SHAPES =
    (static_cast<uint32_t>(GoomEffect::SHAPES9) - static_cast<uint32_t>(GoomEffect::SHAPES0)) + 1;

class VisualFxColorMaps
{
public:
  explicit VisualFxColorMaps(const UTILS::MATH::IGoomRand& goomRand);

  auto SetNextColorMapSet() -> void;

  [[nodiscard]] auto GetColorMap(GoomEffect goomEffect) const
      -> std::shared_ptr<COLOR::RandomColorMaps>;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;

  using ColorMapFunc = std::function<std::shared_ptr<COLOR::RandomColorMaps>(
      const UTILS::MATH::IGoomRand& goomRand)>;
  using ColorMatchedSet = std::array<ColorMapFunc, UTILS::NUM<GoomEffect>>;
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
  const ColorMatchedSet* m_currentColorMatchedMap{&GetNextColorMatchedSet()};

  [[nodiscard]] static auto GetOneGroupColorMatchedSet(const ColorMapFunc& func) -> ColorMatchedSet;
  [[nodiscard]] auto GetTwoGroupsColorMatchedSet(const ColorMapFunc& func1,
                                                 const ColorMapFunc& func2) const
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
  m_currentColorMatchedMap = &GetNextColorMatchedSet();
}

inline auto VisualFxColorMaps::GetColorMap(const GoomEffect goomEffect) const
    -> std::shared_ptr<COLOR::RandomColorMaps>
{
  return (*m_currentColorMatchedMap)[static_cast<size_t>(goomEffect)](m_goomRand);
}

} // namespace CONTROL
} // namespace GOOM
