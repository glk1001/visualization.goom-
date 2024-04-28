module;

#include "color/color_adjustment.h"
#include "color/color_maps.h"
#include "color/color_maps_grids.h"
#include "color/color_utils.h"
#include "color/random_color_maps.h"
#include "color/random_color_maps_groups.h"
#include "draw/goom_draw.h"
#include "draw/shape_drawers/line_drawer_noisy_pixels.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/goom_logger.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/parametric_functions2d.h"
#include "utils/math/paths.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

module Goom.VisualFx.CirclesFx:Circle;

import Goom.Utils;
import Goom.VisualFx.FxHelper;
import :Helper;
import :DotDiameters;
import :DotDrawer;
import :DotPaths;

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

  Circle(FxHelper& fxHelper,
         const Helper& helper,
         const Params& circleParams,
         const UTILS::MATH::OscillatingFunction::Params& pathParams) noexcept;

  auto SetWeightedColorMaps(const COLOR::WeightedRandomColorMaps& weightedMainMaps,
                            const COLOR::WeightedRandomColorMaps& weightedLowMaps) noexcept -> void;
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
  FxHelper* m_fxHelper;
  Helper m_helper;
  DRAW::SHAPE_DRAWERS::LineDrawerNoisyPixels m_lineDrawer;

  [[nodiscard]] auto IsSpecialUpdateNum() const noexcept -> bool;
  [[nodiscard]] auto IsSpecialLineUpdateNum() const noexcept -> bool;
  auto ResetNumSteps() noexcept -> void;

  class CircleDots;
  std::unique_ptr<CircleDots> m_circleDots;

  static constexpr auto MIN_NUM_DOTS = 30U;
  static constexpr auto MAX_NUM_DOTS = 50U;
  [[nodiscard]] auto GetNewNumDots() const noexcept -> uint32_t;
  bool m_resetNumDotsRequired = false;
  auto ResetNumDots() noexcept -> void;
  auto DoResetNumDots() noexcept -> void;

  uint32_t m_dotRotateOffset = 0U;
  bool m_dotRotateIncrement  = true;
  uint32_t m_newNumSteps     = 0U;

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
  std::unique_ptr<DotDrawer> m_dotDrawer;
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

  COLOR::WeightedRandomColorMaps m_mainColorMaps;
  COLOR::WeightedRandomColorMaps m_lowColorMaps;
  COLOR::ColorMapPtrWrapper m_linesMainColorMap{nullptr};
  COLOR::ColorMapPtrWrapper m_linesLowColorMap{nullptr};
  uint32_t m_numRotatingColors = 0;
  std::vector<COLOR::ColorMapPtrWrapper> m_rotatingMainColorMaps{};
  std::vector<COLOR::ColorMapPtrWrapper> m_rotatingLowColorMaps{};
  std::vector<uint32_t> m_rotatingDotNums{};
  static constexpr uint32_t NUM_ROTATING_COLOR_STEPS = 100U;
  UTILS::TValue m_rotatingColorsT{
      {UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_ROTATING_COLOR_STEPS}
  };
  auto UpdateRotatingColorMaps() noexcept -> void;

  enum class GridColorRange : UnderlyingEnumType
  {
    ONE,
    LOW,
    MEDIUM,
    HIGH,
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
      -> std::vector<COLOR::ColorMapPtrWrapper>;
  [[nodiscard]] auto GetVerticalMainColorMaps() const noexcept
      -> std::vector<COLOR::ColorMapPtrWrapper>;
  [[nodiscard]] auto GetHorizontalLowColorMaps() const noexcept
      -> std::vector<COLOR::ColorMapPtrWrapper>;
  [[nodiscard]] auto GetVerticalLowColorMaps() const noexcept
      -> std::vector<COLOR::ColorMapPtrWrapper>;
  [[nodiscard]] auto GetAllDotColorMaps(const COLOR::WeightedRandomColorMaps& baseRandomColorMaps)
      const noexcept -> std::vector<COLOR::ColorMapPtrWrapper>;

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
}

} // namespace GOOM::VISUAL_FX::CIRCLES

