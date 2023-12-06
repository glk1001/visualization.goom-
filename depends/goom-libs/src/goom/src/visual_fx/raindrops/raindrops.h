#pragma once

#include "../fx_helper.h"
#include "color/color_adjustment.h"
#include "color/color_maps.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "draw/shape_drawers/circle_drawer.h"
#include "draw/shape_drawers/line_drawer.h"
#include "goom/goom_graphic.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "raindrop_positions.h"
#include "utils/graphics/blend2d_to_goom.h"
#include "utils/t_values.h"

#include <blend2d.h> // NOLINT(misc-include-cleaner): Blend2d insists on this.
#include <blend2d/context.h>
#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::RAINDROPS
{

class Raindrops
{
public:
  static constexpr auto MIN_NUM_RAINDROPS   = 50U;
  static constexpr auto MAX_NUM_RAINDROPS   = 100U;
  static constexpr auto NUM_START_RAINDROPS = 60U;

  Raindrops(FxHelper& fxHelper,
            uint32_t numRaindrops,
            const COLOR::WeightedRandomColorMaps& randomMainColorMaps,
            const COLOR::WeightedRandomColorMaps& randomLowColorMaps,
            const Rectangle2dInt& rectangle2D,
            const Point2dInt& targetRectangleWeightPoint) noexcept;

  auto SetNumRaindrops(uint32_t newNumRaindrops) noexcept -> void;
  auto SetRectangleWeightPoint(const Point2dInt& targetRectangleWeightPoint) noexcept -> void;
  auto SetWeightedColorMaps(const COLOR::WeightedRandomColorMaps& randomMainColorMaps,
                            const COLOR::WeightedRandomColorMaps& randomLowColorMaps) noexcept
      -> void;

  auto DrawRaindrops() noexcept -> void;
  auto UpdateRaindrops() noexcept -> void;

private:
  FxHelper* m_fxHelper;
  Point2dInt m_screenCentre = m_fxHelper->GetDimensions().GetCentrePoint();
  COLOR::WeightedRandomColorMaps m_randomMainColorMaps;
  COLOR::WeightedRandomColorMaps m_randomLowColorMaps;
  DRAW::SHAPE_DRAWERS::CircleDrawer m_circleDrawer;
  DRAW::SHAPE_DRAWERS::LineDrawerNoClippedEndPoints m_lineDrawer;
  Pixel m_mainWeightPointColor{};
  Pixel m_lowWeightPointColor{};

  UTILS::GRAPHICS::Blend2dToGoom m_blend2dToMainBuffer;
  UTILS::GRAPHICS::Blend2dToGoom m_blend2dToLowBuffer;
  static auto FillCircle(BLContext& blend2dContext,
                         const Pixel& color,
                         float brightness,
                         const Point2dInt& centre,
                         double radius) noexcept -> void;
  auto AddBlend2dImagesToGoomBuffers() -> void;
  auto Blend2dClearAll() -> void;

  static constexpr auto RADIUS_TO_RECT_SIDE_FRAC       = 1.0F / 50.0F;
  static constexpr auto MIN_TO_MAX_RADIUS_FRAC         = 1.0F / 50.0F;
  static constexpr auto MAX_GROWTH_FACTOR              = 3.0F;
  static constexpr auto MIN_GROWTH_STEPS               = 10U;
  static constexpr auto MAX_GROWTH_STEPS               = 50U;
  static constexpr auto WEIGHT_POINT_CIRCLE_BRIGHTNESS = 1.30F;
  static constexpr auto LINE_TO_TARGET_BRIGHTNESS      = 0.05F;
  static constexpr auto LINE_TO_NEXT_DROP_BRIGHTNESS   = 0.10F;
  static constexpr auto MIN_DROP_BRIGHTNESS            = 0.10F;
  static constexpr auto MAX_DROP_BRIGHTNESS            = 2.50F;
  static constexpr auto LOW_BRIGHTNESS_INCREASE        = 1.10F;
  static constexpr auto WEIGHT_POINT_RADIUS_FRAC       = 0.015F;
  static constexpr auto MAX_LINE_THICKNESS             = 2U;
  static constexpr auto MIN_NUM_CONCENTRIC_CIRCLES     = 3U;
  static constexpr auto MAX_NUM_CONCENTRIC_CIRCLES     = 7U;
  struct RaindropParams
  {
    Rectangle2dInt rectangle2D{};
    float minStartingRadius{};
    float maxStartingRadius{};
    uint32_t numConcentricCircles{};
    float maxGrowthRadius{};
    COLOR::ColorMapPtrWrapper sameMainColorMap{nullptr};
    COLOR::ColorMapPtrWrapper sameLowColorMap{nullptr};
  };
  RaindropParams m_raindropParams;
  [[nodiscard]] auto GetNewRaindropParams(const Rectangle2dInt& rectangle2D) const noexcept
      -> RaindropParams;
  [[nodiscard]] auto GetNewRaindropPositionParams(
      const Rectangle2dInt& rectangle2D,
      const Point2dInt& targetRectangleWeightPoint) const noexcept -> RaindropPositions::Params;
  [[nodiscard]] auto GetNewSourceRectangleWeightPoint(const Point2dInt& focusPoint) const noexcept
      -> Point2dInt;
  [[nodiscard]] auto GetFracFromWeightPoint(const Point2dInt& position) const noexcept -> float;

  RaindropPositions m_raindropPositions;

  struct Raindrop
  {
    uint32_t dropNum{};
    uint8_t lineThickness{};
    float fracFromWeightPoint{};
    UTILS::IncrementedValue<float> growthRadius;
    COLOR::ColorMapPtrWrapper mainColorMap{nullptr};
    COLOR::ColorMapPtrWrapper lowColorMap{nullptr};
    UTILS::TValue colorT;
  };
  std::vector<Raindrop> m_raindrops;
  uint32_t m_pendingNewNumRaindrops = 0U;
  auto UpdateAnyPendingNumRaindrops() noexcept -> void;
  [[nodiscard]] auto GetAcceptableNumRaindrops(uint32_t numRequestedRaindrops) const noexcept
      -> uint32_t;
  [[nodiscard]] auto GetNewRaindrops(uint32_t numRaindrops) const noexcept -> std::vector<Raindrop>;
  [[nodiscard]] auto GetNewRaindrop(uint32_t dropNum) const noexcept -> Raindrop;
  static auto UpdateRaindrop(Raindrop& raindrop) noexcept -> void;

  [[nodiscard]] auto GetRaindropColors(const Raindrop& raindrop) const noexcept
      -> DRAW::MultiplePixels;
  auto DrawRaindrop(const Raindrop& raindrop, const DRAW::MultiplePixels& colors) noexcept -> void;
  auto DrawCircleAroundWeightPoint() noexcept -> void;

  static constexpr auto GAMMA = 2.2F;
  COLOR::ColorAdjustment m_colorAdjustment{
      {GAMMA, COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };
};

inline auto Raindrops::SetNumRaindrops(const uint32_t newNumRaindrops) noexcept -> void
{
  m_pendingNewNumRaindrops = newNumRaindrops;
}

} // namespace GOOM::VISUAL_FX::RAINDROPS
