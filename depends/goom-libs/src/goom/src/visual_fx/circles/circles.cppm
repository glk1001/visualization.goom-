module;

#include <algorithm>
#include <cstdint>
#include <vector>

export module Goom.VisualFx.CirclesFx.Circles;

import Goom.Color.RandomColorMaps;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.ParametricFunctions2d;
import Goom.VisualFx.CirclesFx.BitmapGetter;
import Goom.VisualFx.CirclesFx.Circle;
import Goom.VisualFx.CirclesFx.Helper;
import Goom.VisualFx.CirclesFx.DotPaths;
import Goom.VisualFx.FxHelper;
import Goom.PluginInfo;

export namespace GOOM::VISUAL_FX::CIRCLES
{

class Circles
{
public:
  Circles(FxHelper& fxHelper,
          const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
          uint32_t numCircles,
          const std::vector<UTILS::MATH::OscillatingFunction::Params>& pathParams,
          const std::vector<Circle::Params>& circleParams) noexcept;

  auto SetWeightedColorMaps(const COLOR::WeightedRandomColorMaps& weightedMaps,
                            const COLOR::WeightedRandomColorMaps& weightedLowMaps) noexcept -> void;
  [[nodiscard]] auto GetCurrentDirection() const noexcept -> DotPaths::Direction;
  auto ChangeDirection(DotPaths::Direction newDirection) noexcept -> void;
  auto SetPathParams(
      const std::vector<UTILS::MATH::OscillatingFunction::Params>& pathParams) noexcept -> void;
  auto SetGlobalBrightnessFactors(const std::vector<float>& brightnessFactors) noexcept -> void;

  auto Start() noexcept -> void;
  auto UpdateAndDraw() noexcept -> void;
  auto IncrementTs() noexcept -> void;

  [[nodiscard]] auto HasPositionTJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasPositionTJustHitEndBoundary() const noexcept -> bool;

private:
  const PluginInfo* m_goomInfo;
  const UTILS::MATH::GoomRand* m_goomRand;
  BitmapGetter m_bitmapGetter;

  uint32_t m_numCircles;
  std::vector<Circle> m_circles;
  [[nodiscard]] static auto GetCircles(
      FxHelper& fxHelper,
      const Helper& helper,
      const std::vector<UTILS::MATH::OscillatingFunction::Params>& pathParams,
      uint32_t numCircles,
      const std::vector<Circle::Params>& circleParams) noexcept -> std::vector<Circle>;
  auto UpdatePositionSpeed() noexcept -> void;
  auto UpdateAndDrawCircles() noexcept -> void;
};

} // namespace GOOM::VISUAL_FX::CIRCLES

namespace GOOM::VISUAL_FX::CIRCLES
{

inline auto Circles::GetCurrentDirection() const noexcept -> DotPaths::Direction
{
  return m_circles.front().GetCurrentDirection();
}

inline auto Circles::HasPositionTJustHitStartBoundary() const noexcept -> bool
{
  return m_circles.front().HasPositionTJustHitStartBoundary();
}

inline auto Circles::HasPositionTJustHitEndBoundary() const noexcept -> bool
{
  return m_circles.front().HasPositionTJustHitEndBoundary();
}

} // namespace GOOM::VISUAL_FX::CIRCLES

module :private;

