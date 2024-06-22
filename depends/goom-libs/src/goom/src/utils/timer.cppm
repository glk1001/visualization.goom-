module;

#include <algorithm>
#include <cstdint>
#include <functional>

export module Goom.Utils.Timer;

import Goom.Utils.GoomTime;
import Goom.Lib.GoomTypes;

export namespace GOOM::UTILS
{

class Timer
{
public:
  Timer(const GoomTime& goomTime, uint64_t timeLimit, bool setToFinished = false) noexcept;

  [[nodiscard]] auto GetTimeLeft() const noexcept -> uint64_t;
  auto SetTimeLimit(uint64_t timeLimit, bool setToFinished = false) noexcept -> void;
  auto ResetToZero() noexcept -> void;
  auto SetToFinished() noexcept -> void;

  [[nodiscard]] auto JustFinished() const noexcept -> bool;
  [[nodiscard]] auto Finished() const noexcept -> bool;
  [[nodiscard]] auto GetTimeElapsed() const noexcept -> uint64_t;

  [[nodiscard]] auto GetStartTme() const noexcept -> uint64_t;
  [[nodiscard]] auto GetTargetTime() const noexcept -> uint64_t;

private:
  const GoomTime* m_goomTime;
  uint64_t m_startTime;
  uint64_t m_timeLimit;
  uint64_t m_targetTime;
  bool m_finished;
};

class OnOffTimer
{
public:
  struct TimerCounts
  {
    uint32_t numOnCount;
    uint32_t numOnCountAfterFailedOff;
    uint32_t numOffCount;
    uint32_t numOffCountAfterFailedOn;
  };
  OnOffTimer(const GoomTime& goomTime, const TimerCounts& timerCounts) noexcept;

  auto Reset() noexcept -> void;
  auto SetTimerCounts(const TimerCounts& timerCounts) noexcept -> void;

  using Action = std::function<bool()>; // return true if action succeeded.
  struct OnAndOffActions
  {
    Action onAction;
    Action offAction;
  };
  auto SetActions(const OnAndOffActions& onAndOffActions) noexcept -> void;

  auto StartOnTimer() noexcept -> void;
  auto StartOffTimer() noexcept -> void;

  auto Stop() noexcept -> void;

  auto Update() noexcept -> void;
  auto TryToChangeState() noexcept -> void;

  enum class TimerState : UnderlyingEnumType
  {
    NO_TIMERS_ACTIVE,
    ON_TIMER_ACTIVE,
    OFF_TIMER_ACTIVE,
  };
  [[nodiscard]] auto GetTimerState() const noexcept -> TimerState;

  [[nodiscard]] auto GetOnTimer() const noexcept -> const Timer&;
  [[nodiscard]] auto GetOffTimer() const noexcept -> const Timer&;

private:
  TimerCounts m_timerCounts;
  Timer m_onTimer;
  Timer m_offTimer;
  Action m_onAction       = nullptr;
  Action m_offAction      = nullptr;
  TimerState m_timerState = TimerState::NO_TIMERS_ACTIVE;
  auto ChangeStateToOff() -> void;
  auto ChangeStateToOn() -> void;
};

} // namespace GOOM::UTILS

namespace GOOM::UTILS
{

inline auto Timer::SetToFinished() noexcept -> void
{
  m_finished = true;
}

inline auto Timer::GetTimeElapsed() const noexcept -> uint64_t
{
  return std::min(m_timeLimit, m_goomTime->GetElapsedTimeSince(m_startTime));
}

inline auto Timer::GetStartTme() const noexcept -> uint64_t
{
  return m_startTime;
}

inline auto Timer::GetTargetTime() const noexcept -> uint64_t
{
  return m_targetTime;
}

inline auto Timer::JustFinished() const noexcept -> bool
{
  return m_goomTime->GetCurrentTime() == m_targetTime;
}

inline auto Timer::Finished() const noexcept -> bool
{
  return m_finished or (m_goomTime->GetCurrentTime() >= m_targetTime);
}

inline auto OnOffTimer::GetTimerState() const noexcept -> TimerState
{
  return m_timerState;
}

inline auto OnOffTimer::GetOnTimer() const noexcept -> const Timer&
{
  return m_onTimer;
}

inline auto OnOffTimer::GetOffTimer() const noexcept -> const Timer&
{
  return m_offTimer;
}

} // namespace GOOM::UTILS
