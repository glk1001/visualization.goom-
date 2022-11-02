#include "distance_field.h"

//#undef NO_LOGGING

#include "filter_fx/common_types.h"
#include "goom_config.h"
#include "goom_types.h"
#include "logging.h"
#include "utils/enum_utils.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

#include <algorithm>
#include <limits>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::EnumToString;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::Logging; // NOLINT(misc-unused-using-decls)
using UTILS::NameValuePairs;
using UTILS::MATH::HALF;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::IsEven;
using UTILS::MATH::Sq;
using UTILS::MATH::U_HALF;

static const auto DEFAULT_GRID_POINT_ARRAY      = DistanceField::GridPointsWithCentres{};
static const auto DEFAULT_GRID_POINT_CENTRE_MAP = DistanceField::GridPointMap{};

static constexpr auto DEFAULT_GRID_WIDTH     = 4U;
static constexpr auto GRID_WIDTH_RANGE_MODE0 = IGoomRand::NumberRange<uint32_t>{4U, 16U};
static constexpr auto GRID_WIDTH_RANGE_MODE1 = IGoomRand::NumberRange<uint32_t>{16U, 32U};
static constexpr auto GRID_WIDTH_RANGE_MODE2 = IGoomRand::NumberRange<uint32_t>{32U, 64U};

static constexpr auto DEFAULT_GRID_TYPE = DistanceField::GridType::FULL;
static constexpr auto DEFAULT_GRID_SCALE =
    static_cast<float>(DEFAULT_GRID_WIDTH) / NormalizedCoords::COORD_WIDTH;
static constexpr auto DEFAULT_CELL_CENTRE = HALF / DEFAULT_GRID_SCALE;

static constexpr auto DEFAULT_AMPLITUDE     = 0.1F;
static constexpr auto AMPLITUDE_RANGE_MODE0 = AmplitudeRange{
    {0.05F, 1.101F},
    {0.05F, 1.101F},
};
static constexpr auto AMPLITUDE_RANGE_MODE1 = AmplitudeRange{
    {0.50F, 2.01F},
    {0.50F, 2.01F},
};
static constexpr auto AMPLITUDE_RANGE_MODE2 = AmplitudeRange{
    {0.70F, 3.01F},
    {0.70F, 3.01F},
};
static constexpr auto FULL_AMPLITUDE_FACTOR            = 2.0F;
static constexpr auto PARTIAL_DIAMOND_AMPLITUDE_FACTOR = 0.1F;
static constexpr auto PARTIAL_SQUARE_AMPLITUDE_FACTOR = 0.1F;

static constexpr auto PROB_XY_AMPLITUDES_EQUAL = 0.50F;
static constexpr auto PROB_RANDOM_CENTRE       = 0.1F;

static constexpr auto GRID_TYPE_FULL_WEIGHT            = 100.0F;
static constexpr auto GRID_TYPE_PARTIAL_X_WEIGHT       = 10.0F;
static constexpr auto GRID_TYPE_PARTIAL_DIAMOND_WEIGHT = 10.0F;
static constexpr auto GRID_TYPE_PARTIAL_SQUARE_WEIGHT  = 10000000.0F;
static constexpr auto GRID_TYPE_PARTIAL_RANDOM_WEIGHT  = 10.0F;

DistanceField::DistanceField(const Modes mode, const IGoomRand& goomRand) noexcept
  : m_mode{mode},
    m_goomRand{goomRand},
    m_weightedEffects{
        goomRand,
        {
            {    GridType::FULL,        GRID_TYPE_FULL_WEIGHT},
            {    GridType::PARTIAL_X,   GRID_TYPE_PARTIAL_X_WEIGHT},
            {GridType::PARTIAL_DIAMOND, GRID_TYPE_PARTIAL_DIAMOND_WEIGHT},
            {GridType::PARTIAL_SQUARE,  GRID_TYPE_PARTIAL_SQUARE_WEIGHT},
            {GridType::PARTIAL_RANDOM,  GRID_TYPE_PARTIAL_RANDOM_WEIGHT},
        }
    },
    m_params{
        {DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE},
        DEFAULT_GRID_TYPE,
        DEFAULT_GRID_WIDTH,
        DEFAULT_GRID_SCALE,
        DEFAULT_CELL_CENTRE,
        {DEFAULT_GRID_POINT_ARRAY, DEFAULT_GRID_POINT_CENTRE_MAP},
    }
{
}