namespace GOOM::VISUAL_FX::CIRCLES
{

using COLOR::WeightedRandomColorMaps;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::OscillatingFunction;

static constexpr auto LINE_DOT_DIAMETER = BitmapGetter::MIN_DOT_DIAMETER;
static constexpr auto MIN_DOT_DIAMETER  = BitmapGetter::MIN_DOT_DIAMETER + 4;
static constexpr auto MAX_DOT_DIAMETER  = BitmapGetter::MAX_DOT_DIAMETER;

Circles::Circles(FxHelper& fxHelper,
                 const SmallImageBitmaps& smallBitmaps,
                 const uint32_t numCircles,
                 const std::vector<OscillatingFunction::Params>& pathParams,
                 const std::vector<Circle::Params>& circleParams) noexcept
  : m_goomInfo{&fxHelper.GetGoomInfo()},
    m_goomRand{&fxHelper.GetGoomRand()},
    m_bitmapGetter{fxHelper.GetGoomRand(), smallBitmaps},
    m_numCircles{numCircles},
    m_circles{GetCircles(fxHelper,
                         {LINE_DOT_DIAMETER, MIN_DOT_DIAMETER, MAX_DOT_DIAMETER, &m_bitmapGetter},
                         pathParams,
                         m_numCircles,
                         circleParams)}
{
}

auto Circles::GetCircles(FxHelper& fxHelper,
                         const Helper& helper,
                         const std::vector<OscillatingFunction::Params>& pathParams,
                         const uint32_t numCircles,
                         const std::vector<Circle::Params>& circleParams) noexcept
    -> std::vector<Circle>
{
  auto circles = std::vector<Circle>{};
  circles.reserve(numCircles);

  for (auto i = 0U; i < numCircles; ++i)
  {
    circles.emplace_back(fxHelper, helper, circleParams[i], pathParams[i]);
  }

  return circles;
}

auto Circles::SetWeightedColorMaps(const WeightedRandomColorMaps& weightedMaps,
                                   const WeightedRandomColorMaps& weightedLowMaps) noexcept -> void
{
  std::ranges::for_each(m_circles,
                        [&weightedMaps, &weightedLowMaps](Circle& circle)
                        { circle.SetWeightedColorMaps(weightedMaps, weightedLowMaps); });

  m_bitmapGetter.ChangeCurrentBitmap();
}

auto Circles::ChangeDirection(const DotPaths::Direction newDirection) noexcept -> void
{
  std::ranges::for_each(m_circles,
                        [&newDirection](Circle& circle) { circle.ChangeDirection(newDirection); });
}

auto Circles::SetPathParams(const std::vector<OscillatingFunction::Params>& pathParams) noexcept
    -> void
{
  for (auto i = 0U; i < m_numCircles; ++i)
  {
    m_circles.at(i).SetPathParams(pathParams.at(i));
  }
}

auto Circles::SetGlobalBrightnessFactors(const std::vector<float>& brightnessFactors) noexcept
    -> void
{
  for (auto i = 0U; i < m_numCircles; ++i)
  {
    m_circles.at(i).SetGlobalBrightnessFactor(brightnessFactors.at(i));
  }
}

auto Circles::Start() noexcept -> void
{
  std::ranges::for_each(m_circles, [](Circle& circle) { circle.Start(); });
}

auto Circles::UpdateAndDraw() noexcept -> void
{
  UpdateAndDrawCircles();
  UpdatePositionSpeed();
}

auto Circles::UpdateAndDrawCircles() noexcept -> void
{
  std::ranges::for_each(m_circles, [](Circle& circle) { circle.UpdateAndDraw(); });
}

auto Circles::IncrementTs() noexcept -> void
{
  std::ranges::for_each(m_circles, [](Circle& circle) { circle.IncrementTs(); });
}

auto Circles::UpdatePositionSpeed() noexcept -> void
{
  if (static constexpr auto PROB_NO_SPEED_CHANGE = 0.7F;
      m_goomRand->ProbabilityOf<PROB_NO_SPEED_CHANGE>())
  {
    return;
  }

  static constexpr auto MIN_POSITION_STEPS = 100U;
  static constexpr auto MAX_POSITION_STEPS = 1000U;
  const auto newNumSteps                   = std::min(
      MIN_POSITION_STEPS + m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom(), MAX_POSITION_STEPS);

  std::ranges::for_each(
      m_circles, [&newNumSteps](Circle& circle) { circle.UpdatePositionSpeed(newNumSteps); });
}

} // namespace GOOM::VISUAL_FX::CIRCLES
