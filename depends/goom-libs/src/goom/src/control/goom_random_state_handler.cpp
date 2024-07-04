module;

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <vector>

module Goom.Control.GoomRandomStateHandler;

import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.AssertUtils;

namespace GOOM::CONTROL
{

using UTILS::EnumMap;
using UTILS::NUM;
using UTILS::MATH::NumberRange;

static constexpr auto FRESH_STATE_WEIGHT         = 7.0F;
static constexpr auto NON_REPEAT_STATE_WEIGHT    = 7.0F;
static constexpr auto ADD_EXTRA_DRAWABLE_WEIGHT  = 4.0F;
static constexpr auto ADD_REMOVE_DRAWABLE_WEIGHT = 1.0F;
static constexpr auto REMOVE_DRAWABLE_WEIGHT     = 1.0F;

static constexpr auto ONE_WEIGHT   = 1.0F;
static constexpr auto TWO_WEIGHT   = 3.0F;
static constexpr auto THREE_WEIGHT = 3.0F;
static constexpr auto FOUR_WEIGHT  = 2.0F;
static constexpr auto FIVE_WEIGHT  = 1.0F;

GoomRandomStateHandler::GoomRandomStateHandler(const IGoomRand& goomRand)
  : m_goomRand{&goomRand},
    m_weightedChangeTypes{
        goomRand,
        {
            {ChangeType::FRESH_STATE,         FRESH_STATE_WEIGHT},
            {ChangeType::NON_REPEAT_STATE,    NON_REPEAT_STATE_WEIGHT},
            {ChangeType::ADD_EXTRA_DRAWABLE,  ADD_EXTRA_DRAWABLE_WEIGHT},
            {ChangeType::ADD_REMOVE_DRAWABLE, ADD_REMOVE_DRAWABLE_WEIGHT},
            {ChangeType::REMOVE_DRAWABLE,     REMOVE_DRAWABLE_WEIGHT},
        }
    },
    m_weightedNumDrawables{
        goomRand,
        {
            {NumDrawables::ONE,   ONE_WEIGHT},
            {NumDrawables::TWO,   TWO_WEIGHT},
            {NumDrawables::THREE, THREE_WEIGHT},
            {NumDrawables::FOUR,  FOUR_WEIGHT},
            {NumDrawables::FIVE,  FIVE_WEIGHT},
        }
    }
{
  ChangeToFreshState();
}

auto GoomRandomStateHandler::GetFullDrawablesPool(const IGoomRand& goomRand)
    -> std::vector<GoomDrawables>
{
  auto fullPool = std::vector<GoomDrawables>(NUM<GoomDrawables>);

  for (auto i = 0U; i < NUM<GoomDrawables>; ++i)
  {
    fullPool[i] = static_cast<GoomDrawables>(i);
  }

  goomRand.Shuffle(fullPool);

  return fullPool;
}

auto GoomRandomStateHandler::ChangeToNextState() -> void
{
  switch (m_weightedChangeTypes.GetRandomWeighted())
  {
    case ChangeType::FRESH_STATE:
      ChangeToFreshState();
      break;
    case ChangeType::NON_REPEAT_STATE:
      ChangeToNonRepeatState();
      break;
    case ChangeType::ADD_EXTRA_DRAWABLE:
      AddExtraDrawableToCurrentState();
      break;
    case ChangeType::ADD_REMOVE_DRAWABLE:
      AddRemoveDrawableToCurrentState();
      break;
    case ChangeType::REMOVE_DRAWABLE:
      RemoveDrawableFromCurrentState();
      break;
  }
}

auto GoomRandomStateHandler::ChangeToFreshState() -> void
{
  m_drawablesPool = GetFullDrawablesPool(*m_goomRand);

  ChangeToNewState(GetNextNumDrawables());
}

auto GoomRandomStateHandler::ChangeToNonRepeatState() -> void
{
  if (m_drawablesPool.empty())
  {
    ChangeToFreshState();
    return;
  }

  const auto numRandomDrawables =
      std::min(GetNextNumDrawables(), static_cast<uint32_t>(m_drawablesPool.size()));

  ChangeToNewState(numRandomDrawables);
}

auto GoomRandomStateHandler::ChangeToNewState(const uint32_t numRandomDrawables) -> void
{
  const auto randomDrawables = GetNextRandomDrawables(numRandomDrawables);
  const auto buffIntensities = GetBuffIntensities(randomDrawables);

  m_currentDrawablesState = GoomDrawablesState{randomDrawables, buffIntensities};
}

auto GoomRandomStateHandler::AddExtraDrawableToCurrentState() -> void
{
  if (m_drawablesPool.empty() or
      (m_currentDrawablesState.GetDrawables().size() >= MAX_NUM_DRAWABLES))
  {
    ChangeToFreshState();
    return;
  }

  Expects(not m_drawablesPool.empty());

  const auto randomDrawable = GetRandomDrawablesFromPool(1);
  const auto buffIntensity  = GetBuffIntensities(randomDrawable);

  auto randomDrawables = m_currentDrawablesState.GetDrawables();
  randomDrawables.emplace_back(randomDrawable[0]);

  auto buffIntensities = m_currentDrawablesState.GetDrawablesBuffIntensities();
  buffIntensities.emplace_back(buffIntensity[0]);

  m_currentDrawablesState = GoomDrawablesState{randomDrawables, buffIntensities};
}

auto GoomRandomStateHandler::AddRemoveDrawableToCurrentState() -> void
{
  if (m_drawablesPool.empty())
  {
    ChangeToFreshState();
    return;
  }

  const auto randomDrawable = GetRandomDrawablesFromPool(1);
  const auto buffIntensity  = GetBuffIntensities(randomDrawable);

  auto randomDrawables   = m_currentDrawablesState.GetDrawables();
  m_drawablesPool.insert(begin(m_drawablesPool), randomDrawables.back());
  randomDrawables.back() = randomDrawable[0];

  auto buffIntensities   = m_currentDrawablesState.GetDrawablesBuffIntensities();
  buffIntensities.back() = buffIntensity[0];

  m_currentDrawablesState = GoomDrawablesState{randomDrawables, buffIntensities};
}

auto GoomRandomStateHandler::RemoveDrawableFromCurrentState() -> void
{
  auto randomDrawables = m_currentDrawablesState.GetDrawables();
  if (randomDrawables.size() == 1)
  {
    ChangeToFreshState();
    return;
  }

  m_drawablesPool.insert(begin(m_drawablesPool), randomDrawables.back());
  randomDrawables.pop_back();

  auto buffIntensities = m_currentDrawablesState.GetDrawablesBuffIntensities();
  buffIntensities.pop_back();

  m_currentDrawablesState = GoomDrawablesState{randomDrawables, buffIntensities};
}

auto GoomRandomStateHandler::GetNextNumDrawables() const -> uint32_t
{
  return static_cast<uint32_t>(m_weightedNumDrawables.GetRandomWeighted()) + 1;
}

auto GoomRandomStateHandler::GetNextRandomDrawables(const uint32_t numDrawables)
    -> std::vector<GoomDrawables>
{
  static constexpr auto PROB_SINGLE_DRAWABLE = EnumMap<GoomDrawables, float>{{{
      {GoomDrawables::CIRCLES, 1.0F},
      {GoomDrawables::DOTS, 1.0F},
      {GoomDrawables::IFS, 1.0F},
      {GoomDrawables::L_SYSTEM, 1.0F},
      {GoomDrawables::LINES, 1.0F},
      {GoomDrawables::IMAGE, 0.0F},
      {GoomDrawables::PARTICLES, 1.0F},
      {GoomDrawables::RAINDROPS, 1.0F},
      {GoomDrawables::SHAPES, 1.0F},
      {GoomDrawables::STARS, 1.0F},
      {GoomDrawables::TENTACLES, 1.0F},
      {GoomDrawables::TUBES, 1.0F},
  }}};

  auto randomDrawables = GetRandomDrawablesFromPool(numDrawables);

  static constexpr auto MAX_TRIES = 10U;
  for (auto i = 0U; i < MAX_TRIES; ++i)
  {
    if ((numDrawables > 1) or m_goomRand->ProbabilityOf(PROB_SINGLE_DRAWABLE[randomDrawables[0]]))
    {
      break;
    }
    if (m_drawablesPool.empty())
    {
      break;
    }
    m_drawablesPool.insert(begin(m_drawablesPool), randomDrawables[0]);
    randomDrawables = GetRandomDrawablesFromPool(numDrawables);
  }

  return randomDrawables;
}

auto GoomRandomStateHandler::GetRandomDrawablesFromPool(const uint32_t numDrawables)
    -> std::vector<GoomDrawables>
{
  Expects(numDrawables > 0);
  Expects(numDrawables <= m_drawablesPool.size());

  auto randomDrawables =
      std::vector<GoomDrawables>{cend(m_drawablesPool) - numDrawables, cend(m_drawablesPool)};

  m_drawablesPool.erase(cend(m_drawablesPool) - numDrawables, cend(m_drawablesPool));

  return randomDrawables;
}

auto GoomRandomStateHandler::GetBuffIntensities(
    const std::vector<GoomDrawables>& drawables) const noexcept -> std::vector<float>
{
  static constexpr auto BUFF_INTENSITY_RANGES = EnumMap<GoomDrawables, NumberRange<float>>{{{
      {GoomDrawables::CIRCLES, {0.50F, 0.80F}},
      {GoomDrawables::DOTS, {0.40F, 0.70F}},
      {GoomDrawables::IFS, {0.40F, 0.70F}},
      {GoomDrawables::L_SYSTEM, {0.70F, 0.80F}},
      {GoomDrawables::LINES, {0.50F, 0.70F}},
      {GoomDrawables::IMAGE, {0.05F, 0.30F}},
      {GoomDrawables::PARTICLES, {0.50F, 0.80F}},
      {GoomDrawables::RAINDROPS, {0.60F, 0.80F}},
      {GoomDrawables::SHAPES, {0.50F, 0.80F}},
      {GoomDrawables::STARS, {0.50F, 0.60F}},
      {GoomDrawables::TENTACLES, {0.30F, 0.50F}},
      {GoomDrawables::TUBES, {0.70F, 0.80F}},
  }}};

  return std::ranges::to<std::vector<float>>(
      drawables | std::views::transform(
                      [this](const auto drawable)
                      { return m_goomRand->GetRandInRange(BUFF_INTENSITY_RANGES[drawable]); }));
}

} // namespace GOOM::CONTROL
