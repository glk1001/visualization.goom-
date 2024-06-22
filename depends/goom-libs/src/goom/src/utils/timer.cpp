module;

#include <cstdint>

module Goom.Utils.Timer;

import Goom.Utils.GoomTime;
import Goom.Lib.AssertUtils;

namespace GOOM::UTILS
{

Timer::Timer(const GoomTime& goomTime, const uint64_t timeLimit, const bool setToFinished) noexcept
  : m_goomTime{&goomTime},
    m_startTime{m_goomTime->GetCurrentTime()},
    m_timeLimit{timeLimit},
    m_targetTime{m_startTime + timeLimit},
    m_finished{setToFinished}
{
}

auto Timer::ResetToZero() noexcept -> void
{
  m_startTime  = m_goomTime->GetCurrentTime();
  m_targetTime = m_startTime + m_timeLimit;
  m_finished   = false;
}

auto Timer::GetTimeLeft() const noexcept -> uint64_t
{
  if (m_finished or (m_goomTime->GetCurrentTime() >= m_targetTime))
  {
    return 0U;
  }
  return m_targetTime - m_goomTime->GetCurrentTime();
}

auto Timer::SetTimeLimit(const uint64_t timeLimit, const bool setToFinished) noexcept -> void
{
  m_timeLimit  = timeLimit;
  m_targetTime = m_startTime + timeLimit;
  m_finished   = setToFinished;
}

OnOffTimer::OnOffTimer(const GoomTime& goomTime, const TimerCounts& timerCounts) noexcept
  : m_timerCounts{timerCounts},
    m_onTimer{goomTime, m_timerCounts.numOnCount, true},
    m_offTimer{goomTime, m_timerCounts.numOffCount, true}
{
}

auto OnOffTimer::Reset() noexcept -> void
{
  m_onTimer.ResetToZero();
  m_offTimer.ResetToZero();
  m_onTimer.SetToFinished();
  m_offTimer.SetToFinished();
  m_timerState = TimerState::NO_TIMERS_ACTIVE;
}

auto OnOffTimer::SetTimerCounts(const TimerCounts& timerCounts) noexcept -> void
{
  m_timerCounts = timerCounts;
  m_onTimer.SetTimeLimit(m_timerCounts.numOnCount, true);
  m_offTimer.SetTimeLimit(m_timerCounts.numOffCount, true);
}

auto OnOffTimer::SetActions(const OnAndOffActions& onAndOffActions) noexcept -> void
{
  m_onAction  = onAndOffActions.onAction;
  m_offAction = onAndOffActions.offAction;
}

auto OnOffTimer::StartOnTimer() noexcept -> void
{
  Expects(m_timerState == TimerState::NO_TIMERS_ACTIVE);
  Expects(m_onTimer.Finished());
  Expects(m_offTimer.Finished());
  Expects(static_cast<bool>(m_onAction));
  Expects(static_cast<bool>(m_offAction));

  m_timerState = TimerState::ON_TIMER_ACTIVE;
  m_onTimer.ResetToZero();
  m_onAction();

  Ensures(not m_onTimer.Finished());
  Ensures(m_offTimer.Finished());
}

auto OnOffTimer::StartOffTimer() noexcept -> void
{
  Expects(m_timerState == TimerState::NO_TIMERS_ACTIVE);
  Expects(m_onTimer.Finished());
  Expects(m_offTimer.Finished());
  Expects(static_cast<bool>(m_onAction));
  Expects(static_cast<bool>(m_offAction));

  m_timerState = TimerState::OFF_TIMER_ACTIVE;
  m_offTimer.ResetToZero();
  m_offAction();

  Ensures(not m_offTimer.Finished());
  Ensures(m_onTimer.Finished());
}

auto OnOffTimer::Stop() noexcept -> void
{
  if (TimerState::ON_TIMER_ACTIVE == m_timerState)
  {
    m_offAction();
  }
  else if (TimerState::OFF_TIMER_ACTIVE == m_timerState)
  {
    m_onAction();
  }
  m_timerState = TimerState::NO_TIMERS_ACTIVE;
  m_onTimer.SetToFinished();
  m_offTimer.SetToFinished();

  Ensures(m_onTimer.Finished());
  Ensures(m_offTimer.Finished());
  Ensures(m_timerState == TimerState::NO_TIMERS_ACTIVE);
}

auto OnOffTimer::TryToChangeState() noexcept -> void
{
  if (TimerState::ON_TIMER_ACTIVE == m_timerState)
  {
    ChangeStateToOff();
  }
  else if (TimerState::OFF_TIMER_ACTIVE == m_timerState)
  {
    ChangeStateToOn();
  }
}

auto OnOffTimer::Update() noexcept -> void
{
  if (TimerState::ON_TIMER_ACTIVE == m_timerState)
  {
    Expects(m_offTimer.Finished());
    if (m_onTimer.Finished())
    {
      ChangeStateToOff();
    }
  }
  else if (TimerState::OFF_TIMER_ACTIVE == m_timerState)
  {
    Expects(m_onTimer.Finished());
    if (m_offTimer.Finished())
    {
      ChangeStateToOn();
    }
  }
}

auto OnOffTimer::ChangeStateToOff() -> void
{
  Expects(m_offTimer.Finished());

  if (not m_offAction())
  {
    m_onTimer.ResetToZero();
    m_onTimer.SetTimeLimit(m_timerCounts.numOnCountAfterFailedOff);
    Ensures(not m_onTimer.Finished());
    Ensures(m_offTimer.Finished());
    return;
  }

  m_onTimer.SetToFinished();
  m_offTimer.ResetToZero();
  m_offTimer.SetTimeLimit(m_timerCounts.numOffCount);
  m_timerState = TimerState::OFF_TIMER_ACTIVE;

  Ensures(not m_offTimer.Finished());
  Ensures(m_onTimer.Finished());
}

auto OnOffTimer::ChangeStateToOn() -> void
{
  Expects(m_onTimer.Finished());

  if (not m_onAction())
  {
    m_offTimer.ResetToZero();
    m_offTimer.SetTimeLimit(m_timerCounts.numOffCountAfterFailedOn);
    Ensures(not m_offTimer.Finished());
    Ensures(m_onTimer.Finished());
    return;
  }

  m_offTimer.SetToFinished();
  m_onTimer.ResetToZero();
  m_onTimer.SetTimeLimit(m_timerCounts.numOnCount);
  m_timerState = TimerState::ON_TIMER_ACTIVE;

  Ensures(not m_onTimer.Finished());
  Ensures(m_offTimer.Finished());
}

} // namespace GOOM::UTILS
