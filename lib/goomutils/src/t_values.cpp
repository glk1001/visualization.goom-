#include "t_values.h"

#include "goomutils/logging_control.h"
//#undef NO_LOGGING
#include "goomutils/logging.h"
#include "mathutils.h"

#undef NDEBUG
#include <cassert>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace UTILS
{
#else
namespace GOOM::UTILS
{
#endif

void TValue::SetStepSize(const float val)
{
  assert(val >= 0.0F);
  m_stepSize = val;
  m_currentStep = m_currentStep < 0.0 ? -m_stepSize : +m_stepSize;
}

void TValue::Increment()
{
  switch (m_stepType)
  {
    case StepType::SINGLE_CYCLE:
      if (m_t > 1.0 + SMALL_FLOAT)
      {
        return;
      }
      m_t += m_currentStep;
      break;
    case StepType::CONTINUOUS_REPEATABLE:
      m_t += m_currentStep;
      if (m_t > 1.0F + SMALL_FLOAT)
      {
        HandleBoundary(1.0F + SMALL_FLOAT, 0.0F, +1.0F);
        return;
      }
      break;
    case StepType::CONTINUOUS_REVERSIBLE:
      m_t += m_currentStep;
      if (m_t > 1.0F + SMALL_FLOAT)
      {
        HandleBoundary(1.0F + SMALL_FLOAT, 1.0F, -1.0F);
      }
      else if (m_t < 0.0F - SMALL_FLOAT)
      {
        HandleBoundary(0.0 - SMALL_FLOAT, 0.0F, +1.0F);
      }
      break;
  }
}

void TValue::HandleBoundary(const float boundaryValue,
                            const float continueValue,
                            const float stepSign)
{
//  constexpr float DELAY_TIME_VARIANCE_AS_FRACTION = 0.1F;

  if (!m_startedDelay && m_delayTimeAtChanges > 0)
  {
    m_startedDelay = true;
    m_delayAtChangeCount = m_delayTimeAtChanges;
    /**
    const auto minDelayTime = static_cast<uint32_t>((1.0F - DELAY_TIME_VARIANCE_AS_FRACTION) *
                                                    static_cast<float>(m_delayTimeAtChanges));
    const auto maxDelayTime = static_cast<uint32_t>((1.0F + DELAY_TIME_VARIANCE_AS_FRACTION) *
                                                    static_cast<float>(m_delayTimeAtChanges));
    m_delayAtChangeCount = GetRandInRange(minDelayTime, maxDelayTime + 1);
     **/
  }

  if (m_delayAtChangeCount > 0)
  {
    m_delayAtChangeCount--;
    m_t = boundaryValue;
    return;
  }

  m_t = continueValue;

  if (stepSign < 0.0F)
  {
    m_currentStep = -m_stepSize;
    assert(m_currentStep <= 0.0F);
  }
  else
  {
    m_currentStep = +m_stepSize;
    assert(m_currentStep >= 0.0F);
  }

  m_startedDelay = false;
}

#if __cplusplus <= 201402L
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS
#endif
