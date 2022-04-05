#pragma once

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
  Pixel color{};
  Pixel lowColor{};
  Pixel innerColor{};
  Pixel innerLowColor{};
  Pixel outerCircleColor{};
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
      int x1, int y1, int x2, int y2, const std::vector<Pixel>& colors, uint8_t thickness)>;
  using DrawCircleFunc = std::function<void(
      int x, int y, int radius, const std::vector<Pixel>& colors, uint8_t thickness)>;
  using DrawSmallImageFunc =
      std::function<void(int xMid,
                         int yMid,
                         UTILS::GRAPHICS::SmallImageBitmaps::ImageNames imageName,
                         uint32_t size,
                         const std::vector<Pixel>& colors)>;
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
    std::shared_ptr<COLOR::RandomColorMaps> colorMaps;
    std::shared_ptr<COLOR::RandomColorMaps> lowColorMaps;
    float radiusEdgeOffset;
    float brightnessFactor;
  };

  Tube() noexcept = delete;
  Tube(const Data& data, const UTILS::MATH::OscillatingPath::Params& pathParams) noexcept;

  [[nodiscard]] auto IsActive() const -> bool;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> colorMaps);
  void SetWeightedLowColorMaps(std::shared_ptr<COLOR::RandomColorMaps> lowColorMaps);

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
  void SetCirclePathParams(const UTILS::MATH::OscillatingPath::Params& params);
  static const float NORMAL_CIRCLE_SPEED;
  void SetCircleSpeed(float val);
  void IncreaseCircleSpeed();
  void DecreaseCircleSpeed();

  void DrawCircleOfShapes();

private:
  class TubeImpl;
  const std::shared_ptr<TubeImpl> m_impl;
};

} // namespace VISUAL_FX::TUBES
} // namespace GOOM
