#pragma once

#include "utils/math/goom_rand_base.h"

namespace GOOM::FILTER_FX
{

struct Amplitude
{
  float x;
  float y;
};
struct IntAmplitude
{
  int32_t x;
  int32_t y;
};
struct FrequencyFactor
{
  float x;
  float y;
};
struct SqDistMult
{
  float x;
  float y;
};
struct SqDistOffset
{
  float x;
  float y;
};

struct AmplitudeRange
{
  UTILS::MATH::IGoomRand::NumberRange<float> xRange;
  UTILS::MATH::IGoomRand::NumberRange<float> yRange;
};
struct FrequencyFactorRange
{
  UTILS::MATH::IGoomRand::NumberRange<float> xRange;
  UTILS::MATH::IGoomRand::NumberRange<float> yRange;
};
struct SqDistMultRange
{
  UTILS::MATH::IGoomRand::NumberRange<float> xRange;
  UTILS::MATH::IGoomRand::NumberRange<float> yRange;
};
struct SqDistOffsetRange
{
  UTILS::MATH::IGoomRand::NumberRange<float> xRange;
  UTILS::MATH::IGoomRand::NumberRange<float> yRange;
};

} // namespace GOOM::FILTER_FX
