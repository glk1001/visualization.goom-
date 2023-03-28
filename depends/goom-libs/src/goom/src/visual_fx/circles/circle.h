#pragma once

#include "bitmap_getter_base.h"
#include "color/color_adjustment.h"
#include "color/color_maps.h"
#include "color/color_maps_grids.h"
#include "color/random_color_maps.h"
#include "dot_diameters.h"
#include "dot_drawer.h"
#include "dot_paths.h"
#include "draw/goom_draw.h"
#include "draw/shape_drawers/line_drawer_noisy_pixels.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "helper.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/paths.h"
#include "utils/propagate_const.h"
#include "utils/t_values.h"
#include "utils/timer.h"
#include "visual_fx/fx_helper.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

class Circle
{
public:
  struct OneWayParams
  {
    float circleRadius{};
    Point2dInt circleCentreStart;
    Point2dInt circleCentreTarget;
  };
  struct Params
  {
    OneWayParams toTargetParams;
    OneWayParams fromTargetParams;
  };

  Circle(const FxHelper& fxHelper,
         const Helper& helper,
         const Params& circleParams,
         const UTILS::MATH::OscillatingFunction::Params& pathParams) noexcept;

  auto SetWeightedColorMaps(
      const std::shared_ptr<const COLOR::RandomColorMaps>& weightedMainMaps,
      const std::shared_ptr<const COLOR::RandomColorMaps>& weightedLowMaps) noexcept -> void;
  auto SetPathParams(const UTILS::MATH::OscillatingFunction::Params& pathParams) noexcept -> void;
  [[nodiscard]] auto GetCurrentDirection() const noexcept -> DotPaths::Direction;
  auto ChangeDirection(DotPaths::Direction newDirection) noexcept -> void;
  auto SetGlobalBrightnessFactor(float val) noexcept -> void;

  auto Start() noexcept -> void;
  auto UpdatePositionSpeed(uint32_t newNumSteps) noexcept -> void;
  auto UpdateAndDraw() noexcept -> void;
  auto IncrementTs() noexcept -> void;

  [[nodiscard]] auto HasPositionTJustHitABoundary() const noexcept -> bool;
  [[nodiscard]] auto HasPositionTJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasPositionTJustHitEndBoundary() const noexcept -> bool;

private:
  DRAW::SHAPE_DRAWERS::LineDrawerNoisyPixels m_lineDrawer;
  const UTILS::MATH::IGoomRand* m_goomRand;
  Helper m_helper;

  class CircleDots;
  std::unique_ptr<CircleDots> m_circleDots;

  static constexpr auto MIN_NUM_DOTS = 30U;
  static constexpr auto MAX_NUM_DOTS = 50U;
  [[nodiscard]] auto GetNewNumDots() const noexcept -> uint32_t;
  bool m_resetNumDotsRequired = false;
  auto ResetNumDots() noexcept -> void;
  auto DoResetNumDots() noexcept -> void;

  uint64_t m_updateNum       = 0U;
  uint32_t m_dotRotateOffset = 0U;
  bool m_dotRotateIncrement  = true;
  [[nodiscard]] auto IsSpecialUpdateNum() const noexcept -> bool;
  [[nodiscard]] auto IsSpecialLineUpdateNum() const noexcept -> bool;

  auto UpdateTime() noexcept -> void;
  auto ResetNumSteps() noexcept -> void;

  uint32_t m_newNumSteps = 0U;

  auto DrawNextCircle() noexcept -> void;
  auto DrawNextCircleDots() noexcept -> void;

