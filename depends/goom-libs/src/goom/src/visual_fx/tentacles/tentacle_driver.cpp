#include "tentacle_driver.h"

#include "color/colormaps.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "tentacles3d.h"
#include "utils/goom_rand_base.h"
#include "utils/mathutils.h"
#include "v2d.h"

#undef NDEBUG
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <numeric>
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
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using UTILS::IGoomRand;
using UTILS::m_half_pi;
using UTILS::m_pi;
using UTILS::SmallImageBitmaps;

const size_t TentacleDriver::CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES = 400;

TentacleDriver::TentacleDriver(IGoomDraw& draw,
                               IGoomRand& goomRand,
                               const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{draw},
    m_goomRand{goomRand},
    m_halfScreenWidth{static_cast<int32_t>(m_draw.GetScreenWidth() / 2)},
    m_halfScreenHeight{static_cast<int32_t>(m_draw.GetScreenHeight() / 2)},
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

        }
    },
    // clang-format on
    m_dotDrawer{
        m_draw,
        m_goomRand,
        smallBitmaps,
        // clang-format off
        // min dot sizes
        {
            m_goomRand,
            {
                {1, 100},
                {3, 50},
                {5, 5},
                {7, 100},
            }
        },
        // normal dot sizes
        {
            m_goomRand,
            {
                { 1, 50},
                { 3, 20},
                { 5, 10},
                { 7, 10},
                { 9, 10},
                {11, 10},
                {13,  1},
                {15,  1},
            }
        }
        // clang-format on
    }
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

void TentacleDriver::SetColorMode(const ColorModes colorMode)
{
  m_colorMode = colorMode;
}

constexpr double TENT2D_X_MIN = 0.0;
constexpr double TENT2D_Y_MIN = 0.065736;
constexpr double TENT2D_Y_MAX = 10000.0;

void TentacleDriver::Init(const ColorMapGroup initialColorMapGroup, const ITentacleLayout& layout)
{
  m_numTentacles = layout.GetNumPoints();

  const size_t numInParamGroup = m_numTentacles / m_iterParamsGroups.size();

  // IMPORTANT: Dividing the m_numTentacles into param groups makes the tentacle movements
  //            look good. I tried making them all move the same, but it just didn't look
  //            right. For the moment this seems best.
  const float tStep = 1.0F / static_cast<float>(numInParamGroup - 1);
  size_t paramsIndex = 0;
  float t = 0.0F;
  m_tentacleParams.resize(m_numTentacles);
  for (size_t i = 0; i < m_numTentacles; ++i)
  {
    const IterParamsGroup paramsGroup = m_iterParamsGroups.at(paramsIndex);
    if (0 == (i % numInParamGroup))
    {
      if (paramsIndex < (m_iterParamsGroups.size() - 1))
      {
        ++paramsIndex;
      }
      t = 0.0;
    }
    m_tentacleParams[i] = paramsGroup.GetNext(t);
    t += tStep;

    AddColorizer(initialColorMapGroup, m_tentacleParams[i]);
    AddTentacle(i, m_tentacleParams[i]);
  }

  UpdateTentaclesLayout(layout);

  m_updateNum = 0;
}

void TentacleDriver::AddColorizer(const ColorMapGroup initialColorMapGroup,
                                  const IterationParams& params)
{
  std::shared_ptr<TentacleColorMapColorizer> colorizer{std::make_shared<TentacleColorMapColorizer>(
      initialColorMapGroup, params.numNodes, m_goomRand)};
  m_colorizers.emplace_back(colorizer);
}

