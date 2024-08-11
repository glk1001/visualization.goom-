module;

#include <cstdint>
#include <vector>

module Goom.Control.GoomRandomStateHandler;

import Goom.Control.GoomDrawablesData;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.AssertUtils;

namespace GOOM::CONTROL
{

using UTILS::NUM;
using UTILS::MATH::GoomRand;

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

GoomRandomStateHandler::GoomRandomStateHandler(const GoomRand& goomRand)
  : m_goomRand{&goomRand},
    m_drawablesPool{GetFullDrawablesPool(*m_goomRand)},
    m_weightedChangeTypes{
        goomRand,
        {
            {.key=ChangeType::FRESH_STATE,         .weight=FRESH_STATE_WEIGHT},
            {.key=ChangeType::NON_REPEAT_STATE,    .weight=NON_REPEAT_STATE_WEIGHT},
            {.key=ChangeType::ADD_EXTRA_DRAWABLE,  .weight=ADD_EXTRA_DRAWABLE_WEIGHT},
            {.key=ChangeType::ADD_REMOVE_DRAWABLE, .weight=ADD_REMOVE_DRAWABLE_WEIGHT},
            {.key=ChangeType::REMOVE_DRAWABLE,     .weight=REMOVE_DRAWABLE_WEIGHT},
        }
    },
    m_weightedNumDrawables{
        goomRand,
        {
            {.key=NumDrawables::ONE,   .weight=ONE_WEIGHT},
            {.key=NumDrawables::TWO,   .weight=TWO_WEIGHT},
            {.key=NumDrawables::THREE, .weight=THREE_WEIGHT},
            {.key=NumDrawables::FOUR,  .weight=FOUR_WEIGHT},
            {.key=NumDrawables::FIVE,  .weight=FIVE_WEIGHT},
        }
    },
    m_currentDrawablesState{GetNewRandomState(GetNextNumDrawables())}
{
}

auto GoomRandomStateHandler::GetFullDrawablesPool(const GoomRand& goomRand)
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
  m_currentDrawablesState = GetNewRandomState(numRandomDrawables);
}

auto GoomRandomStateHandler::GetNewRandomState(const uint32_t numRandomDrawables)
    -> GoomDrawablesState
{
  const auto randomDrawables = GetNextRandomDrawables(numRandomDrawables);
  return GoomDrawablesState{randomDrawables,
                            GetRandInRangeBuffIntensities(*m_goomRand, randomDrawables)};
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
  const auto buffIntensity  = GetRandInRangeBuffIntensities(*m_goomRand, randomDrawable);

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
  const auto buffIntensity  = GetRandInRangeBuffIntensities(*m_goomRand, randomDrawable);

  auto randomDrawables = m_currentDrawablesState.GetDrawables();
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
  auto randomDrawables = GetRandomDrawablesFromPool(numDrawables);

  static constexpr auto MAX_TRIES = 10U;
  for (auto i = 0U; i < MAX_TRIES; ++i)
  {
    if ((numDrawables > 1) or
        m_goomRand->ProbabilityOf(GetProbCanBeSingleDrawable(randomDrawables[0])))
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

} // namespace GOOM::CONTROL