  static constexpr auto GAMMA = 1.0F;
  COLOR::ColorAdjustment m_colorAdjustment{
      {GAMMA, COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };

  float m_globalBrightnessFactor = 1.0F;
  [[nodiscard]] auto GetCurrentBrightness() const noexcept -> float;
  [[nodiscard]] auto GetDotBrightness(float brightness) const noexcept -> float;
  [[nodiscard]] auto GetLineBrightness(float brightness) const noexcept -> float;

  bool m_alternateMainLowDotColors = false;
  bool m_showLine                  = false;
  std::experimental::propagate_const<std::unique_ptr<DotDrawer>> m_dotDrawer;
  auto DrawLine(const Point2dInt& position1,
                const Point2dInt& position2,
                float lineBrightness,
                float tLineColor) noexcept -> void;
  [[nodiscard]] auto DrawLineDots(float lineBrightness,
                                  const Point2dInt& position1,
                                  const Point2dInt& position2,
                                  float tLineColor) noexcept -> float;
  auto DrawDot(uint32_t dotNum, const Point2dInt& pos, const DRAW::MultiplePixels& colors) noexcept
      -> void;
  auto DrawConnectingLine(const Point2dInt& position1,
                          const Point2dInt& position2,
                          float lineBrightness,
                          float tDotColor) noexcept -> void;

  std::shared_ptr<const COLOR::RandomColorMaps> m_mainColorMaps;
  std::shared_ptr<const COLOR::RandomColorMaps> m_lowColorMaps;
  const COLOR::IColorMap* m_linesMainColorMap{};
  const COLOR::IColorMap* m_linesLowColorMap{};
  uint32_t m_numRotatingColors = 0;
  std::vector<const COLOR::IColorMap*> m_rotatingMainColorMaps{};
  std::vector<const COLOR::IColorMap*> m_rotatingLowColorMaps{};
  std::vector<uint32_t> m_rotatingDotNums{};
  static constexpr uint32_t NUM_ROTATING_COLOR_STEPS = 100U;
  UTILS::TValue m_rotatingColorsT{
      {UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_ROTATING_COLOR_STEPS}
  };
  auto UpdateRotatingColorMaps() noexcept -> void;

  enum class GridColorRange
  {
    ONE,
    LOW,
    MEDIUM,
    HIGH,
    _num // unused, and marks the enum end
  };
  GridColorRange m_currentGridColorRange = GridColorRange::ONE;
  GOOM::UTILS::MATH::Weights<GridColorRange> m_weightedGridColorRanges;
  uint32_t m_numDifferentGridMaps                 = 1U;
  COLOR::ColorMapsGrid m_mainColorMapsGrid        = GetMainColorMapsGrid();
  COLOR::ColorMapsGrid m_lowColorMapsGrid         = GetLowColorMapsGrid();
  static constexpr float DEFAULT_COLOR_GRID_MIX_T = 0.5F;
  float m_currentColorGridMixT                    = DEFAULT_COLOR_GRID_MIX_T;
  auto UpdateNumDifferentGridMaps() noexcept -> void;
  [[nodiscard]] auto GetMainColorMapsGrid() const noexcept -> COLOR::ColorMapsGrid;
  [[nodiscard]] auto GetLowColorMapsGrid() const noexcept -> COLOR::ColorMapsGrid;
  [[nodiscard]] auto GetHorizontalMainColorMaps() const noexcept
      -> std::vector<const COLOR::IColorMap*>;
  [[nodiscard]] auto GetVerticalMainColorMaps() const noexcept
      -> std::vector<const COLOR::IColorMap*>;
  [[nodiscard]] auto GetHorizontalLowColorMaps() const noexcept
      -> std::vector<const COLOR::IColorMap*>;
  [[nodiscard]] auto GetVerticalLowColorMaps() const noexcept
      -> std::vector<const COLOR::IColorMap*>;
  [[nodiscard]] auto GetAllDotColorMaps(const COLOR::RandomColorMaps& baseRandomColorMaps)
      const noexcept -> std::vector<const COLOR::IColorMap*>;

  [[nodiscard]] auto GetColorMixT(float tX, float tY) const noexcept -> float;
  struct AllDotColors
  {
    std::vector<Pixel> mainColors;
    std::vector<Pixel> lowColors;
  };
  [[nodiscard]] auto GetAllDotColors() const noexcept -> AllDotColors;
  [[nodiscard]] auto GetSingleDotColors(uint32_t dotNum,
                                        const AllDotColors& allDotColors,
                                        float dotBrightness) const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetCorrectedColor(float brightness, const Pixel& color) const noexcept
      -> Pixel;
};

class Circle::CircleDots
{
public:
  CircleDots(const UTILS::MATH::IGoomRand& goomRand,
             const Helper& helper,
             const Params& circleParams,
             const UTILS::MATH::OscillatingFunction::Params& pathParams,
             uint32_t numDots) noexcept;

  auto SetPathParams(const UTILS::MATH::OscillatingFunction::Params& pathParams) noexcept -> void;
  auto ResetNumDots(uint32_t numDots) noexcept -> void;

