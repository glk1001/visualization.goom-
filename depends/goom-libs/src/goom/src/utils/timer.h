#pragma once

#include <cstdint>

namespace GOOM::UTILS
{

class Timer
{
public:
  Timer() noexcept = delete;
  explicit Timer(uint32_t numCount, bool setToFinished = false) noexcept;

  [[nodiscard]] auto GetTimeLimit() const -> uint32_t;
  void SetTimeLimit(uint32_t val);

  void ResetToZero();
  void SetToFinished();
  void Increment();
  [[nodiscard]] auto JustFinished() const -> bool;
  [[nodiscard]] auto Finished() const -> bool;
  [[nodiscard]] auto GetCurrentCount() const -> uint64_t;

private:
  uint32_t m_numCount;
  uint64_t m_count;
};

inline Timer::Timer(const uint32_t numCount, const bool setToFinished) noexcept
  : m_numCount{numCount}, m_count(setToFinished ? m_numCount : 0)
{
}

inline auto Timer::GetCurrentCount() const -> uint64_t
{
  return m_count;
}

inline auto Timer::GetTimeLimit() const -> uint32_t
{
  return m_numCount;
}

inline void Timer::SetTimeLimit(const uint32_t val)
{
  m_numCount = val;
  m_count = 0;
}

inline auto Timer::JustFinished() const -> bool
{
  return m_count == m_numCount;
}

inline auto Timer::Finished() const -> bool
{
  return m_count >= m_numCount;
}

inline void Timer::ResetToZero()
{
  m_count = 0;
}

inline void Timer::SetToFinished()
{
  m_count = m_numCount;
}

inline void Timer::Increment()
{
  ++m_count;
}

} // namespace GOOM::UTILS
