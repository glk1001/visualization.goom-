#include "tentacle_driver.h"

#include "color/colormaps.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "tentacle3d.h"
#include "utils/goom_rand_base.h"
#include "utils/mathutils.h"

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
using UTILS::m_pi;
using UTILS::SmallImageBitmaps;

const size_t TentacleDriver::CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES = 400;

TentacleDriver::TentacleDriver(IGoomDraw& draw,
                               IGoomRand& goomRand,
                               const SmallImageBitmaps& smallBitmaps,
                               const ColorMapGroup initialColorMapGroup,
                               const ITentacleLayout& tentacleLayout) noexcept
  : m_draw{draw},
    m_goomRand{goomRand},
    m_tentacleLayout{tentacleLayout},
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
    m_tentacleParams{GetTentacleParams(m_tentacleLayout.GetNumPoints(), m_iterParamsGroups)},
    m_colorizers{GetColorizers(m_goomRand, m_tentacleParams, initialColorMapGroup)},
    m_tentacles{GetTentacles(m_goomRand, m_tentacleParams, m_tentacleLayout, m_colorizers)},
    m_tentaclePlotter{m_draw, m_goomRand, smallBitmaps}
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

auto TentacleDriver::GetColorizers(IGoomRand& goomRand,
                                   const std::vector<IterationParams>& tentacleParams,
                                   const COLOR::ColorMapGroup initialColorMapGroup)
    -> std::vector<std::shared_ptr<ITentacleColorizer>>
{
  std::vector<std::shared_ptr<ITentacleColorizer>> colorizers{};
  for (const auto& params : tentacleParams)
  {
    std::shared_ptr<TentacleColorMapColorizer> colorizer{
        std::make_shared<TentacleColorMapColorizer>(initialColorMapGroup, params.numNodes,
                                                    goomRand)};
    colorizers.emplace_back(colorizer);
  }

  return colorizers;
}

auto TentacleDriver::GetTentacleParams(const size_t numTentacles,
                                       const std::vector<IterParamsGroup>& iterParamsGroups)
    -> std::vector<IterationParams>
{
  std::vector<IterationParams> tentacleParams(numTentacles);
  const size_t numInParamGroup = numTentacles / iterParamsGroups.size();

  // IMPORTANT: Dividing the numTentacles into param groups makes the tentacle movements
  //            look good. I tried making them all move the same, but it just didn't look
  //            right. For the moment this seems best.
  const float tStep = 1.0F / static_cast<float>(numInParamGroup - 1);
  size_t paramsIndex = 0;
  float t = 0.0F;
  for (size_t i = 0; i < numTentacles; ++i)
  {
    const IterParamsGroup paramsGroup = iterParamsGroups.at(paramsIndex);
    if (0 == (i % numInParamGroup))
    {
      if (paramsIndex < (iterParamsGroups.size() - 1))
      {
        ++paramsIndex;
      }
      t = 0.0;
    }
    tentacleParams[i] = paramsGroup.GetNextIterationParams(t);
    t += tStep;
  }

  return tentacleParams;
}

auto TentacleDriver::GetTentacles(
    IGoomRand& goomRand,
    const std::vector<IterationParams>& tentacleParams,
    const ITentacleLayout& tentacleLayout,
    const std::vector<std::shared_ptr<ITentacleColorizer>>& colorizers) -> std::vector<Tentacle3D>
{
  std::vector<Tentacle3D> tentacles{};

  for (size_t i = 0; i < tentacleParams.size(); ++i)
  {
    std::unique_ptr<Tentacle2D> tentacle2D{CreateNewTentacle2D(goomRand, i, tentacleParams[i])};

    // To hide the annoying flapping tentacle head, make near the head very dark.
    constexpr V3dFlt INITIAL_HEAD_POS = {0, 0, 0};
    const auto headColor = Pixel{5, 5, 5, MAX_ALPHA};
    const Pixel headLowColor = headColor;
    Tentacle3D tentacle{
        std::move(tentacle2D), colorizers[colorizers.size() - 1], headColor, headLowColor,
        INITIAL_HEAD_POS,      Tentacle2D::MIN_NUM_NODES,         goomRand};

    tentacles.emplace_back(std::move(tentacle));
  }

  UpdateTentaclesLayout(tentacles, tentacleLayout);

  return tentacles;
}

auto TentacleDriver::IterParamsGroup::GetNextIterationParams(const float t) const
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

auto TentacleDriver::CreateNewTentacle2D(IGoomRand& goomRand,
                                         const size_t id,
                                         const IterationParams& params)
    -> std::unique_ptr<Tentacle2D>
{
  const float tentacleLen = std::max(1.0F, goomRand.GetRandInRange(0.99F, 1.01F) * params.length);
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
  for (auto& tentacle : m_tentacles)
  {
    tentacle.Get2DTentacle().StartIterating();
  }

  m_updateNum = 0;
}

void TentacleDriver::UpdateTentaclesLayout(std::vector<Tentacle3D>& tentacles,
                                           const ITentacleLayout& tentacleLayout)
{
  std::vector<size_t> sortedLongestFirst(tentacles.size());
  std::iota(sortedLongestFirst.begin(), sortedLongestFirst.end(), 0);
  const auto compareByLength = [&](const size_t id1, const size_t id2)
  {
    const double len1 = tentacles[id1].Get2DTentacle().GetLength();
    const double len2 = tentacles[id2].Get2DTentacle().GetLength();
    // Sort by longest first.
    return len1 > len2;
  };
  std::sort(sortedLongestFirst.begin(), sortedLongestFirst.end(), compareByLength);

  for (size_t i = 0; i < tentacles.size(); ++i)
  {
    tentacles[sortedLongestFirst.at(i)].SetHead(tentacleLayout.GetPoints().at(i));
  }

  // To help with perspective, any tentacles near vertical centre will be shortened.
  for (auto& tentacle : tentacles)
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
  for (auto& params : m_tentacleParams)
  {
    const float newFreq = value * params.iterZeroYValWaveFreq;
    params.iterZeroYValWave.SetFrequency(newFreq);
  }
}

void TentacleDriver::SetReverseColorMix(const bool value)
{
  for (auto& tentacle : m_tentacles)
  {
    tentacle.SetReverseColorMix(value);
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
  for (auto& group : groups)
  {
    group = m_colorMaps->GetRandomGroup();
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

  TentaclesColorMapsChanged();

  m_tentaclePlotter.ChangeDotSizes();
  m_tentaclePlotter.ChangeNumNodesBetweenDots();
}

inline void TentacleDriver::TentaclesColorMapsChanged()
{
  for (auto& tentacle : m_tentacles)
  {
    tentacle.ColorMapsChanged();
  }
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

  for (size_t i = 0; i < m_tentacles.size(); ++i)
  {
    Tentacle3D& tentacle = m_tentacles[i];
    Tentacle2D& tentacle2D = tentacle.Get2DTentacle();

    const float iterZeroYVal = m_tentacleParams[i].iterZeroYValWave.GetNext();
    tentacle2D.SetIterZeroLerpFactor(static_cast<double>(ITER_ZERO_LERP_FACTOR));
    tentacle2D.SetIterZeroYVal(static_cast<double>(iterZeroYVal));

    tentacle2D.Iterate();

    m_tentaclePlotter.Plot3D(tentacle);
  }
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
