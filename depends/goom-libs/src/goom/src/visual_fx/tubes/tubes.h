#pragma once

#include "goom/goom_graphic.h"
#include "goom/point2d.h"
#include "goom/spimpl.h"
#include "tube_data.h"
#include "utils/math/parametric_functions2d.h"

#include <cstdint>
#include <functional>

namespace GOOM::COLOR
{
class WeightedRandomColorMaps;
}

namespace GOOM::VISUAL_FX::TUBES
{

enum class ColorMapMixMode
{
  SHAPES_ONLY,
  STRIPED_SHAPES_ONLY,
  CIRCLES_ONLY,
  SHAPES_AND_CIRCLES,
  STRIPED_SHAPES_AND_CIRCLES,
};

struct ShapeColors
{
  Pixel mainColor{};
  Pixel lowColor{};
  Pixel innerMainColor{};
  Pixel innerLowColor{};
  Pixel outerCircleMainColor{};
  Pixel outerCircleLowColor{};
};

class BrightnessAttenuation
{
public:
  static constexpr float DIST_SQ_CUTOFF = 0.10F;
  struct Properties
  {
    uint32_t screenWidth{};
    uint32_t screenHeight{};
    float cutoffBrightness{};
  };

  explicit BrightnessAttenuation(const Properties& properties) noexcept;
  [[nodiscard]] auto GetPositionBrightness(const Point2dInt& pos,
                                           float minBrightnessPastCutoff) const noexcept -> float;

private:
  float m_cutoffBrightness;
  uint32_t m_maxRSquared;
  [[nodiscard]] auto GetDistFromCentreFactor(const Point2dInt& pos) const noexcept -> float;
};

class Tube
{
public:
  Tube(const TubeData& data, const UTILS::MATH::OscillatingFunction::Params& pathParams) noexcept;

  [[nodiscard]] auto IsActive() const noexcept -> bool;

  auto SetWeightedMainColorMaps(const COLOR::WeightedRandomColorMaps& weightedMaps) noexcept
      -> void;
  auto SetWeightedLowColorMaps(const COLOR::WeightedRandomColorMaps& weightedMaps) noexcept -> void;

  auto ResetColorMaps() noexcept -> void;

  auto SetBrightnessFactor(float val) noexcept -> void;

  auto SetMaxJitterOffset(int32_t val) noexcept -> void;

  using TransformCentreFunc = std::function<Vec2dInt(uint32_t tubeId, const Point2dInt& centre)>;
  auto SetTransformCentreFunc(const TransformCentreFunc& func) noexcept -> void;
  auto SetCentrePathT(float val) noexcept -> void;
  static const float NORMAL_CENTRE_SPEED;
  auto SetCentreSpeed(float val) noexcept -> void;
  auto IncreaseCentreSpeed() noexcept -> void;
  auto DecreaseCentreSpeed() noexcept -> void;

  auto SetAllowOscillatingCirclePaths(bool val) noexcept -> void;
  auto SetCirclePathParams(const UTILS::MATH::OscillatingFunction::Params& params) noexcept -> void;
  static const float NORMAL_CIRCLE_SPEED;
  auto SetCircleSpeed(float val) noexcept -> void;
  auto IncreaseCircleSpeed() noexcept -> void;
  auto DecreaseCircleSpeed() noexcept -> void;

  auto DrawCircleOfShapes() noexcept -> void;

private:
  class TubeImpl;
  spimpl::unique_impl_ptr<TubeImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX::TUBES
