#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

template<class T, class U>
class FunctionOfFunction : public IZoomAdjustmentEffect
{
public:
  explicit FunctionOfFunction(const UTILS::MATH::IGoomRand& goomRand) noexcept;
  template<typename FuncArg>
  FunctionOfFunction(const FuncArg& funcArg, const UTILS::MATH::IGoomRand& goomRand) noexcept;
  template<typename FuncOfArg, typename FuncArg>
  FunctionOfFunction(const FuncOfArg& funcOfArg,
                     const FuncArg& funcArg,
                     const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

private:
  T m_funcOf;
  U m_func;
};

template<class T, class U>
inline FunctionOfFunction<T, U>::FunctionOfFunction(const UTILS::MATH::IGoomRand& goomRand) noexcept
  : m_funcOf{goomRand}, m_func{goomRand}
{
}

template<class T, class U>
template<typename FuncArg>
inline FunctionOfFunction<T, U>::FunctionOfFunction(const FuncArg& funcArg,
                                                    const UTILS::MATH::IGoomRand& goomRand) noexcept
  : m_funcOf{goomRand}, m_func{funcArg, goomRand}
{
}

template<class T, class U>
template<typename FuncOfArg, typename FuncArg>
inline FunctionOfFunction<T, U>::FunctionOfFunction(const FuncOfArg& funcOfArg,
                                                    const FuncArg& funcArg,
                                                    const UTILS::MATH::IGoomRand& goomRand) noexcept
  : m_funcOf{funcOfArg, goomRand}, m_func{funcArg, goomRand}
{
}

template<class T, class U>
inline auto FunctionOfFunction<T, U>::SetRandomParams() noexcept -> void
{
  m_func.SetRandomParams();
  m_funcOf.SetRandomParams();
}

template<class T, class U>
inline auto FunctionOfFunction<T, U>::GetZoomAdjustment(
    const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto funcCoords = m_func.GetZoomAdjustment(coords);
  return m_funcOf.GetZoomAdjustment({funcCoords.x, funcCoords.y});
}

template<class T, class U>
inline auto FunctionOfFunction<T, U>::GetZoomAdjustmentEffectNameValueParams() const noexcept
    -> UTILS::NameValuePairs
{
  // TODO(glk) - Incorporate m_funcOf.
  return m_func.GetZoomAdjustmentEffectNameValueParams();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
