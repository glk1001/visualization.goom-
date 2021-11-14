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
} // namespace COLOR

namespace UTILS
{
class IGoomRand;
} // namespace UTILS

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
  explicit VisualFxColorMaps(UTILS::IGoomRand& goomRand) noexcept;

  void SetNextColorMapSet();

  [[nodiscard]] auto GetColorMap(GoomEffect goomEffect) const
      -> std::shared_ptr<COLOR::RandomColorMaps>;

private:
  UTILS::IGoomRand& m_goomRand;

  using ColorMapFunc =
      std::function<std::shared_ptr<COLOR::RandomColorMaps>(UTILS::IGoomRand& goomRand)>;
  using ColorMatchedSet = std::array<ColorMapFunc, UTILS::NUM<GoomEffect>>;
  const UTILS::Weights<ColorMatchedSet> m_colorMatchedSets;
  const ColorMatchedSet* m_currentColorMatchedMap;

  [[nodiscard]] auto GetNextColorMatchedSet() -> const ColorMatchedSet&;

  [[nodiscard]] static auto GetConstColorMatchedSet(const ColorMapFunc& func) -> ColorMatchedSet;
  [[nodiscard]] auto GetColorPairColorMatchedSet(const ColorMapFunc& func1,
                                                 const ColorMapFunc& func2) -> ColorMatchedSet;

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

inline auto VisualFxColorMaps::GetColorMap(GoomEffect goomEffect) const
    -> std::shared_ptr<COLOR::RandomColorMaps>
{
  return (*m_currentColorMatchedMap)[static_cast<size_t>(goomEffect)](m_goomRand);
}

} // namespace CONTROL
} // namespace GOOM
