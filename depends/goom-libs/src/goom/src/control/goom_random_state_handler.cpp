module;

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

module Goom.Control.GoomRandomStateHandler;

import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.AssertUtils;

namespace GOOM::CONTROL
{

using UTILS::EnumMap;
using UTILS::NUM;
using UTILS::MATH::Weights;

static constexpr auto PROB_SINGLE_DRAWABLE          = EnumMap<GoomDrawables, float>{{{
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
static constexpr auto DRAWABLE_NAMES                = EnumMap<GoomDrawables, std::string_view>{{{
    {GoomDrawables::CIRCLES, "CIRCLS"},
    {GoomDrawables::DOTS, "DOTS"},
    {GoomDrawables::IFS, "IFS"},
    {GoomDrawables::L_SYSTEM, "LSYS"},
    {GoomDrawables::LINES, "LNS"},
    {GoomDrawables::IMAGE, "IMG"},
    {GoomDrawables::PARTICLES, "PART"},
    {GoomDrawables::RAINDROPS, "DROPS"},
    {GoomDrawables::SHAPES, "SHPS"},
    {GoomDrawables::STARS, "STARS"},
    {GoomDrawables::TENTACLES, "TENTCL"},
    {GoomDrawables::TUBES, "TUBES"},
}}};
static constexpr auto DEFAULT_BUFF_INTENSITY_RANGES = EnumMap<GoomDrawables, BuffIntensityRange>{{{
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

static constexpr auto FRESH_STATE_WEIGHT         = 6.0F;
static constexpr auto ADD_EXTRA_DRAWABLE_WEIGHT  = 2.0F;
static constexpr auto ADD_REMOVE_DRAWABLE_WEIGHT = 1.0F;
static constexpr auto REMOVE_DRAWABLE_WEIGHT     = 1.0F;
static constexpr auto ONE_WEIGHT                 = 1.0F;
static constexpr auto TWO_WEIGHT                 = 3.0F;
static constexpr auto THREE_WEIGHT               = 3.0F;
static constexpr auto FOUR_WEIGHT                = 2.0F;
static constexpr auto FIVE_WEIGHT                = 1.0F;

GoomRandomStateHandler::GoomRandomStateHandler(const IGoomRand& goomRand)
  : m_goomRand{&goomRand},
    m_weightedChangeTypes{
        goomRand,
        {
            {ChangeType::FRESH_STATE,         FRESH_STATE_WEIGHT},
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

  const auto numRandomDrawables  = GetNextNumDrawables();
  const auto randomDrawables     = GetNextRandomDrawables(numRandomDrawables);
  const auto stateName           = GetDrawablesStateName(randomDrawables);
  const auto buffIntensityRanges = GetBuffIntensityRanges(randomDrawables);

  m_currentDrawablesState = GoomDrawablesState{stateName, randomDrawables, buffIntensityRanges};
}

auto GoomRandomStateHandler::AddExtraDrawableToCurrentState() -> void
{
  if (m_currentDrawablesState.GetDrawables().size() >= MAX_NUM_DRAWABLES)
  {
    ChangeToFreshState();
    return;
  }

  Expects(not m_drawablesPool.empty());

  const auto randomDrawable     = GetRandomDrawablesFromPool(1);
  const auto drawableName       = GetDrawablesStateName(randomDrawable);
  const auto buffIntensityRange = GetBuffIntensityRanges(randomDrawable);

  const auto stateName = m_currentDrawablesState.GetName() + "_" + drawableName;

  auto randomDrawables = m_currentDrawablesState.GetDrawables();
  randomDrawables.emplace_back(randomDrawable[0]);

  auto buffIntensityRanges = m_currentDrawablesState.GetBuffIntensityRanges();
  buffIntensityRanges.emplace_back(buffIntensityRange[0]);

  m_currentDrawablesState = GoomDrawablesState{stateName, randomDrawables, buffIntensityRanges};
}

auto GoomRandomStateHandler::AddRemoveDrawableToCurrentState() -> void
{
  ChangeToFreshState();
}

auto GoomRandomStateHandler::RemoveDrawableFromCurrentState() -> void
{
  ChangeToFreshState();
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

auto GoomRandomStateHandler::GetDrawablesStateName(
    const std::vector<GoomDrawables>& drawables) const noexcept -> std::string
{
  auto stateName = std::string{};

  for (auto i = 0U; i < drawables.size(); ++i)
  {
    stateName += std::string{DRAWABLE_NAMES[drawables[i]]} + "_";
  }
  stateName.pop_back();

  return stateName;
}

auto GoomRandomStateHandler::GetBuffIntensityRanges(
    const std::vector<GoomDrawables>& drawables) const noexcept -> std::vector<BuffIntensityRange>
{
  auto buffIntensityRanges = std::vector<BuffIntensityRange>(drawables.size());

  for (auto i = 0U; i < drawables.size(); ++i)
  {
    buffIntensityRanges[i] = DEFAULT_BUFF_INTENSITY_RANGES[drawables[i]];
  }

  return buffIntensityRanges;
}

} // namespace GOOM::CONTROL
