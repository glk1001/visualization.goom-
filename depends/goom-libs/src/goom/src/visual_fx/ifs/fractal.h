#pragma once

#include "fractal_hits.h"
#include "ifs_types.h"
#include "similitudes.h"
#include "utils/goom_rand_base.h"
#include "utils/t_values.h"

#include <cstdint>
#include <vector>

namespace GOOM
{

namespace COLOR
{
class RandomColorMaps;
}

namespace UTILS
{
class IGoomRand;
class ImageBitmap;
class SmallImageBitmaps;
}

namespace VISUAL_FX::IFS
{

class Fractal
{
public:
  Fractal(uint32_t screenWidth,
          uint32_t screenHeight,
          const UTILS::IGoomRand& goomRand,
          const COLOR::RandomColorMaps& randomColorMaps,
          const UTILS::SmallImageBitmaps& smallBitmaps);

  void Init();
  void Reset();

  [[nodiscard]] auto GetSpeed() const -> uint32_t;
  void SetSpeed(uint32_t val);

  [[nodiscard]] auto GetNextIfsPoints() -> const std::vector<IfsPoint>&;
  [[nodiscard]] auto GetMaxHitCount() const -> uint32_t;

private:
  Similitudes m_similitudes;
  const UTILS::IGoomRand& m_goomRand;

  const Flt m_halfWidth;
  const Flt m_halfHeight;

  static constexpr uint32_t MIN_MAX_COUNT_TIMES_SPEED = 950;
  static constexpr uint32_t MAX_MAX_COUNT_TIMES_SPEED = 1500;
  uint32_t m_iterationCount = 0;
  void UpdateIterationCount();

  static constexpr uint32_t INITIAL_SPEED = 6;
  uint32_t m_prevSpeed = INITIAL_SPEED;
  uint32_t m_speed = INITIAL_SPEED;
  static constexpr uint32_t NUM_SPEED_TRANSITION_STEPS = 500;
  UTILS::TValue m_speedTransitionT{UTILS::TValue::StepType::SINGLE_CYCLE,
                                   NUM_SPEED_TRANSITION_STEPS};
  uint32_t m_maxCountTimesSpeed = MAX_MAX_COUNT_TIMES_SPEED;

  FractalHits m_hits1;
  FractalHits m_hits2;
  std::reference_wrapper<FractalHits> m_prevHits;
  std::reference_wrapper<FractalHits> m_curHits;
  void InitHits();

  void UpdateMainSimis();
  [[nodiscard]] auto GetCurrentIfsPoints() -> const std::vector<IfsPoint>&;
  void DrawFractal();
  void Trace(uint32_t curDepth, const FltPoint& point0);
  [[nodiscard]] static auto AreSimilarPoints(const FltPoint& point1, const FltPoint& point2)
      -> bool;
  void UpdateHits(const Similitude& simi, const FltPoint& point);
};

inline auto Fractal::GetMaxHitCount() const -> uint32_t
{
  return m_curHits.get().GetMaxHitCount();
}

} // namespace VISUAL_FX::IFS
} // namespace GOOM