  [[nodiscard]] auto GetNumDots() const noexcept -> uint32_t;
  [[nodiscard]] auto GetDotPaths() const noexcept -> const DotPaths&;
  [[nodiscard]] auto GetDotPaths() noexcept -> DotPaths&;
  [[nodiscard]] auto GetDotDiameters() const noexcept -> const DotDiameters&;
  [[nodiscard]] auto GetDotDiameters() noexcept -> DotDiameters&;
  [[nodiscard]] auto GetTLineColorStep() const noexcept -> float;
  [[nodiscard]] auto GetNumMaps() const noexcept -> const UTILS::EnumMap<GridColorRange, uint32_t>&;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  const Helper* m_helper;
  Params m_circleParams;
  UTILS::MATH::OscillatingFunction::Params m_pathParams;

  uint32_t m_numDots;

  DotPaths m_dotPaths;
  [[nodiscard]] auto GetNewDotPaths() const noexcept -> DotPaths;
  [[nodiscard]] static auto GetDotStartingPositions(uint32_t numDots,
                                                    const Point2dInt& centre,
                                                    float radius) noexcept
      -> std::vector<Point2dInt>;
  DotDiameters m_dotDiameters;
  [[nodiscard]] auto GetNewDotDiameters() const noexcept -> DotDiameters;

  float m_tLineColorStep;
  [[nodiscard]] auto GetNewTLineColorStep() const noexcept -> float;

  UTILS::EnumMap<GridColorRange, uint32_t> m_numMaps;
  [[nodiscard]] auto GetNewNumMaps() const noexcept -> UTILS::EnumMap<GridColorRange, uint32_t>;
};

inline auto Circle::IncrementTs() noexcept -> void
{
  m_circleDots->GetDotPaths().IncrementPositionT();

  if (m_circleDots->GetDotPaths().HasUpdatedDotPathsToAndFrom())
  {
    m_mainColorMapsGrid.SetVerticalT(m_circleDots->GetDotPaths().GetPositionTRef());
    m_lowColorMapsGrid.SetVerticalT(m_circleDots->GetDotPaths().GetPositionTRef());
    m_circleDots->GetDotPaths().ResetUpdatedDotPathsToAndFromFlag();
  }
}

inline auto Circle::UpdatePositionSpeed(const uint32_t newNumSteps) noexcept -> void
{
  m_newNumSteps = newNumSteps;
}

inline auto Circle::GetCurrentDirection() const noexcept -> DotPaths::Direction
{
  return m_circleDots->GetDotPaths().GetCurrentDirection();
}

inline auto Circle::ChangeDirection(const DotPaths::Direction newDirection) noexcept -> void
{
  m_circleDots->GetDotPaths().ChangeDirection(newDirection);
}

inline auto Circle::HasPositionTJustHitABoundary() const noexcept -> bool
{
  return m_circleDots->GetDotPaths().HasPositionTJustHitStartBoundary() or
         m_circleDots->GetDotPaths().HasPositionTJustHitEndBoundary();
}

inline auto Circle::HasPositionTJustHitStartBoundary() const noexcept -> bool
{
  return m_circleDots->GetDotPaths().HasPositionTJustHitStartBoundary();
}

inline auto Circle::HasPositionTJustHitEndBoundary() const noexcept -> bool
{
  return m_circleDots->GetDotPaths().HasPositionTJustHitEndBoundary();
}

inline auto Circle::CircleDots::GetNumDots() const noexcept -> uint32_t
{
  return m_numDots;
}

inline auto Circle::CircleDots::GetDotPaths() const noexcept -> const DotPaths&
{
  return m_dotPaths;
}

inline auto Circle::CircleDots::GetDotPaths() noexcept -> DotPaths&
{
  return m_dotPaths;
}

inline auto Circle::CircleDots::GetDotDiameters() const noexcept -> const DotDiameters&
{
  return m_dotDiameters;
}

inline auto Circle::CircleDots::GetDotDiameters() noexcept -> DotDiameters&
{
  return m_dotDiameters;
}

inline auto Circle::CircleDots::GetTLineColorStep() const noexcept -> float
{
  return m_tLineColorStep;
}

inline auto Circle::CircleDots::GetNumMaps() const noexcept
    -> const UTILS::EnumMap<GridColorRange, uint32_t>&
{
  return m_numMaps;
};

} // namespace GOOM::VISUAL_FX::CIRCLES