auto DistanceField::SetRandomParams() noexcept -> void
{
  if (m_mode == Modes::MODE0)
  {
    SetMode0RandomParams();
  }
  else if (m_mode == Modes::MODE1)
  {
    SetMode1RandomParams();
  }
  else
  {
    SetMode2RandomParams();
  }
}

auto DistanceField::SetMode0RandomParams() noexcept -> void
{
  SetRandomParams(AMPLITUDE_RANGE_MODE0, GRID_WIDTH_RANGE_MODE0);
}

auto DistanceField::SetMode1RandomParams() noexcept -> void
{
  SetRandomParams(AMPLITUDE_RANGE_MODE1, GRID_WIDTH_RANGE_MODE1);
}

auto DistanceField::SetMode2RandomParams() noexcept -> void
{
  SetRandomParams(AMPLITUDE_RANGE_MODE2, GRID_WIDTH_RANGE_MODE2);
}

auto DistanceField::SetRandomParams(const AmplitudeRange& amplitudeRange,
                                    const GridWidthRange& gridWidthRange) noexcept -> void
{
  const auto gridType   = m_weightedEffects.GetRandomWeighted();
  const auto gridWidth  = GetGridWidth(gridType, gridWidthRange);
  const auto gridScale  = static_cast<float>(gridWidth) / NormalizedCoords::COORD_WIDTH;
  const auto cellCentre = HALF / gridScale;
  const auto gridArrays = GetGridsArray(gridType, gridWidth);

  const auto amplitude = GetAmplitude(amplitudeRange, gridType, gridWidth, gridArrays);

  SetParams({
      amplitude,
      gridType,
      static_cast<int32_t>(gridWidth - 1),
      gridScale,
      cellCentre,
      gridArrays,
  });

  Ensures(GetZoomInCoefficientsViewport().GetViewportWidth() == NormalizedCoords::COORD_WIDTH);
}

auto DistanceField::GetGridWidth(const GridType gridType,
                                 const GridWidthRange& gridWidthRange) const noexcept -> uint32_t
{
  const auto gridWidth = m_goomRand.GetRandInRange(gridWidthRange);

  if ((gridType == GridType::PARTIAL_DIAMOND) and IsEven(gridWidth))
  {
    return gridWidth + 1;
  }

  return gridWidth;
}

auto DistanceField::GetGridsArray(const GridType gridType, const uint32_t gridWidth) const noexcept
    -> Params::GridArrays
{
  if (gridType == GridType::FULL)
  {
    return {DEFAULT_GRID_POINT_ARRAY, DEFAULT_GRID_POINT_CENTRE_MAP};
  }

  const auto gridPointArray      = GetGridPointsWithCentres(gridType, gridWidth);
  const auto gridPointCentresMap = GetGridPointCentresMap(gridWidth, gridPointArray);

  return {gridPointArray, gridPointCentresMap};
}

auto DistanceField::GetGridPointsWithCentres(const GridType gridType,
                                             const uint32_t gridWidth) const noexcept
    -> GridPointsWithCentres
{
  if (gridType == GridType::PARTIAL_X)
  {
    return GetGridPointXArray(gridWidth);
  }
  if (gridType == GridType::PARTIAL_DIAMOND)
  {
    return GetGridPointDiamondArray(gridWidth);
  }
  if (gridType == GridType::PARTIAL_SQUARE)
  {
    return GetGridPointSquareArray(gridWidth);
  }
  if (gridType == GridType::PARTIAL_RANDOM)
  {
    return GetGridPointRandomArray(gridWidth);
  }

  FailFast();
  return {};
}

auto DistanceField::GetGridPointXArray(const uint32_t gridWidth) noexcept -> GridPointsWithCentres
{
  auto gridPointArray = GridPointsWithCentres{};

  for (auto y = 0U; y < gridWidth; ++y)
  {
    gridPointArray.emplace_back(y, y);
    gridPointArray.emplace_back(y, gridWidth - y - 1);
  }

  return gridPointArray;
}

auto DistanceField::GetGridPointDiamondArray(const uint32_t gridWidth) noexcept
    -> GridPointsWithCentres
{
  auto gridPointArray = GridPointsWithCentres{};

  gridPointArray.emplace_back(0U, U_HALF * gridWidth - 1);
  gridPointArray.emplace_back(U_HALF * gridWidth - 1, gridWidth - 1);
  gridPointArray.emplace_back(gridWidth - 1, U_HALF * gridWidth - 1);
  gridPointArray.emplace_back(U_HALF * gridWidth - 1, 0U);

  return gridPointArray;
}

