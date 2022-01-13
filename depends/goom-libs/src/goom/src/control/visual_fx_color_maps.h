#pragma once

#include "utils/enumutils.h"
#include "utils/goom_rand_base.h"

#include <array>
#include <functional>
#include <memory>

namespace GOOM
{

namespace COLOR
{
class RandomColorMaps;
}

namespace UTILS
{
class IGoomRand;
}

namespace CONTROL
{

enum class GoomEffect
{
  DOTS0 = 0,
  DOTS1,
  DOTS2,
  DOTS3,
  DOTS4,
  IFS,
  IMAGE,
  LINES1,
  LINES2,
  STARS,
  STARS_LOW,
  TENTACLES,
  TUBE,
  TUBE_LOW,
  _NUM // unused and must be last
};

class VisualFxColorMaps
{
public:
  explicit VisualFxColorMaps(const UTILS::IGoomRand& goomRand) noexcept;

  void SetNextColorMapSet();

  [[nodiscard]] auto GetColorMap(GoomEffect goomEffect) const
      -> std::shared_ptr<COLOR::RandomColorMaps>;

private:
  const UTILS::IGoomRand& m_goomRand;

  using ColorMapFunc =
      std::function<std::shared_ptr<COLOR::RandomColorMaps>(const UTILS::IGoomRand& goomRand)>;
  using ColorMatchedSet = std::array<ColorMapFunc, UTILS::NUM<GoomEffect>>;
  enum class ColorMatchedSets
  {
    RED_GREEN_STANDARD_MAPS,
    RED_BLUE_STANDARD_MAPS,
    YELLOW_BLUE_STANDARD_MAPS,
    YELLOW_PURPLE_STANDARD_MAPS,
    ORANGE_GREEN_STANDARD_MAPS,
    ORANGE_PURPLE_STANDARD_MAPS,
    CONST_ALL_STANDARD_MAPS,
    CONST_HEAT_STANDARD_MAPS,
    CONST_COLD_STANDARD_MAPS,
    CONST_DIVERGING_BLACK_STANDARD_MAPS,
    CONST_WES_ANDERSON_MAPS,
    COLOR_MATCHED_SET1,
    COLOR_MATCHED_SET2,
    COLOR_MATCHED_SET3,
    COLOR_MATCHED_SET4,
    COLOR_MATCHED_SET5,
    COLOR_MATCHED_SET6,
    COLOR_MATCHED_SET7,
    COLOR_MATCHED_SET8,
    _NUM // unused and must be last
  };
  using ColorMatchedSetArray = std::array<ColorMatchedSet, UTILS::NUM<ColorMatchedSets>>;
  const ColorMatchedSetArray m_colorMatchedSets;
  [[nodiscard]] auto GetColorMatchedSetArray() const -> ColorMatchedSetArray;
  const UTILS::Weights<ColorMatchedSets> m_colorMatchedSetWeights;
  const ColorMatchedSet* m_currentColorMatchedMap;

  [[nodiscard]] auto GetNextColorMatchedSet() const -> const ColorMatchedSet&;

  [[nodiscard]] static auto GetConstColorMatchedSet(const ColorMapFunc& func) -> ColorMatchedSet;
  [[nodiscard]] auto GetColorPairColorMatchedSet(const ColorMapFunc& func1,
                                                 const ColorMapFunc& func2) const
      -> ColorMatchedSet;

  [[nodiscard]] static auto GetColorMatchedSet1() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet2() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet3() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet4() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet5() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet6() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet7() -> ColorMatchedSet;
  [[nodiscard]] static auto GetColorMatchedSet8() -> ColorMatchedSet;
};

inline void VisualFxColorMaps::SetNextColorMapSet()
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