namespace GOOM::VISUAL_FX::CIRCLES
{

using COLOR::ColorMapPtrWrapper;
using COLOR::ColorMapsGrid;
using COLOR::GetBrighterColor;
using COLOR::GetUnweightedRandomColorMaps;
using COLOR::WeightedRandomColorMaps;
using DRAW::MultiplePixels;
using DRAW::ReversePixels;
using UTILS::EnumMap;
using UTILS::TValue;
using UTILS::MATH::AngleParams;
using UTILS::MATH::CirclePath;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::ModDecrement;
using UTILS::MATH::ModIncrement;
using UTILS::MATH::OscillatingFunction;

static constexpr auto DOT_MAIN_COLOR_BRIGHTNESS_FACTOR     = 0.1F;
static constexpr auto DOT_LOW_COLOR_BRIGHTNESS_FACTOR      = 2.0F;
static constexpr auto DOT_ROTATING_COLOR_BRIGHTNESS_FACTOR = 5.0F;
static constexpr auto LINE_MAIN_COLOR_BRIGHTNESS_FACTOR    = 0.1F;
static constexpr auto LINE_LOW_COLOR_BRIGHTNESS_FACTOR     = 1.0F;
static constexpr auto NEAR_START_BRIGHTNESS_FACTOR         = 0.25F;
static constexpr auto NEAR_END_BRIGHTNESS_VALUE            = 0.001F;
static constexpr auto SPECIAL_NUM_BRIGHTNESS_INCREASE      = 1.5F;
static constexpr auto SPECIAL_NUM_LINE_BRIGHTNESS_INCREASE = 2.0F;
static constexpr auto MIN_COLOR_GRID_MIX_T                 = 0.1F;
static constexpr auto MAX_COLOR_GRID_MIX_T                 = 0.9F;
static constexpr auto PROB_NO_ROTATING_COLORS              = 0.9F;
static constexpr auto PROB_DOT_ROTATE_INCREMENT            = 0.5F;
static constexpr auto MIN_NUM_ROTATING_COLORS              = 1U;
static constexpr auto MAX_NUM_ROTATING_COLORS              = 2U;
static constexpr auto CLOSE_TO_START_T                     = 0.1F;
static constexpr auto CLOSE_TO_END_T                       = 0.2F;

static constexpr auto PROB_SHOW_LINE                     = 0.5F;
static constexpr auto PROB_ALTERNATE_MAIN_LOW_DOT_COLORS = 0.5F;

static constexpr auto GRID_COLOR_RANGE_ONE_WEIGHT    = 20.0F;
static constexpr auto GRID_COLOR_RANGE_LOW_WEIGHT    = 03.0F;
static constexpr auto GRID_COLOR_RANGE_MEDIUM_WEIGHT = 02.0F;
static constexpr auto GRID_COLOR_RANGE_HIGH_WEIGHT   = 01.0F;

static constexpr auto CIRCLE_NOISE_RADIUS     = 10U;
static constexpr auto NUM_CIRCLE_NOISE_PIXELS = 5U;

Circle::CircleDots::CircleDots(const IGoomRand& goomRand,
                               const Helper& helper,
                               const Params& circleParams,
                               const UTILS::MATH::OscillatingFunction::Params& pathParams,
                               const uint32_t numDots) noexcept
  : m_goomRand{&goomRand},
    m_helper{&helper},
    m_circleParams{circleParams},
    m_pathParams{pathParams},
    m_numDots{numDots},
    m_dotPaths{GetNewDotPaths()},
    m_dotDiameters{GetNewDotDiameters()},
    m_tLineColorStep{GetNewTLineColorStep()},
    m_numMaps{GetNewNumMaps()}
{
  Ensures(UTILS::MATH::IsEven(m_numDots));
  Ensures(m_dotPaths.GetNumDots() == m_numDots);
}

inline auto Circle::CircleDots::ResetNumDots(const uint32_t numDots) noexcept -> void
{
  m_numDots        = numDots;
  m_dotPaths       = GetNewDotPaths();
  m_dotDiameters   = GetNewDotDiameters();
  m_tLineColorStep = GetNewTLineColorStep();
  m_numMaps        = GetNewNumMaps();

  Ensures(m_dotPaths.GetNumDots() == m_numDots);
}

auto Circle::CircleDots::SetPathParams(
    const UTILS::MATH::OscillatingFunction::Params& pathParams) noexcept -> void
{
  m_pathParams = pathParams;
  m_dotPaths.SetPathParams({m_pathParams, m_pathParams});
}

inline auto Circle::CircleDots::GetNewDotPaths() const noexcept -> DotPaths
{
  return {
      *m_goomRand,
      m_numDots,
      {GetDotStartingPositions(m_numDots,
        m_circleParams.toTargetParams.circleCentreStart,
        m_circleParams.toTargetParams.circleRadius),
        GetDotStartingPositions(m_numDots,
        m_circleParams.fromTargetParams.circleCentreStart,
        m_circleParams.fromTargetParams.circleRadius)},
      {m_circleParams.toTargetParams.circleCentreTarget,
        m_circleParams.fromTargetParams.circleCentreTarget},
      {m_pathParams, m_pathParams},
  };
}

inline auto Circle::CircleDots::GetNewDotDiameters() const noexcept -> DotDiameters
{
  return {
      *m_goomRand, {m_numDots, m_helper->minDotDiameter, m_helper->maxDotDiameter}
  };
}

inline auto Circle::CircleDots::GetNewNumMaps() const noexcept
    -> UTILS::EnumMap<GridColorRange, uint32_t>
{
  static constexpr auto LOW_DIVISOR    = 6U;
  static constexpr auto MEDIUM_DIVISOR = 2U;

  return EnumMap<GridColorRange, uint32_t>{{{
      {GridColorRange::ONE, 1U},
      {GridColorRange::LOW, m_numDots / LOW_DIVISOR},
      {GridColorRange::MEDIUM, m_numDots / MEDIUM_DIVISOR},
      {GridColorRange::HIGH, m_numDots},
  }}};
}

inline auto Circle::CircleDots::GetNewTLineColorStep() const noexcept -> float
{
  Expects(m_numDots > 0);
  return 1.0F / static_cast<float>(m_numDots);
}

inline auto Circle::CircleDots::GetDotStartingPositions(const uint32_t numDots,
                                                        const Point2dInt& centre,
                                                        const float radius) noexcept
    -> std::vector<Point2dInt>
{
  const auto stepProperties = TValue::NumStepsProperties{TValue::StepType::SINGLE_CYCLE, numDots};
  auto positionT            = std::make_unique<TValue>(stepProperties);
  const auto centrePos      = ToVec2dFlt(centre);
  const auto path =
      std::make_unique<CirclePath>(std::move(positionT), centrePos, radius, AngleParams{});

  auto dotStartingPositions = std::vector<Point2dInt>(numDots);

  for (auto i = 0U; i < numDots; ++i)
  {
    dotStartingPositions.at(i) = path->GetNextPoint();

    path->IncrementT();
  }

  return dotStartingPositions;
}

Circle::Circle(FxHelper& fxHelper,
               const Helper& helper,
               const Params& circleParams,
               const OscillatingFunction::Params& pathParams) noexcept
  : m_fxHelper{&fxHelper},
    m_helper{helper},
    m_lineDrawer{fxHelper.GetDraw(), fxHelper.GetGoomRand(),
                 {CIRCLE_NOISE_RADIUS, NUM_CIRCLE_NOISE_PIXELS}},
    m_circleDots{std::make_unique<CircleDots>(m_fxHelper->GetGoomRand(),
                                              m_helper,
                                              circleParams,
                                              pathParams,
                                              GetNewNumDots())},
    m_dotDrawer{
        std::make_unique<DotDrawer>(fxHelper.GetDraw(), m_fxHelper->GetGoomRand(), m_helper)},
    m_mainColorMaps{GetUnweightedRandomColorMaps(m_fxHelper->GetGoomRand(), MAX_ALPHA)},
    m_lowColorMaps{GetUnweightedRandomColorMaps(m_fxHelper->GetGoomRand(), MAX_ALPHA)},
    m_weightedGridColorRanges{
        m_fxHelper->GetGoomRand(),
        {
            {GridColorRange::ONE, GRID_COLOR_RANGE_ONE_WEIGHT},
            {GridColorRange::LOW, GRID_COLOR_RANGE_LOW_WEIGHT},
            {GridColorRange::MEDIUM, GRID_COLOR_RANGE_MEDIUM_WEIGHT},
            {GridColorRange::HIGH, GRID_COLOR_RANGE_HIGH_WEIGHT},
        }
    }
{
  UpdateRotatingColorMaps();
}

inline auto Circle::GetNewNumDots() const noexcept -> uint32_t
{
  static_assert(UTILS::MATH::IsEven(MIN_NUM_DOTS));
  static_assert(UTILS::MATH::IsEven(MAX_NUM_DOTS));

  auto numDots = m_fxHelper->GetGoomRand().GetRandInRange(MIN_NUM_DOTS, MAX_NUM_DOTS + 1);
  if (not UTILS::MATH::IsEven(numDots))
  {
    numDots += 1;
  }
  return numDots;
}

inline auto Circle::ResetNumDots() noexcept -> void
{
  if (not HasPositionTJustHitEndBoundary())
  {
    m_resetNumDotsRequired = true;
    return;
  }

  DoResetNumDots();
  m_resetNumDotsRequired = false;
}

inline auto Circle::DoResetNumDots() noexcept -> void
{
  m_circleDots->ResetNumDots(GetNewNumDots());
  m_dotRotateOffset = 0U;

  m_mainColorMapsGrid = GetMainColorMapsGrid();
  m_lowColorMapsGrid  = GetLowColorMapsGrid();
  UpdateRotatingColorMaps();
}

inline auto Circle::GetColorMixT([[maybe_unused]] const float tX,
                                 [[maybe_unused]] const float tY) const noexcept -> float
{
  return m_currentColorGridMixT;
}

inline auto Circle::UpdateNumDifferentGridMaps() noexcept -> void
{
  if (static constexpr auto NUM_UPDATE_SKIPS = 5U;
      (m_fxHelper->GetGoomTime().GetCurrentTime() % NUM_UPDATE_SKIPS) != 0)
  {
    return;
  }

  m_currentGridColorRange = m_weightedGridColorRanges.GetRandomWeighted();
  m_numDifferentGridMaps  = m_circleDots->GetNumMaps()[m_currentGridColorRange];
}

inline auto Circle::GetMainColorMapsGrid() const noexcept -> ColorMapsGrid
{
  return {GetHorizontalMainColorMaps(),
          m_circleDots->GetDotPaths().GetPositionTRef(),
          GetVerticalMainColorMaps(),
          [this](const float tX, const float tY) { return GetColorMixT(tX, tY); }};
}

inline auto Circle::GetLowColorMapsGrid() const noexcept -> ColorMapsGrid
{
  return {GetHorizontalLowColorMaps(),
          m_circleDots->GetDotPaths().GetPositionTRef(),
          GetVerticalLowColorMaps(),
          [this](const float tX, const float tY) { return GetColorMixT(tX, tY); }};
}

inline auto Circle::GetHorizontalMainColorMaps() const noexcept -> std::vector<ColorMapPtrWrapper>
{
  return {m_mainColorMaps.GetRandomColorMap()};
}

inline auto Circle::GetVerticalMainColorMaps() const noexcept -> std::vector<ColorMapPtrWrapper>
{
  return GetAllDotColorMaps(m_mainColorMaps);
}

inline auto Circle::GetHorizontalLowColorMaps() const noexcept -> std::vector<ColorMapPtrWrapper>
{
  return {m_lowColorMaps.GetRandomColorMap()};
}

inline auto Circle::GetVerticalLowColorMaps() const noexcept -> std::vector<ColorMapPtrWrapper>
{
  return GetAllDotColorMaps(m_lowColorMaps);
}

auto Circle::UpdateRotatingColorMaps() noexcept -> void
{
  const auto newNumRotatingColors = m_fxHelper->GetGoomRand().ProbabilityOf(PROB_NO_ROTATING_COLORS)
                                        ? 0U
                                        : m_fxHelper->GetGoomRand().GetRandInRange(
                                              MIN_NUM_ROTATING_COLORS, MAX_NUM_ROTATING_COLORS + 1);

  if (m_numRotatingColors == newNumRotatingColors)
  {
    return;
  }
  m_numRotatingColors = newNumRotatingColors;

  m_dotRotateIncrement = m_fxHelper->GetGoomRand().ProbabilityOf(PROB_DOT_ROTATE_INCREMENT);

  m_rotatingDotNums.resize(m_numRotatingColors);
  for (auto& dotNum : m_rotatingDotNums)
  {
    dotNum = m_fxHelper->GetGoomRand().GetRandInRange(0U, m_circleDots->GetNumDots());
  }

  m_rotatingMainColorMaps.resize(m_numRotatingColors, ColorMapPtrWrapper{nullptr});
  for (auto& colorMap : m_rotatingMainColorMaps)
  {
    colorMap = m_mainColorMaps.GetRandomColorMap();
  }

  m_rotatingLowColorMaps.resize(m_numRotatingColors, ColorMapPtrWrapper{nullptr});
  for (auto& colorMap : m_rotatingLowColorMaps)
  {
    colorMap = m_lowColorMaps.GetRandomColorMap();
  }
}

auto Circle::GetAllDotColorMaps(const WeightedRandomColorMaps& baseRandomColorMaps) const noexcept
    -> std::vector<ColorMapPtrWrapper>
{
  auto differentMaps = std::vector(m_numDifferentGridMaps, ColorMapPtrWrapper{nullptr});
  for (auto& map : differentMaps)
  {
    map = baseRandomColorMaps.GetRandomColorMap();
  }

  auto dotColorMaps = std::vector(m_circleDots->GetNumDots(), ColorMapPtrWrapper{nullptr});

  auto start = 0U;
  for (auto k = 0U; k < m_numDifferentGridMaps; ++k)
  {
    for (auto i = start; i < m_circleDots->GetNumDots(); i += m_numDifferentGridMaps)
    {
      dotColorMaps.at(i) = differentMaps.at(k);
    }
    ++start;
  }

  return dotColorMaps;
}

auto Circle::SetWeightedColorMaps(const WeightedRandomColorMaps& weightedMainMaps,
                                  const WeightedRandomColorMaps& weightedLowMaps) noexcept -> void
{
  m_mainColorMaps = weightedMainMaps;
  m_lowColorMaps  = weightedLowMaps;

  UpdateRotatingColorMaps();

  UpdateNumDifferentGridMaps();
  m_currentColorGridMixT =
      m_fxHelper->GetGoomRand().GetRandInRange(MIN_COLOR_GRID_MIX_T, MAX_COLOR_GRID_MIX_T);
  m_mainColorMapsGrid.SetColorMaps(GetHorizontalMainColorMaps(), GetVerticalMainColorMaps());
  m_lowColorMapsGrid.SetColorMaps(GetHorizontalLowColorMaps(), GetVerticalLowColorMaps());

  m_linesMainColorMap = GetHorizontalMainColorMaps().at(0);
  m_linesLowColorMap  = GetHorizontalLowColorMaps().at(0);

  m_showLine = m_fxHelper->GetGoomRand().ProbabilityOf(PROB_SHOW_LINE);

  m_circleDots->GetDotDiameters().ChangeDiameters();
  m_alternateMainLowDotColors =
      m_fxHelper->GetGoomRand().ProbabilityOf(PROB_ALTERNATE_MAIN_LOW_DOT_COLORS);
  m_dotDrawer->SetWeightedColorMaps(weightedMainMaps);
}

auto Circle::SetPathParams(const OscillatingFunction::Params& pathParams) noexcept -> void
{
  ResetNumDots();
  m_circleDots->SetPathParams(pathParams);
}

auto Circle::SetGlobalBrightnessFactor(const float val) noexcept -> void
{
  m_globalBrightnessFactor = val;
  m_dotDrawer->SetGlobalBrightnessFactor(m_globalBrightnessFactor);
}

auto Circle::Start() noexcept -> void
{
  m_circleDots->GetDotPaths().ChangeDirection(DotPaths::Direction::TO_TARGET);
}

auto Circle::UpdateAndDraw() noexcept -> void
{
  DrawNextCircle();

  if (m_resetNumDotsRequired)
  {
    ResetNumDots();
  }
}

inline auto Circle::DrawNextCircle() noexcept -> void
{
  DrawNextCircleDots();
  ResetNumSteps();
}

auto Circle::ResetNumSteps() noexcept -> void
{
  if (0 == m_newNumSteps)
  {
    return;
  }
  if (not HasPositionTJustHitEndBoundary())
  {
    return;
  }
  m_circleDots->GetDotPaths().SetPositionTNumSteps(m_newNumSteps);
  m_newNumSteps = 0;
}

auto Circle::DrawNextCircleDots() noexcept -> void
{
  LogInfo("m_globalBrightnessFactor = {}", m_globalBrightnessFactor);

  const auto nextDotPositions = m_circleDots->GetDotPaths().GetNextDotPositions();

  const auto brightness     = GetCurrentBrightness();
  const auto dotBrightness  = GetDotBrightness(brightness);
  const auto lineBrightness = GetLineBrightness(brightness);
  const auto allDotColors   = GetAllDotColors();
  LogInfo("brightness = {}", brightness);
  LogInfo("dotBrightness = {}", dotBrightness);
  LogInfo("lineBrightness = {}", lineBrightness);

  auto rotateIndex     = m_dotRotateOffset;
  auto tLineColor      = 0.0F;
  auto prevDotPosition = nextDotPositions.at(m_circleDots->GetNumDots() - 1);
  for (auto i = 0U; i < m_circleDots->GetNumDots(); ++i)
  {
    const auto dotPosition = nextDotPositions.at(i);
    const auto dotDiameter = m_circleDots->GetDotDiameters().GetDiameters().at(rotateIndex);
    const auto dotColors   = GetSingleDotColors(rotateIndex, allDotColors, dotBrightness);
    LogInfo(
        "corrected dotColors low = {},{},{}", dotColors[1].R(), dotColors[1].G(), dotColors[1].B());

    m_dotDrawer->DrawDot(dotPosition, dotDiameter, dotColors);

    DrawLine(prevDotPosition, dotPosition, lineBrightness, tLineColor);

    prevDotPosition = dotPosition;
    tLineColor += m_circleDots->GetTLineColorStep();
    rotateIndex = m_dotRotateIncrement ? ModIncrement(rotateIndex, m_circleDots->GetNumDots())
                                       : ModDecrement(rotateIndex, m_circleDots->GetNumDots());
  }

  m_dotRotateOffset = m_dotRotateIncrement
                          ? ModIncrement(m_dotRotateOffset, m_circleDots->GetNumDots())
                          : ModDecrement(m_dotRotateOffset, m_circleDots->GetNumDots());
  m_rotatingColorsT.Increment();
}

inline auto Circle::GetAllDotColors() const noexcept -> AllDotColors
{
  return {m_mainColorMapsGrid.GetCurrentHorizontalLineColors(),
          m_lowColorMapsGrid.GetCurrentHorizontalLineColors()};
}

inline auto Circle::GetSingleDotColors(const uint32_t dotNum,
                                       const AllDotColors& allDotColors,
                                       const float dotBrightness) const noexcept -> MultiplePixels
{
  for (auto i = 0U; i < m_numRotatingColors; ++i)
  {
    if (dotNum == m_rotatingDotNums.at(i))
    {
      return {m_rotatingMainColorMaps.at(i).GetColor(m_rotatingColorsT()),
              GetBrighterColor(DOT_ROTATING_COLOR_BRIGHTNESS_FACTOR * dotBrightness,
                               m_rotatingLowColorMaps.at(i).GetColor(m_rotatingColorsT()))};
    }
  }

  const auto dotMainColor = allDotColors.mainColors.at(dotNum);
  const auto dotLowColor  = allDotColors.lowColors.at(dotNum);
  return {GetCorrectedColor(DOT_MAIN_COLOR_BRIGHTNESS_FACTOR * dotBrightness, dotMainColor),
          GetCorrectedColor(DOT_LOW_COLOR_BRIGHTNESS_FACTOR * dotBrightness, dotLowColor)};
}

inline auto Circle::GetCurrentBrightness() const noexcept -> float
{
  return m_circleDots->GetDotPaths().IsCloseToEndBoundary(CLOSE_TO_END_T)
             ? NEAR_END_BRIGHTNESS_VALUE
             : m_globalBrightnessFactor;
}

inline auto Circle::GetDotBrightness(const float brightness) const noexcept -> float
{
  if (not m_circleDots->GetDotPaths().IsCloseToStartBoundary(CLOSE_TO_START_T))
  {
    if (IsSpecialUpdateNum())
    {
      return SPECIAL_NUM_BRIGHTNESS_INCREASE * brightness;
    }
    return brightness;
  }

  return NEAR_START_BRIGHTNESS_FACTOR * brightness;
}

inline auto Circle::IsSpecialUpdateNum() const noexcept -> bool
{
  static constexpr auto SPECIAL_UPDATE_MULTIPLE = 5U;
  return 0 == (m_fxHelper->GetGoomTime().GetCurrentTime() % SPECIAL_UPDATE_MULTIPLE);
}

inline auto Circle::IsSpecialLineUpdateNum() const noexcept -> bool
{
  if (m_circleDots->GetDotPaths().IsCloseToStartBoundary(CLOSE_TO_START_T))
  {
    return true;
  }
  static constexpr auto LINE_UPDATE_MULTIPLE = 8U;
  return 0 == (m_fxHelper->GetGoomTime().GetCurrentTime() % LINE_UPDATE_MULTIPLE);
}

inline auto Circle::GetLineBrightness(const float brightness) const noexcept -> float
{
  return IsSpecialLineUpdateNum() ? (SPECIAL_NUM_LINE_BRIGHTNESS_INCREASE * brightness)
                                  : brightness;
}

inline auto Circle::GetCorrectedColor(const float brightness, const Pixel& color) const noexcept
    -> Pixel
{
  LogInfo("corrected color brightness = {}", brightness);
  return m_colorAdjustment.GetAdjustment(brightness, color);
}

inline auto Circle::DrawLine(const Point2dInt& position1,
                             const Point2dInt& position2,
                             const float lineBrightness,
                             const float tLineColor) noexcept -> void
{
  const auto lastTDotColor = DrawLineDots(lineBrightness, position1, position2, tLineColor);
  DrawConnectingLine(position1, position2, lineBrightness, lastTDotColor);
}

inline auto Circle::DrawLineDots(const float lineBrightness,
                                 const Point2dInt& position1,
                                 const Point2dInt& position2,
                                 const float tLineColor) noexcept -> float
{
  static constexpr auto NUM_LINE_DOTS = 5U;

  static constexpr auto T_DOT_POS_STEP = 1.0F / static_cast<float>(NUM_LINE_DOTS);
  auto tDotPos                         = T_DOT_POS_STEP;

  const auto tDotColorStep = m_circleDots->GetTLineColorStep() / static_cast<float>(NUM_LINE_DOTS);

  auto tDotColor = tLineColor + tDotColorStep;
  for (auto i = 0U; i < (NUM_LINE_DOTS - 1); ++i)
  {
    const auto dotPos    = lerp(position1, position2, tDotPos);
    const auto mainColor = GetCorrectedColor(LINE_MAIN_COLOR_BRIGHTNESS_FACTOR * lineBrightness,
                                             m_linesMainColorMap.GetColor(tDotColor));
    const auto lowColor  = GetCorrectedColor(LINE_LOW_COLOR_BRIGHTNESS_FACTOR * lineBrightness,
                                            m_linesLowColorMap.GetColor(tDotColor));

    DrawDot(i, dotPos, {mainColor, lowColor});

    tDotPos += T_DOT_POS_STEP;
    tDotColor += tDotColorStep;
  }

  return tDotColor;
}

inline auto Circle::DrawDot(const uint32_t dotNum,
                            const Point2dInt& pos,
                            const MultiplePixels& colors) noexcept -> void
{
  if (m_alternateMainLowDotColors and UTILS::MATH::IsEven(dotNum))
  {
    m_dotDrawer->DrawDot(pos, m_helper.lineDotDiameter, ReversePixels(colors));
  }
  else
  {
    m_dotDrawer->DrawDot(pos, m_helper.lineDotDiameter, colors);
  }
}

auto Circle::DrawConnectingLine(const Point2dInt& position1,
                                const Point2dInt& position2,
                                const float lineBrightness,
                                const float tDotColor) noexcept -> void
{
  if ((not m_circleDots->GetDotPaths().IsCloseToStartBoundary(CLOSE_TO_START_T)) and
      ((not m_showLine) or (not IsSpecialLineUpdateNum())))
  {
    return;
  }

  const auto mainColor = GetCorrectedColor(LINE_MAIN_COLOR_BRIGHTNESS_FACTOR * lineBrightness,
                                           m_linesMainColorMap.GetColor(tDotColor));
  const auto lowColor  = GetCorrectedColor(LINE_LOW_COLOR_BRIGHTNESS_FACTOR * lineBrightness,
                                          m_linesLowColorMap.GetColor(tDotColor));
  LogInfo("corrected lowColor = {},{},{}", lowColor.R(), lowColor.G(), lowColor.B());


  m_lineDrawer.DrawLine(position1, position2, {mainColor, lowColor});
}

} // namespace GOOM::VISUAL_FX::CIRCLES
