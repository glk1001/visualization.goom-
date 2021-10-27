#pragma once

#include "utils/enumutils.h"

#include <array>
#include <functional>
#include <memory>

namespace GOOM
{

namespace COLOR
{
class RandomColorMaps;
} // namespace COLOR

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
  VisualFxColorMaps() noexcept;

  void SetNextColorMapSet();

  [[nodiscard]] auto GetColorMap(GoomEffect goomEffect) const
      -> std::shared_ptr<COLOR::RandomColorMaps>;

private:
  using ColorMapFunc = std::function<std::shared_ptr<COLOR::RandomColorMaps>()>;
  using ColorMatchedSet = std::array<ColorMapFunc, UTILS::NUM<GoomEffect>>;
  const ColorMatchedSet* m_currentColorMatchedMap{};

  [[nodiscard]] static auto GetNextColorMatchedSet() -> const ColorMatchedSet&;

  static constexpr size_t NUM_COLOR_MATCHED_SETS = 18;
  using ColorMatchedSets = std::array<ColorMatchedSet, NUM_COLOR_MATCHED_SETS>;
  [[nodiscard]] static auto GetColorMatchedSets() -> const ColorMatchedSets&;

  [[nodiscard]] static auto GetConstColorMatchedSet(const ColorMapFunc& func) -> ColorMatchedSet;

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
  return (*m_currentColorMatchedMap)[static_cast<size_t>(goomEffect)]();
}

} // namespace CONTROL
} // namespace GOOM
