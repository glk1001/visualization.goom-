module;

#include <cstdint>

export module Goom.FilterFx.CommonTypes;

import Goom.Utils.Math.GoomRand;

export namespace GOOM::FILTER_FX
{

template<typename T>
struct Amplitude_t // NOLINT(readability-identifier-naming)
{
  T x;
  T y;
};
using Amplitude    = Amplitude_t<float>;
using IntAmplitude = Amplitude_t<int32_t>;
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
  UTILS::MATH::NumberRange<float> xRange;
  UTILS::MATH::NumberRange<float> yRange;
};
struct FrequencyFactorRange
{
  UTILS::MATH::NumberRange<float> xRange;
  UTILS::MATH::NumberRange<float> yRange;
};
struct SqDistMultRange
{
  UTILS::MATH::NumberRange<float> xRange;
  UTILS::MATH::NumberRange<float> yRange;
};
struct SqDistOffsetRange
{
  UTILS::MATH::NumberRange<float> xRange;
  UTILS::MATH::NumberRange<float> yRange;
};

} // namespace GOOM::FILTER_FX
