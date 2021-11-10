#include "tentacle_driver.h"

#include "color/colormaps.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "goom/logging_control.h"
//#undef NO_LOGGING
#include "goom/logging.h"
#include "tentacles.h"
#include "utils/goom_rand_base.h"
#include "utils/mathutils.h"
#include "v2d.h"

#undef NDEBUG
#include <cassert>
#include <cmath>
#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace TENTACLES
{
#else
namespace GOOM::VISUAL_FX::TENTACLES
{
#endif

using COLOR::ColorMapGroup;
using COLOR::GetColorMultiply;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using UTILS::IGoomRand;
using UTILS::ImageBitmap;
using UTILS::m_half_pi;
using UTILS::m_pi;
using UTILS::SmallImageBitmaps;

const size_t TentacleDriver::CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES = 400;

TentacleDriver::TentacleDriver(IGoomDraw& draw,
                               IGoomRand& goomRand,
                               const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{draw},
    m_goomRand{goomRand},
    m_tentacles{m_goomRand},
    // clang-format off
    m_iterParamsGroups{
        {
            m_goomRand,
            {100, 0.600F, 1.0F, {1.5F, -10.0f, +10.0F, m_pi}, 100.0F},
            {125, 0.700F, 2.0F, {1.0F, -10.0F, +10.0F, 0.0F}, 105.0F},
        },
        {
            m_goomRand,
            {125, 0.700F, 0.5F, {1.0F, -10.0F, +10.0F, 0.0F}, 100.0},
            {150, 0.800F, 1.5F, {1.5F, -10.0F, +10.0F, m_pi}, 105.0},
        },
        {
            m_goomRand,
            {150, 0.800F, 1.5F, {1.5F, -10.0F, +10.0F, m_pi}, 100.0},
            {200, 0.900F, 2.5F, {1.0F, -10.0F, +10.0F, 0.0F}, 105.0},

        }},
    // clang-format on
    m_dotDrawer{m_draw, m_goomRand, smallBitmaps}
{
}

inline auto TentacleDriver::ChangeCurrentColorMapEvent() const -> bool
{
  return m_goomRand.ProbabilityOfMInN(3, 5);
}

void TentacleDriver::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps>& weightedMaps)
{
  m_colorMaps = weightedMaps;
}

void TentacleDriver::SetColorMode(const ColorModes m)
{
  m_colorMode = m;
}

constexpr double TENT2D_X_MIN = 0.0;
constexpr double TENT2D_Y_MIN = 0.065736;
constexpr double TENT2D_Y_MAX = 10000.0;

void TentacleDriver::Init(const ColorMapGroup initialColorMapGroup, const ITentacleLayout& layout)
{
  m_numTentacles = layout.GetNumPoints();

  m_tentacleParams.resize(m_numTentacles);

  constexpr V3dFlt INITIAL_HEAD_POS = {0, 0, 0};

  const size_t numInParamGroup = m_numTentacles / m_iterParamsGroups.size();
  const float tStep = 1.0F / static_cast<float>(numInParamGroup - 1);

  size_t paramsIndex = 0;
  float t = 0.0;
  for (size_t i = 0; i < m_numTentacles; ++i)
  {
    const IterParamsGroup paramsGrp = m_iterParamsGroups.at(paramsIndex);
    if (0 == (i % numInParamGroup))
    {
      if (paramsIndex < (m_iterParamsGroups.size() - 1))
      {
        ++paramsIndex;
      }
      t = 0.0;
    }
    const IterationParams params = paramsGrp.GetNext(t);
    t += tStep;
    m_tentacleParams[i] = params;

    std::shared_ptr<TentacleColorMapColorizer> colorizer{
        std::make_shared<TentacleColorMapColorizer>(initialColorMapGroup, params.numNodes,
                                                    m_goomRand)};
    m_colorizers.emplace_back(colorizer);

    std::unique_ptr<Tentacle2D> tentacle2D{CreateNewTentacle2D(i, params)};

    // To hide the annoying flapping tentacle head, make near the head very dark.
    const auto headColor = Pixel{5, 5, 5, MAX_ALPHA};
    const Pixel headLowColor = headColor;
    Tentacle3D tentacle{std::move(tentacle2D),
                        m_colorizers[m_colorizers.size() - 1],
                        headColor,
                        headLowColor,
                        INITIAL_HEAD_POS,
                        Tentacle2D::MIN_NUM_NODES,
                        m_goomRand};

    m_tentacles.AddTentacle(std::move(tentacle));
  }

  UpdateTentaclesLayout(layout);

  m_updateNum = 0;
}

auto TentacleDriver::IterParamsGroup::GetNext(const float t) const
    -> TentacleDriver::IterationParams
{
  const float prevYWeight =
      goomRand.GetRandInRange(0.9F, 1.1F) * stdnew::lerp(first.prevYWeight, last.prevYWeight, t);
  IterationParams params{};
  params.length = goomRand.GetRandInRange(1.0F, 1.1F * stdnew::lerp(first.length, last.length, t));
  assert(params.length >= 1.0F);
  params.numNodes = static_cast<size_t>(
      goomRand.GetRandInRange(0.9F, 1.1F) *
      stdnew::lerp(static_cast<float>(first.numNodes), static_cast<float>(last.numNodes), t));
  assert(params.numNodes >= 10);
  params.prevYWeight = prevYWeight;
  params.iterZeroYValWave = first.iterZeroYValWave;
  params.iterZeroYValWaveFreq =
      goomRand.GetRandInRange(0.9F, 1.1F) *
      stdnew::lerp(first.iterZeroYValWaveFreq, last.iterZeroYValWaveFreq, t);
  return params;
}

auto TentacleDriver::CreateNewTentacle2D(size_t id, const IterationParams& iterationParams)
    -> std::unique_ptr<Tentacle2D>
{
  const float tentacleLen =
      std::max(1.0F, m_goomRand.GetRandInRange(0.99F, 1.01F) * iterationParams.length);
  assert(tentacleLen >= 1);
  const double tent2d_xMax = TENT2D_X_MIN + static_cast<double>(tentacleLen);
  assert(tent2d_xMax >= 1.0);

  std::unique_ptr<Tentacle2D> tentacle{std::make_unique<Tentacle2D>(
      id, iterationParams.numNodes, TENT2D_X_MIN, tent2d_xMax, TENT2D_Y_MIN, TENT2D_Y_MAX,
      iterationParams.prevYWeight, 1.0F - iterationParams.prevYWeight)};

  tentacle->SetDoDamping(true);

  return tentacle;
}

void TentacleDriver::StartIterating()
{
  for (auto& t : m_tentacles)
  {
    t.Get2DTentacle().StartIterating();
  }
}

void TentacleDriver::UpdateTentaclesLayout(const ITentacleLayout& layout)
{
  assert(layout.GetNumPoints() == m_numTentacles);

  std::vector<size_t> sortedLongestFirst(m_numTentacles);
  std::iota(sortedLongestFirst.begin(), sortedLongestFirst.end(), 0);
  const auto compareByLength = [this](const size_t id1, const size_t id2)
  {
    const double len1 = m_tentacles[id1].Get2DTentacle().GetLength();
    const double len2 = m_tentacles[id2].Get2DTentacle().GetLength();
    // Sort by longest first.
    return len1 > len2;
  };
  std::sort(sortedLongestFirst.begin(), sortedLongestFirst.end(), compareByLength);

  for (size_t i = 0; i < m_numTentacles; ++i)
  {
    m_tentacles[sortedLongestFirst.at(i)].SetHead(layout.GetPoints().at(i));
  }

  // To help with perspective, any tentacles near vertical centre will be shortened.
  for (auto& tentacle : m_tentacles)
  {
    const V3dFlt& head = tentacle.GetHead();
    if (std::fabs(head.x) < 10.0F)
    {
      Tentacle2D& tentacle2D = tentacle.Get2DTentacle();
      const double xMin = tentacle2D.GetXMin();
      const double xMax = tentacle2D.GetXMax();
      const double newXMax = xMin + (1.0 * (xMax - xMin));
      tentacle2D.SetXDimensions(xMin, newXMax);
      tentacle.SetNumHeadNodes(
          std::max(6 * Tentacle2D::MIN_NUM_NODES, tentacle.Get2DTentacle().GetNumNodes() / 2));
    }
  }
}

void TentacleDriver::MultiplyIterZeroYValWaveFreq(const float val)
{
  for (size_t i = 0; i < m_numTentacles; ++i)
  {
    const float newFreq = val * m_tentacleParams[i].iterZeroYValWaveFreq;
    m_tentacleParams[i].iterZeroYValWave.SetFrequency(newFreq);
  }
}

void TentacleDriver::SetReverseColorMix(const bool val)
{
  for (auto& t : m_tentacles)
  {
    t.SetReverseColorMix(val);
  }
}

void TentacleDriver::UpdateIterTimers()
{
  for (auto* const t : m_iterTimers)
  {
    t->Next();
  }
}

auto TentacleDriver::GetNextColorMapGroups() const -> std::vector<ColorMapGroup>
{
  const size_t numDifferentGroups =
      ((m_colorMode == ColorModes::MINIMAL) || (m_colorMode == ColorModes::ONE_GROUP_FOR_ALL) ||
       m_goomRand.ProbabilityOfMInN(1, 100))
          ? 1
          : m_goomRand.GetRandInRange(1U, std::min(5U, static_cast<uint32_t>(m_colorizers.size())));
  std::vector<ColorMapGroup> groups(numDifferentGroups);
  for (size_t i = 0; i < numDifferentGroups; ++i)
  {
    groups[i] = m_colorMaps->GetRandomGroup();
  }

  std::vector<ColorMapGroup> nextColorMapGroups(m_colorizers.size());
  const size_t numPerGroup = nextColorMapGroups.size() / numDifferentGroups;
  size_t n = 0;
  for (size_t i = 0; i < nextColorMapGroups.size(); ++i)
  {
    nextColorMapGroups[i] = groups[n];
    if ((0 == (i % numPerGroup)) && (n < (numDifferentGroups - 1)))
    {
      ++n;
    }
  }

  if (m_goomRand.ProbabilityOfMInN(1, 2))
  {
    m_goomRand.Shuffle(begin(nextColorMapGroups), end(nextColorMapGroups));
  }

  return nextColorMapGroups;
}

void TentacleDriver::CheckForTimerEvents()
{
  if ((m_updateNum % CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES) != 0)
  {
    return;
  }

  const std::vector<ColorMapGroup> nextGroups = GetNextColorMapGroups();
  for (size_t i = 0; i < m_colorizers.size(); ++i)
  {
    m_colorizers[i]->SetColorMapGroup(nextGroups[i]);
  }

  if (m_colorMode != ColorModes::MINIMAL)
  {
    for (auto& colorizer : m_colorizers)
    {
      if (ChangeCurrentColorMapEvent())
      {
        colorizer->ChangeColorMap();
      }
    }
  }

  m_tentacles.ColorMapsChanged();

  m_dotDrawer.ChangeDotSizes();
  m_numNodesBetweenDots =
      m_goomRand.GetRandInRange(MIN_STEPS_BETWEEN_NODES, MAX_STEPS_BETWEEN_NODES + 1U);
}

void TentacleDriver::FreshStart()
{
  const ColorMapGroup nextColorMapGroup = m_colorMaps->GetRandomGroup();
  for (auto& colorizer : m_colorizers)
  {
    colorizer->SetColorMapGroup(nextColorMapGroup);
    if (m_colorMode != ColorModes::MINIMAL)
    {
      colorizer->ChangeColorMap();
    }
  }
}

void TentacleDriver::Update(const float angle,
                            const float distance,
                            const float distance2,
                            const Pixel& color,
                            const Pixel& lowColor)
{
  ++m_updateNum;

  UpdateIterTimers();
  CheckForTimerEvents();

  constexpr float ITER_ZERO_LERP_FACTOR = 0.9F;

  for (size_t i = 0; i < m_numTentacles; ++i)
  {
    Tentacle3D& tentacle = m_tentacles[i];
    Tentacle2D& tentacle2D = tentacle.Get2DTentacle();

    const float iterZeroYVal = m_tentacleParams[i].iterZeroYValWave.GetNext();
    tentacle2D.SetIterZeroLerpFactor(static_cast<double>(ITER_ZERO_LERP_FACTOR));
    tentacle2D.SetIterZeroYVal(static_cast<double>(iterZeroYVal));

    tentacle2D.Iterate();

    Plot3D(tentacle, color, lowColor, angle, distance, distance2);
  }
}

inline auto GetBrightnessCut(const Tentacle3D& tentacle, const float distance2) -> float
{
  if (std::abs(tentacle.GetHead().x) < 10)
  {
    if (distance2 < 8)
    {
      return 0.5F;
    }
    return 0.2F;
  }
  return 1.0F;
}

constexpr int COORD_IGNORE_VAL = -666;

void TentacleDriver::Plot3D(const Tentacle3D& tentacle,
                            const Pixel& dominantColor,
                            const Pixel& dominantLowColor,
                            const float angle,
                            const float distance,
                            const float distance2)
{
  const std::vector<V3dFlt> vertices = tentacle.GetVertices();
  const size_t n = vertices.size();

  V3dFlt cam = {0.0F, 0.0F, -3.0F}; // TODO ????????????????????????????????
  cam.z += distance2;
  cam.y += 2.0F * std::sin(-(angle - m_half_pi) / 4.3F);

  constexpr float HEAD_CUTOFF = 10.0F;
  constexpr float ANGLE_ADJ_FRACTION = 0.05F;
  float angleAboutY = angle;
  if ((-HEAD_CUTOFF < tentacle.GetHead().x) && (tentacle.GetHead().x < 0.0F))
  {
    angleAboutY -= ANGLE_ADJ_FRACTION * m_pi;
  }
  else if ((0.0F <= tentacle.GetHead().x) && (tentacle.GetHead().x < HEAD_CUTOFF))
  {
    angleAboutY += ANGLE_ADJ_FRACTION * m_pi;
  }

  const float sinCamAngle = std::sin(m_pi - angleAboutY);
  const float cosCamAngle = std::cos(m_pi - angleAboutY);

  std::vector<V3dFlt> v3{vertices};
  for (size_t i = 0; i < n; ++i)
  {
    RotateV3DAboutYAxis(sinCamAngle, cosCamAngle, v3[i], v3[i]);
    TranslateV3D(cam, v3[i]);
  }

  const std::vector<V2dInt> v2 = ProjectV3DOntoV2D(v3, distance);

  constexpr float BRIGHTNESS = 2.0F;
  constexpr float BRIGHTNESS_DISTANCE_CUTOFF = 30.F;

  const float brightnessCut = GetBrightnessCut(tentacle, distance2);

  // Faraway tentacles get smaller and draw_line adds them together making them look
  // very white and over-exposed. If we reduce the brightness, then all the combined
  // tentacles look less bright and white and more colors show through.
  using GetMixedColorsFunc = std::function<std::tuple<Pixel, Pixel>(const size_t nodeNum)>;
  GetMixedColorsFunc getMixedColors = [&](const size_t nodeNum)
  {
    return tentacle.GetMixedColors(nodeNum, dominantColor, dominantLowColor,
                                   brightnessCut * BRIGHTNESS);
  };
  if (distance2 > BRIGHTNESS_DISTANCE_CUTOFF)
  {
    constexpr float FAR_AWAY_DISTANCE = 50.0F;
    constexpr float MIN_RAND_BRIGHTNESS = 0.1F;
    constexpr float MAX_RAND_BRIGHTNESS = 0.3F;
    const float randBrightness =
        m_goomRand.GetRandInRange(MIN_RAND_BRIGHTNESS, MAX_RAND_BRIGHTNESS);
    const float brightness =
        brightnessCut * std::max(randBrightness, FAR_AWAY_DISTANCE / distance2);
    getMixedColors = [&, brightness](const size_t nodeNum)
    { return tentacle.GetMixedColors(nodeNum, dominantColor, dominantLowColor, brightness); };
  }

  for (size_t nodeNum = 0; nodeNum < (v2.size() - 1); ++nodeNum)
  {
    const int32_t ix0 = v2[nodeNum].x;
    const int32_t ix1 = v2[nodeNum + 1].x;
    const int32_t iy0 = v2[nodeNum].y;
    const int32_t iy1 = v2[nodeNum + 1].y;

    if (((ix0 == COORD_IGNORE_VAL) && (iy0 == COORD_IGNORE_VAL)) ||
        ((ix1 == COORD_IGNORE_VAL) && (iy1 == COORD_IGNORE_VAL)))
    {
      LogDebug("Skipping draw ignore vals {}: ix0 = {}, iy0 = {}, ix1 = {}, iy1 = {}.", nodeNum,
               ix0, iy0, ix1, iy1);
    }
    else if ((ix0 == ix1) && (iy0 == iy1))
    {
      LogDebug("Skipping draw equal points {}: ix0 = {}, iy0 = {}, ix1 = {}, iy1 = {}.", nodeNum,
               ix0, iy0, ix1, iy1);
    }
    else
    {
      LogDebug("draw_line {}: ix0 = {}, iy0 = {}, ix1 = {}, iy1 = {}.", nodeNum, ix0, iy0, ix1,
               iy1);

#if __cplusplus <= 201402L
      const auto mixedColors = getMixedColors(nodeNum);
      const auto color = std::get<0>(mixedColors);
      const auto lowColor = std::get<1>(mixedColors);
#else
      const auto [color, lowColor] = getMixedColors(nodeNum);
#endif
      const std::vector<Pixel> colors{color, lowColor};

      constexpr uint8_t THICKNESS = 1;
      m_draw.Line(ix0, iy0, ix1, iy1, colors, THICKNESS);

      if ((nodeNum % m_numNodesBetweenDots) == 0)
      {
        const Pixel color1 = m_colorizers.at(tentacle.Get2DTentacle().GetID())->GetColor(nodeNum);
        constexpr float DOT_BRIGHTNESS = 5.0F;
        m_dotDrawer.DrawDots({ix1, iy1}, {color1, color}, DOT_BRIGHTNESS);
      }
    }
  }
}

auto TentacleDriver::ProjectV3DOntoV2D(const std::vector<V3dFlt>& point3D,
                                       const float distance) const -> std::vector<V2dInt>
{
  std::vector<V2dInt> point2D(point3D.size());

  const int Xp0 = point3D[0].ignore || (point3D[0].z <= 2)
                      ? 1
                      : static_cast<int>((distance * point3D[0].x) / point3D[0].z);
  const int Xpn = point3D[point3D.size() - 1].ignore || (point3D[point3D.size() - 1].z <= 2)
                      ? 1
                      : static_cast<int>((distance * point3D[point3D.size() - 1].x) /
                                         point3D[point3D.size() - 1].z);
  const float xSpread = std::min(1.0F, std::abs(static_cast<float>(Xp0 - Xpn)) / 10.0F);

  for (size_t i = 0; i < point3D.size(); ++i)
  {
    if ((!point3D[i].ignore) && (point3D[i].z > 2))
    {
      const auto Xp = static_cast<int32_t>((xSpread * distance * point3D[i].x) / point3D[i].z);
      const auto Yp = static_cast<int32_t>((xSpread * distance * point3D[i].y) / point3D[i].z);
      point2D[i].x = Xp + static_cast<int32_t>(m_draw.GetScreenWidth() >> 1);
      point2D[i].y = -Yp + static_cast<int32_t>(m_draw.GetScreenHeight() >> 1);
    }
    else
    {
      point2D[i].x = COORD_IGNORE_VAL;
      point2D[i].y = COORD_IGNORE_VAL;
    }
  }

  return point2D;
}

inline void TentacleDriver::RotateV3DAboutYAxis(const float sinAngle,
                                                const float cosAngle,
                                                const V3dFlt& vSrc,
                                                V3dFlt& vDest)
{
  const float vi_x = vSrc.x;
  const float vi_z = vSrc.z;
  vDest.x = (vi_x * cosAngle) - (vi_z * sinAngle);
  vDest.z = (vi_x * sinAngle) + (vi_z * cosAngle);
  vDest.y = vSrc.y;
}

inline void TentacleDriver::TranslateV3D(const V3dFlt& vAdd, V3dFlt& vInOut)
{
  vInOut.x += vAdd.x;
  vInOut.y += vAdd.y;
  vInOut.z += vAdd.z;
}

TentacleColorMapColorizer::TentacleColorMapColorizer(const ColorMapGroup cmg,
                                                     const size_t numNodes,
                                                     IGoomRand& goomRand) noexcept
  : m_numNodes{numNodes},
    m_currentColorMapGroup{cmg},
    m_colorMaps{goomRand},
    m_colorMap{m_colorMaps.GetRandomColorMapPtr(m_currentColorMapGroup, RandomColorMaps::ALL)},
    m_prevColorMap{m_colorMap}
{
}

auto TentacleColorMapColorizer::GetColorMapGroup() const -> ColorMapGroup
{
  return m_currentColorMapGroup;
}

void TentacleColorMapColorizer::SetColorMapGroup(const ColorMapGroup colorMapGroup)
{
  m_currentColorMapGroup = colorMapGroup;
}

void TentacleColorMapColorizer::ChangeColorMap()
{
  // Save the current color map to do smooth transitions to next color map.
  m_prevColorMap = m_colorMap;
  m_tTransition = 1.0;
  m_colorMap = m_colorMaps.GetRandomColorMapPtr(m_currentColorMapGroup, RandomColorMaps::ALL);
}

auto TentacleColorMapColorizer::GetColor(const size_t nodeNum) const -> Pixel
{
  const float t = static_cast<float>(nodeNum) / static_cast<float>(m_numNodes);
  Pixel nextColor = m_colorMap->GetColor(t);

  // Keep going with the smooth transition until 'm_tTransition' runs out.
  if (m_tTransition > 0.0F)
  {
    nextColor = IColorMap::GetColorMix(nextColor, m_prevColorMap->GetColor(t), m_tTransition);
    m_tTransition -= TRANSITION_STEP;
  }

  return nextColor;
}

CirclesTentacleLayout::CirclesTentacleLayout(const float radiusMin,
                                             const float radiusMax,
                                             const std::vector<size_t>& numCircleSamples,
                                             const float zConst)
{
  const size_t numCircles = numCircleSamples.size();
  if (numCircles < 2)
  {
    std::logic_error(std20::format("There must be >= 2 circle sample numbers not {}.", numCircles));
  }
  for (const auto numSample : numCircleSamples)
  {
    if ((numSample % 2) != 0)
    {
      // Perspective looks bad with odd because of x=0 tentacle.
      std::logic_error(std20::format("Circle sample num must be even not {}.", numSample));
    }
  }

#ifndef NO_LOGGING
  // TODO - Should be lerps here?
  const auto logLastPoint = [&](size_t i, const float r, const float angle)
  {
    const size_t el = points.size() - 1;
    logDebug("  sample {:3}: angle = {:+6.2f}, cos(angle) = {:+6.2f}, r = {:+6.2f},"
             " pt[{:3}] = ({:+6.2f}, {:+6.2f}, {:+6.2f})",
             i, angle, cos(angle), r, el, points.at(el).x, points.at(el).y, points.at(el).z);
  };
#endif

  const auto getSamplePoints = [&](const float radius, const size_t numSample,
                                   const float angleStart, const float angleFinish)
  {
    const float angleStep = (angleFinish - angleStart) / static_cast<float>(numSample - 1);
    float angle = angleStart;
    for (size_t i = 0; i < numSample; ++i)
    {
      const float x = radius * std::cos(angle);
      const float y = radius * std::sin(angle);
      const V3dFlt point = {x, y, zConst};
      m_points.push_back(point);
#ifndef NO_LOGGING
      logLastPoint(i, radius, angle);
#endif
      angle += angleStep;
    }
  };

  const float angleLeftStart = +m_half_pi;
  const float angleLeftFinish = 1.5F * m_pi;
  const float angleRightStart = -m_half_pi;
  const float angleRightFinish = +m_half_pi;

  const float angleOffsetStart = 0.035F * m_pi;
  const float angleOffsetFinish = 0.035F * m_pi;
  const float offsetStep =
      (angleOffsetStart - angleOffsetFinish) / static_cast<float>(numCircles - 1);
  const float radiusStep = (radiusMax - radiusMin) / static_cast<float>(numCircles - 1);

  float r = radiusMax;
  float angleOffset = angleOffsetStart;
  for (const auto numSample : numCircleSamples)
  {
    getSamplePoints(r, numSample / 2, angleLeftStart + angleOffset, angleLeftFinish - angleOffset);
    getSamplePoints(r, numSample / 2, angleRightStart + angleOffset,
                    angleRightFinish - angleOffset);

    r -= radiusStep;
    angleOffset -= offsetStep;
  }
}

auto CirclesTentacleLayout::GetNumPoints() const -> size_t
{
  return m_points.size();
}

auto CirclesTentacleLayout::GetPoints() const -> const std::vector<V3dFlt>&
{
  return m_points;
}

#if __cplusplus <= 201402L
} // namespace TENTACLES
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::TENTACLES
#endif
