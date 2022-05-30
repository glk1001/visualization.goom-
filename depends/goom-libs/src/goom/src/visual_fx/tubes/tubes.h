#pragma once

#include "draw/goom_draw.h"
#include "goom/spimpl.h"
#include "goom_graphic.h"
#include "point2d.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/paths.h"
#include "utils/timer.h"

#include <functional>
#include <memory>

namespace GOOM
{

namespace COLOR
{
class RandomColorMaps;
}

namespace UTILS::MATH
{
class IGoomRand;
}

namespace VISUAL_FX::TUBES
{

enum class ColorMapMixMode
{
  SHAPES_ONLY,
  STRIPED_SHAPES_ONLY,
  CIRCLES_ONLY,
  SHAPES_AND_CIRCLES,
  STRIPED_SHAPES_AND_CIRCLES,
  _num // unused and must be last
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
  BrightnessAttenuation(uint32_t screenWidth, uint32_t screenHeight, float cutoffBrightness);
  [[nodiscard]] auto GetPositionBrightness(const Point2dInt& pos,
                                           float minBrightnessPastCutoff) const -> float;

private:
  const float m_cutoffBrightness;
  const uint32_t m_maxRSquared;
  [[nodiscard]] auto GetDistFromCentreFactor(const Point2dInt& pos) const -> float;
};

class Tube
{
public:
  using DrawLineFunc = std::function<void(
      Point2dInt point1, Point2dInt point2, const DRAW::MultiplePixels& colors, uint8_t thickness)>;
  using DrawCircleFunc = std::function<void(
      Point2dInt point, int radius, const DRAW::MultiplePixels& colors, uint8_t thickness)>;
  using DrawSmallImageFunc =
      std::function<void(Point2dInt midPoint,
                         UTILS::GRAPHICS::SmallImageBitmaps::ImageNames imageName,
                         uint32_t size,
                         const DRAW::MultiplePixels& colors)>;
  struct DrawFuncs
  {
    DrawLineFunc drawLine;
    DrawCircleFunc drawCircle;
    DrawSmallImageFunc drawSmallImage;
  };
  struct Data
  {
    uint32_t tubeId;
    DrawFuncs drawFuncs;
    uint32_t screenWidth;
    uint32_t screenHeight;
    const UTILS::MATH::IGoomRand& goomRand;
    std::shared_ptr<COLOR::RandomColorMaps> mainColorMaps;
    std::shared_ptr<COLOR::RandomColorMaps> lowColorMaps;
    float radiusEdgeOffset;
    float brightnessFactor;
  };

  Tube() noexcept = delete;
  Tube(const Data& data, const UTILS::MATH::OscillatingFunction::Params& pathParams) noexcept;

  [[nodiscard]] auto IsActive() const -> bool;

  void SetWeightedMainColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);
  void SetWeightedLowColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  void ResetColorMaps();

  void SetBrightnessFactor(float val);

  void SetMaxJitterOffset(int32_t val);

  using TransformCentreFunc = std::function<Vec2dInt(uint32_t tubeId, const Point2dInt& centre)>;
  void SetTransformCentreFunc(const TransformCentreFunc& func);
  void SetCentrePathT(float val);
  static const float NORMAL_CENTRE_SPEED;
  void SetCentreSpeed(float val);
  void IncreaseCentreSpeed();
  void DecreaseCentreSpeed();

  void SetAllowOscillatingCirclePaths(bool val);
  void SetCirclePathParams(const UTILS::MATH::OscillatingFunction::Params& params);
  static const float NORMAL_CIRCLE_SPEED;
  void SetCircleSpeed(float val);
  void IncreaseCircleSpeed();
  void DecreaseCircleSpeed();

  void DrawCircleOfShapes();

private:
  class TubeImpl;
  spimpl::unique_impl_ptr<TubeImpl> m_impl;
};

} // namespace VISUAL_FX::TUBES
} // namespace GOOM