auto DistanceField::GetGridPointSquareArray(const uint32_t gridWidth) noexcept
    -> GridPointsWithCentres
{
  auto gridPointArray = GridPointsWithCentres{};

  const auto start = gridWidth <= 2U ? 0U : 1U;
  const auto finish = gridWidth <= 2U ? gridWidth : gridWidth - 1;

  for (auto y = start; y < finish; ++y)
  {
    gridPointArray.emplace_back(0U, y);
    gridPointArray.emplace_back(gridWidth - 1, y);
  }
  for (auto x = start + 1U; x < finish - 1U; ++x)
  {
    gridPointArray.emplace_back(x, 0U);
    gridPointArray.emplace_back(x, gridWidth - 1);
  }

  return gridPointArray;
}

auto DistanceField::GetGridPointRandomArray(const uint32_t gridWidth) const noexcept
    -> GridPointsWithCentres
{
  auto gridPointArray = GridPointsWithCentres{};

  for (auto y = 0U; y < gridWidth; ++y)
  {
    for (auto x = 0U; x < gridWidth; ++x)
    {
      if (m_goomRand.ProbabilityOf(PROB_RANDOM_CENTRE))
      {
        gridPointArray.emplace_back(x, y);
      }
    }
  }

  return gridPointArray;
}

auto DistanceField::GetGridPointCentresMap(
    const uint32_t gridWidth, const GridPointsWithCentres& gridPointsWithCentres) noexcept
    -> GridPointMap
{
  auto gridPointCentresMap = GridPointMap(gridWidth);

  for (auto y = 0U; y < gridWidth; ++y)
  {
    gridPointCentresMap[y].resize(gridWidth);
    for (auto x = 0U; x < gridWidth; ++x)
    {
      gridPointCentresMap[y][x] = FindNearestGridPointsWithCentres({x, y}, gridPointsWithCentres);
    }
  }

  return gridPointCentresMap;
}

inline auto DistanceField::FindNearestGridPointsWithCentres(
    const Point2dInt& gridPoint, const GridPointsWithCentres& gridPointsWithCentres) noexcept
    -> GridCentresList
{
  auto minDistancePoints = GridCentresList{};
  auto minDistanceSq     = std::numeric_limits<int32_t>::max();

  for (const auto& centrePoint : gridPointsWithCentres)
  {
    const auto distanceSq = SqDistance(gridPoint, centrePoint);
    if (distanceSq < minDistanceSq)
    {
      minDistanceSq     = distanceSq;
      minDistancePoints = GridCentresList{1, centrePoint};
    }
    else if (distanceSq == minDistanceSq)
    {
      minDistancePoints.emplace_back(centrePoint);
    }
  }

  Ensures(not minDistancePoints.empty());

  return minDistancePoints;
}

inline auto DistanceField::GetAmplitude(const AmplitudeRange& amplitudeRange,
                                        const GridType gridType,
                                        const uint32_t gridWidth,
                                        const Params::GridArrays& gridArrays) const noexcept
    -> Amplitude
{
  const auto xAmplitude = m_goomRand.GetRandInRange(amplitudeRange.xRange);
  const auto yAmplitude = m_goomRand.ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL)
                              ? xAmplitude
                              : m_goomRand.GetRandInRange(amplitudeRange.yRange);

  const auto amplitudeFactor = GetAmplitudeFactor(gridType, gridWidth, gridArrays);

  return {amplitudeFactor * xAmplitude, amplitudeFactor * yAmplitude};
}

inline auto DistanceField::GetAmplitudeFactor(const GridType gridType,
                                              const uint32_t gridWidth,
                                              const Params::GridArrays& gridArrays) noexcept
    -> float
{
  if (gridType == GridType::PARTIAL_X)
  {
    return 1.0F + std::log2(static_cast<float>(gridWidth));
  }
  if (gridType == GridType::PARTIAL_DIAMOND)
  {
    return PARTIAL_DIAMOND_AMPLITUDE_FACTOR;
  }
  if (gridType == GridType::PARTIAL_SQUARE)
  {
    return PARTIAL_SQUARE_AMPLITUDE_FACTOR;
  }
  if (gridType == GridType::PARTIAL_RANDOM)
  {
    return std::log2(static_cast<float>(gridArrays.gridPointsWithCentres.size()));
  }

  return FULL_AMPLITUDE_FACTOR * std::log2(static_cast<float>(gridWidth));
}