void TentacleDriver::AddTentacle(const size_t id, const IterationParams& params)
{
  std::unique_ptr<Tentacle2D> tentacle2D{CreateNewTentacle2D(id, params)};

  // To hide the annoying flapping tentacle head, make near the head very dark.
  constexpr V3dFlt INITIAL_HEAD_POS = {0, 0, 0};
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

auto TentacleDriver::IterParamsGroup::GetNext(const float t) const
    -> TentacleDriver::IterationParams
{
  IterationParams params{};
  params.length =
      goomRand.GetRandInRange(1.0F, 1.000001F * stdnew::lerp(first.length, last.length, t));
  assert(params.length >= 1.0F);
  params.numNodes = static_cast<size_t>(
      goomRand.GetRandInRange(1.0F, 1.000001F) *
      stdnew::lerp(static_cast<float>(first.numNodes), static_cast<float>(last.numNodes), t));
  assert(params.numNodes >= 10);

  params.prevYWeight = goomRand.GetRandInRange(1.0F, 1.000001F) *
                       stdnew::lerp(first.prevYWeight, last.prevYWeight, t);
  params.iterZeroYValWave = first.iterZeroYValWave;
  params.iterZeroYValWaveFreq =
      goomRand.GetRandInRange(1.0F, 1.000001F) *
      stdnew::lerp(first.iterZeroYValWaveFreq, last.iterZeroYValWaveFreq, t);
  return params;
}

auto TentacleDriver::CreateNewTentacle2D(const size_t id, const IterationParams& params)
    -> std::unique_ptr<Tentacle2D>
{
  const float tentacleLen = std::max(1.0F, m_goomRand.GetRandInRange(0.99F, 1.01F) * params.length);
  assert(tentacleLen >= 1);
  const double tent2d_xMax = TENT2D_X_MIN + static_cast<double>(tentacleLen);
  assert(tent2d_xMax >= 1.0);

  std::unique_ptr<Tentacle2D> tentacle{
      std::make_unique<Tentacle2D>(id, params.numNodes, TENT2D_X_MIN, tent2d_xMax, TENT2D_Y_MIN,
                                   TENT2D_Y_MAX, params.prevYWeight, 1.0F - params.prevYWeight)};

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

void TentacleDriver::MultiplyIterZeroYValWaveFreq(const float value)
{
  for (size_t i = 0; i < m_numTentacles; ++i)
  {
    const float newFreq = value * m_tentacleParams[i].iterZeroYValWaveFreq;
    m_tentacleParams[i].iterZeroYValWave.SetFrequency(newFreq);
  }
}

void TentacleDriver::SetReverseColorMix(const bool value)
{
  for (auto& t : m_tentacles)
  {
    t.SetReverseColorMix(value);
  }
}

auto TentacleDriver::GetNextColorMapGroups() const -> std::vector<ColorMapGroup>
{
  const size_t numDifferentGroups =
      ((m_colorMode == ColorModes::MINIMAL) || (m_colorMode == ColorModes::ONE_GROUP_FOR_ALL) ||
       m_goomRand.ProbabilityOfMInN(1, 100))
          ? 1
          : m_goomRand.GetRandInRange(1U, std::min(5U, static_cast<uint32_t>(m_colorizers.size())));
  assert(numDifferentGroups > 0);

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

void TentacleDriver::Update()
{
  ++m_updateNum;

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

    Plot3D(tentacle);
  }
}

constexpr int COORD_IGNORE_VAL = -666;

void TentacleDriver::Plot3D(const Tentacle3D& tentacle)
{
  const float brightness = GetBrightness(tentacle);
  const std::vector<V2dInt> points2D = Get2DTentaclePoints(tentacle);

  for (size_t nodeNum = 0; nodeNum < (points2D.size() - 1); ++nodeNum)
  {
    const int32_t ix0 = points2D[nodeNum].x;
    const int32_t ix1 = points2D[nodeNum + 1].x;
    const int32_t iy0 = points2D[nodeNum].y;
    const int32_t iy1 = points2D[nodeNum + 1].y;

    if (((ix0 == COORD_IGNORE_VAL) && (iy0 == COORD_IGNORE_VAL)) ||
        ((ix1 == COORD_IGNORE_VAL) && (iy1 == COORD_IGNORE_VAL)))
    {
      continue;
    }
    if ((ix0 == ix1) && (iy0 == iy1))
    {
      continue;
    }

    DrawNode(tentacle, nodeNum, ix0, iy0, ix1, iy1, brightness);
  }
}

inline void TentacleDriver::DrawNode(const Tentacle3D& tentacle,
                                     const size_t nodeNum,
                                     const int32_t x0,
                                     const int32_t y0,
                                     const int32_t x1,
                                     const int32_t y1,
                                     const float brightness)
{
  const std::vector<Pixel> colors = GetMixedColors(tentacle, brightness, nodeNum);
  DrawNodeLine(x0, y0, x1, y1, colors);
  DrawNodeDot(nodeNum, x1, y1, colors);
}

inline void TentacleDriver::DrawNodeLine(const int32_t x0,
                                         const int32_t y0,
                                         const int32_t x1,
                                         const int32_t y1,
                                         const std::vector<Pixel>& colors)
{
  constexpr uint8_t THICKNESS = 1;
  m_draw.Line(x0, y0, x1, y1, colors, THICKNESS);
}

inline void TentacleDriver::DrawNodeDot(const size_t nodeNum,
                                        const int32_t x,
                                        const int32_t y,
                                        const std::vector<Pixel>& colors)
{
  if ((nodeNum % m_numNodesBetweenDots) != 0)
  {
    return;
  }

  constexpr float DOT_BRIGHTNESS = 1.5F;
  m_dotDrawer.DrawDots({x, y}, colors, DOT_BRIGHTNESS);
}

inline auto TentacleDriver::Get2DTentaclePoints(const Tentacle3D& tentacle) const
    -> std::vector<V2dInt>
{
  const std::vector<V3dFlt> vertices = tentacle.GetVertices();
  const V3dFlt cam = GetCameraPosition();
  const float angleAboutY = GetTentacleAngleAboutY(tentacle);

  const std::vector<V3dFlt> points3D = GetTransformedPoints(vertices, cam, m_pi - angleAboutY);

  return GetPerspectiveProjection(points3D);
}

inline auto TentacleDriver::GetTentacleAngleAboutY(const Tentacle3D& tentacle) const -> float
{
  constexpr float HEAD_CUTOFF = 10.0F;
  constexpr float ANGLE_ADJ_FRACTION = 0.05F;
  float angleAboutY = m_tentacleAngle;
  if ((-HEAD_CUTOFF < tentacle.GetHead().x) && (tentacle.GetHead().x < 0.0F))
  {
    angleAboutY -= ANGLE_ADJ_FRACTION * m_pi;
  }
  else if ((0.0F <= tentacle.GetHead().x) && (tentacle.GetHead().x < HEAD_CUTOFF))
  {
    angleAboutY += ANGLE_ADJ_FRACTION * m_pi;
  }

  return angleAboutY;
}

inline auto TentacleDriver::GetCameraPosition() const -> V3dFlt
{
  V3dFlt cam = {0.0F, 0.0F, -3.0F}; // TODO ????????????????????????????????
  cam.z += m_cameraDistance;
  cam.y += 2.0F * std::sin(-(m_tentacleAngle - m_half_pi) / 4.3F);
  return cam;
}

inline auto TentacleDriver::GetTransformedPoints(const std::vector<V3dFlt>& points,
                                                 const V3dFlt& translate,
                                                 const float angle) -> std::vector<V3dFlt>
{
  const float sinAngle = std::sin(angle);
  const float cosAngle = std::cos(angle);

  std::vector<V3dFlt> transformedPoints{points};

  for (auto& transformedPoint : transformedPoints)
  {
    RotateAboutYAxis(sinAngle, cosAngle, transformedPoint, transformedPoint);
    Translate(translate, transformedPoint);
  }

  return transformedPoints;
}

inline auto TentacleDriver::GetMixedColors(const Tentacle3D& tentacle,
                                           const float brightness,
                                           const size_t nodeNum) const -> std::vector<Pixel>
{
  const std::pair<Pixel, Pixel> colors =
      tentacle.GetMixedColors(nodeNum, m_dominantColor, m_dominantLowColor, brightness);

  return {colors.first, colors.second};
}

inline auto TentacleDriver::GetBrightness(const Tentacle3D& tentacle) const -> float
{
  // Faraway tentacles get smaller and draw_line adds them together making them look
  // very white and over-exposed. If we reduce the brightness, then all the combined
  // tentacles look less bright and white and more colors show through.

  constexpr float BRIGHTNESS = 2.0F;
  constexpr float BRIGHTNESS_DISTANCE_CUTOFF = 30.F;
  const float brightnessCut = GetBrightnessCut(tentacle);

  if (m_cameraDistance <= BRIGHTNESS_DISTANCE_CUTOFF)
  {
    return brightnessCut * BRIGHTNESS;
  }

  constexpr float FAR_AWAY_DISTANCE = 50.0F;
  const float farAwayBrightness = FAR_AWAY_DISTANCE / m_cameraDistance;
  constexpr float MIN_RAND_BRIGHTNESS = 0.1F;
  constexpr float MAX_RAND_BRIGHTNESS = 0.3F;
  const float randBrightness = m_goomRand.GetRandInRange(MIN_RAND_BRIGHTNESS, MAX_RAND_BRIGHTNESS);
  return brightnessCut * std::max(randBrightness, farAwayBrightness);
}

inline auto TentacleDriver::GetBrightnessCut(const Tentacle3D& tentacle) const -> float
{
  if (std::abs(tentacle.GetHead().x) < 10)
  {
    if (m_cameraDistance < 8)
    {
      return 0.5F;
    }
    return 0.2F;
  }
  return 1.0F;
}

auto TentacleDriver::GetPerspectiveProjection(const std::vector<V3dFlt>& points3D) const
    -> std::vector<V2dInt>
{
  std::vector<V2dInt> points2D(points3D.size());

  const int32_t xProj0 =
      points3D[0].ignore || (points3D[0].z <= 2)
          ? 1
          : static_cast<int32_t>((m_projectionDistance * points3D[0].x) / points3D[0].z);
  const int32_t xProjN =
      points3D[points3D.size() - 1].ignore || (points3D[points3D.size() - 1].z <= 2)
          ? 1
          : static_cast<int32_t>((m_projectionDistance * points3D[points3D.size() - 1].x) /
                                 points3D[points3D.size() - 1].z);

  const float xSpread = std::min(1.0F, std::abs(static_cast<float>(xProj0 - xProjN)) / 10.0F);

  for (size_t i = 0; i < points3D.size(); ++i)
  {
    if ((!points3D[i].ignore) && (points3D[i].z > 2))
    {
      const auto xProj =
          static_cast<int32_t>((xSpread * m_projectionDistance * points3D[i].x) / points3D[i].z);
      const auto yProj =
          static_cast<int32_t>((xSpread * m_projectionDistance * points3D[i].y) / points3D[i].z);
      points2D[i].x = xProj + m_halfScreenWidth;
      points2D[i].y = -yProj + m_halfScreenHeight;
    }
    else
    {
      points2D[i].x = COORD_IGNORE_VAL;
      points2D[i].y = COORD_IGNORE_VAL;
    }
  }

  return points2D;
}

inline void TentacleDriver::RotateAboutYAxis(const float sinAngle,
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

inline void TentacleDriver::Translate(const V3dFlt& vAdd, V3dFlt& vInOut)
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

inline void TentacleColorMapColorizer::SetColorMapGroup(const ColorMapGroup colorMapGroup)
{
  m_currentColorMapGroup = colorMapGroup;
}

inline void TentacleColorMapColorizer::ChangeColorMap()
{
  // Save the current color map to do smooth transitions to next color map.
  m_prevColorMap = m_colorMap;
  m_tTransition = 1.0;
  m_colorMap = m_colorMaps.GetRandomColorMapPtr(m_currentColorMapGroup, RandomColorMaps::ALL);
}

inline auto TentacleColorMapColorizer::GetColor(const size_t nodeNum) const -> Pixel
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

#if __cplusplus <= 201402L
} // namespace TENTACLES
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::TENTACLES
#endif
