#pragma once

#include "bitmap_getter.h"
#include "circle.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "point2d.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "visual_fx/fx_helper.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

class Circles
{
public:
  Circles(const FxHelper& fxHelper,
          const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
          uint32_t numCircles,
          const std::vector<Circle::Params>& circleParams);

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps,
                            std::shared_ptr<COLOR::RandomColorMaps> weightedLowMaps);

  void SetZoomMidpoint(const Point2dInt& zoomMidpoint);

  void Start();
  void UpdateAndDraw();

private:
  BitmapGetter m_bitmapGetter;

  static constexpr float GAMMA = 1.0F / 2.2F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  COLOR::GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};

  const uint32_t m_numCircles;
  std::vector<Circle> m_circles;
  [[nodiscard]] static auto GetCircles(const FxHelper& fxHelper,
                                       const Circle::Helper& helper,
                                       uint32_t numCircles,
                                       const std::vector<Circle::Params>& circleParams)
      -> std::vector<Circle>;
};

} // namespace GOOM::VISUAL_FX::CIRCLES