auto DistanceField::GetDistanceSquaredFromClosestPoint(const NormalizedCoords& point) const noexcept
    -> float
{
  const auto gridPoint = GetCorrespondingGridPoint(point);

  if (m_params.gridType == GridType::FULL)
  {
    const auto normalizedGridCentre = GetNormalizedGridPointCentre(gridPoint);
    return GetSqDistance(point, normalizedGridCentre);
  }

  const auto gridPointsWithCentres = GetNearsetGridPointsWithCentres(gridPoint);
  const auto normalizedGridCentres = GetNormalizedGridPointCentres(gridPointsWithCentres);

  return GetMinDistanceSquared(point, normalizedGridCentres);
}

inline auto DistanceField::GetCorrespondingGridPoint(const NormalizedCoords& point) const noexcept
    -> Point2dInt
{
  const auto x = static_cast<int32_t>(
      std::floor(m_params.gridScale * (point.GetX() - NormalizedCoords::MIN_COORD)));
  const auto y = static_cast<int32_t>(
      std::floor(m_params.gridScale * (point.GetY() - NormalizedCoords::MIN_COORD)));

  return {std::clamp(x, 0, m_params.gridMax), std::clamp(y, 0, m_params.gridMax)};
}

inline auto DistanceField::GetNearsetGridPointsWithCentres(
    const Point2dInt& gridPoint) const noexcept -> GridCentresList
{
  if (m_params.gridType == GridType::FULL)
  {
    return GridCentresList{1, gridPoint};
  }

  const auto x = static_cast<uint32_t>(gridPoint.x);
  const auto y = static_cast<uint32_t>(gridPoint.y);

  return m_params.gridArrays.gridPointCentresMap.at(y).at(x);
}

inline auto DistanceField::GetNormalizedGridPointCentres(
    const GridCentresList& gridPointsWithCentres) const noexcept -> std::vector<NormalizedCoords>
{
  auto normalizedCentres = std::vector<NormalizedCoords>{};

  for (const auto& gridPoint : gridPointsWithCentres)
  {
    normalizedCentres.emplace_back(GetNormalizedGridPointCentre(gridPoint));
  }

  return normalizedCentres;
}

inline auto DistanceField::GetNormalizedGridPointCentre(
    const Point2dInt& gridPointWithCentre) const noexcept -> NormalizedCoords
{
  return {(NormalizedCoords::MIN_COORD +
           (static_cast<float>(gridPointWithCentre.x) / m_params.gridScale)) +
              m_params.cellCentre,
          (NormalizedCoords::MIN_COORD +
           (static_cast<float>(gridPointWithCentre.y) / m_params.gridScale)) +
              m_params.cellCentre};
}

inline auto DistanceField::GetMinDistanceSquared(
    const NormalizedCoords& point, const std::vector<NormalizedCoords>& centres) noexcept -> float
{
  static constexpr auto MAX_DISTANCE_SQUARED = 1.0F + (2.0F * Sq(NormalizedCoords::COORD_WIDTH));
  auto minDistanceSquared                    = MAX_DISTANCE_SQUARED;

  for (const auto& centre : centres)
  {
    const auto distanceSquared = GetSqDistance(point, centre);
    if (distanceSquared < minDistanceSquared)
    {
      minDistanceSquared = distanceSquared;
    }
  }

  Ensures(minDistanceSquared < MAX_DISTANCE_SQUARED);

  return minDistanceSquared;
}

auto DistanceField::GetZoomInCoefficientsEffectNameValueParams() const noexcept -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({PARAM_GROUP, "Dist Field"});
  const auto gridWidth      = m_params.gridMax + 1;
  return {
      GetPair(fullParamGroup, "GridType", EnumToString(m_params.gridType)),
      GetPair(fullParamGroup, "GridWidth", gridWidth),
      GetPair(fullParamGroup,
              "Grid Centres",
              m_params.gridType == GridType::FULL
                  ? static_cast<size_t>(Sq(gridWidth))
                  : m_params.gridArrays.gridPointsWithCentres.size()),
      GetPair(fullParamGroup, "amplitude", Point2dFlt{m_params.amplitude.x, m_params.amplitude.y}),
  };
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
