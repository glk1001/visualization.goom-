#pragma once

#include "goom_graphic.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/timer.h"

#include <functional>
#include <memory>

namespace GOOM
{

namespace COLOR
{
class RandomColorMaps;
} // namespace COLOR

namespace TUBES
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
  Pixel color{};
  Pixel lowColor{};
  Pixel innerColor{};
  Pixel innerLowColor{};
  Pixel outerCircleColor{};
  Pixel outerCircleLowColor{};
};

struct PathParams
{
  float oscillatingAmplitude = 1.0;
  float xOscillatingFreq = 1.0;
  float yOscillatingFreq = 1.0;
};

class BrightnessAttenuation
{
public:
  static constexpr float DIST_SQ_CUTOFF = 0.10F;
  BrightnessAttenuation(uint32_t screenWidth, uint32_t screenHeight, float cutoffBrightness);
  auto GetPositionBrightness(const V2dInt& pos, float minBrightnessPastCutoff) const -> float;

private:
  const float m_cutoffBrightness;
  const uint32_t m_maxRSquared;
  auto GetDistFromCentreFactor(const V2dInt& pos) const -> float;
};

class Tube
{
public:
  using DrawLineFunc = std::function<void(
      int x1, int y1, int x2, int y2, const std::vector<Pixel>& colors, uint8_t thickness)>;
  using DrawCircleFunc = std::function<void(
      int x, int y, int radius, const std::vector<Pixel>& colors, uint8_t thickness)>;
  using DrawSmallImageFunc = std::function<void(int xMid,
                                                int yMid,
                                                UTILS::SmallImageBitmaps::ImageNames imageName,
                                                uint32_t size,
                                                const std::vector<Pixel>& colors)>;
  struct DrawFuncs
  {
    DrawLineFunc drawLine;
    DrawCircleFunc drawCircle;
    DrawSmallImageFunc drawSmallImage;
  };

  Tube() noexcept = delete;
  Tube(uint32_t tubeId,
       const DrawFuncs& drawFuncs,
       uint32_t screenWidth,
       uint32_t screenHeight,
       std::shared_ptr<COLOR::RandomColorMaps> colorMaps,
       std::shared_ptr<COLOR::RandomColorMaps> lowColorMaps,
       float radiusEdgeOffset,
       float brightnessFactor) noexcept;

  [[nodiscard]] auto GetTubeId() const -> uint32_t;
  [[nodiscard]] auto IsActive() const -> bool;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> colorMaps);
  void SetWeightedLowColorMaps(std::shared_ptr<COLOR::RandomColorMaps> lowColorMaps);

  void ResetColorMaps();
  void RotateShapeColorMaps();

  [[nodiscard]] auto GetBrightnessFactor() const -> float;
  void SetBrightnessFactor(float val);

  [[nodiscard]] auto GetMaxJitterOffset() const -> int32_t;
  void SetMaxJitterOffset(int32_t val);

  using TransformCentreFunc = std::function<V2dInt(uint32_t tubeId, const V2dInt& centre)>;
  void SetTransformCentreFunc(const TransformCentreFunc& f);
  [[nodiscard]] auto GetCentrePathT() const -> float;
  void SetCentrePathT(float val);
  [[nodiscard]] auto GetCentreSpeed() const -> float;
  static const float NORMAL_CENTRE_SPEED;
  void SetCentreSpeed(float val);
  void IncreaseCentreSpeed();
  void DecreaseCentreSpeed();

  void SetAllowOscillatingCirclePaths(bool val);
  void SetCirclePathParams(const PathParams& params);
  [[nodiscard]] auto GetCircleSpeed() const -> float;
  static const float NORMAL_CIRCLE_SPEED;
  void SetCircleSpeed(float val);
  void IncreaseCircleSpeed();
  void DecreaseCircleSpeed();

  void DrawCircleOfShapes();

private:
  class TubeImpl;
  const std::shared_ptr<TubeImpl> m_impl;
};

} // namespace TUBES
} // namespace GOOM
