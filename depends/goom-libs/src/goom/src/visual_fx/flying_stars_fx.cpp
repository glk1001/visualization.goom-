//#undef NO_LOGGING

#include "flying_stars_fx.h"

#include "flying_stars/star_drawer.h"
#include "flying_stars/star_maker.h"
#include "flying_stars/star_types_base.h"
#include "flying_stars/star_types_container.h"
#include "flying_stars/stars.h"
#include "fx_helper.h"
#include "goom_config.h"
#include "goom_logger.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "spimpl.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "visual_fx/fx_utils/random_pixel_blender.h"

#include <algorithm>
#include <string>
#include <vector>

namespace GOOM::VISUAL_FX
{

using FLYING_STARS::IStarType;
using FLYING_STARS::Star;
using FLYING_STARS::StarDrawer;
using FLYING_STARS::StarMaker;
using FLYING_STARS::StarTypesContainer;
using FX_UTILS::RandomPixelBlender;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::Weights;

static constexpr auto COLOR_MAP_MODE_ONE_MAP_PER_ANGLE_WEIGHT      = 30.0F;
static constexpr auto COLOR_MAP_MODE_ONE_MAP_FOR_ALL_ANGLES_WEIGHT = 10.0F;
static constexpr auto COLOR_MAP_MODE_MEGA_RANDOM_WEIGHT            = 01.0F;

class FlyingStarsFx::FlyingStarsImpl
{
public:
  FlyingStarsImpl(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps);

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  [[nodiscard]] auto GetCurrentStarTypeColorMapsNames() const noexcept -> std::vector<std::string>;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  auto ApplyMultiple() noexcept -> void;

private:
  const FxHelper* m_fxHelper;
  StarMaker m_starMaker;
  StarDrawer m_starDrawer;
  StarTypesContainer m_starTypesContainer;

  uint32_t m_counter                  = 0;
  static constexpr uint32_t MAX_COUNT = 100;

  using ColorMapMode = IStarType::ColorMapMode;
  Weights<ColorMapMode> m_colorMapModeWeights;
  auto ChangeMapsAndModes() -> void;

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  static constexpr uint32_t MAX_TOTAL_NUM_ACTIVE_STARS = 1024;
  static constexpr uint32_t MIN_TOTAL_NUM_ACTIVE_STARS = 100;
  std::vector<Star> m_activeStars{};
  auto CheckForStarEvents() noexcept -> void;
  auto SoundEventOccurred() noexcept -> void;
  auto ChangeColorMapMode() noexcept -> void;
  auto DrawStars() noexcept -> void;
  [[nodiscard]] auto IsStarDead(const Star& star) const noexcept -> bool;
  auto RemoveDeadStars() noexcept -> void;

  static constexpr auto MIN_NUM_STAR_CLUSTERS   = 0U;
  static constexpr auto MAX_NUM_STAR_CLUSTERS   = 2U;
  static constexpr auto MAX_STAR_CLUSTER_WIDTH  = 320.0F;
  static constexpr auto MAX_STAR_CLUSTER_HEIGHT = 200.0F;
  // Why 320,200 ? Because the FX was developed on 320x200.
  static constexpr auto MIN_STAR_CLUSTER_HEIGHT = 50.0F;
  float m_heightRatio =
      m_fxHelper->goomInfo->GetDimensions().GetFltHeight() / MAX_STAR_CLUSTER_HEIGHT;
  auto AddStarClusters() -> void;
  auto AddStarCluster(const IStarType& starType, uint32_t totalNumActiveStars) noexcept -> void;
  [[nodiscard]] auto GetNumStarsToAdd(uint32_t totalNumActiveStars) const noexcept -> uint32_t;
  [[nodiscard]] auto GetMaxStarsInACluster() const noexcept -> uint32_t;
  [[nodiscard]] auto GetStarProperties() const noexcept -> StarMaker::StarProperties;
};

FlyingStarsFx::FlyingStarsFx(const FxHelper& fxHelper,
                             const SmallImageBitmaps& smallBitmaps) noexcept
  : m_pimpl{spimpl::make_unique_impl<FlyingStarsImpl>(fxHelper, smallBitmaps)}
{
}

auto FlyingStarsFx::GetFxName() const noexcept -> std::string
{
  return "Flying Stars FX";
}

auto FlyingStarsFx::Start() noexcept -> void
{
  // nothing to be done
}

auto FlyingStarsFx::Finish() noexcept -> void
{
  // nothing to be done
}

auto FlyingStarsFx::Resume() noexcept -> void
{
  // nothing to be done
}

auto FlyingStarsFx::Suspend() noexcept -> void
{
  // nothing to be done
}

auto FlyingStarsFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept
    -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto FlyingStarsFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto FlyingStarsFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept
    -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto FlyingStarsFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentStarTypeColorMapsNames();
}

auto FlyingStarsFx::ApplyMultiple() noexcept -> void
{
  m_pimpl->ApplyMultiple();
}

FlyingStarsFx::FlyingStarsImpl::FlyingStarsImpl(const FxHelper& fxHelper,
                                                const SmallImageBitmaps& smallBitmaps)
  : m_fxHelper{&fxHelper},
    m_starMaker{*fxHelper.goomRand},
    m_starDrawer{*fxHelper.draw, *m_fxHelper->goomRand, smallBitmaps},
    m_starTypesContainer{*fxHelper.goomInfo, *fxHelper.goomRand},
    m_colorMapModeWeights{
        *m_fxHelper->goomRand,
        {
            { ColorMapMode::ONE_MAP_PER_ANGLE,      COLOR_MAP_MODE_ONE_MAP_PER_ANGLE_WEIGHT },
            { ColorMapMode::ONE_MAP_FOR_ALL_ANGLES, COLOR_MAP_MODE_ONE_MAP_FOR_ALL_ANGLES_WEIGHT },
            { ColorMapMode::ALL_MAPS_RANDOM,        COLOR_MAP_MODE_MEGA_RANDOM_WEIGHT },
        }
    },
    m_pixelBlender{*fxHelper.goomRand}
{
  m_activeStars.reserve(MAX_TOTAL_NUM_ACTIVE_STARS);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetCurrentStarTypeColorMapsNames() const noexcept
    -> std::vector<std::string>
{
  return m_starTypesContainer.GetCurrentColorMapsNames();
}

auto FlyingStarsFx::FlyingStarsImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  //LogInfo("Setting weighted color maps for id {}", weightedColorMaps.id);
  //LogInfo("Main color maps: {}", weightedColorMaps.mainColorMaps->GetColorMapsName());
  //LogInfo("Low color maps: {}", weightedColorMaps.lowColorMaps->GetColorMapsName());

  m_starTypesContainer.SetWeightedColorMaps(
      weightedColorMaps.id, weightedColorMaps.mainColorMaps, weightedColorMaps.lowColorMaps);

  ChangeColorMapMode();
}

inline auto FlyingStarsFx::FlyingStarsImpl::ChangeColorMapMode() noexcept -> void
{
  m_starTypesContainer.SetColorMapMode(m_colorMapModeWeights.GetRandomWeighted());
}

inline auto FlyingStarsFx::FlyingStarsImpl::ChangeMapsAndModes() -> void
{
  ChangeColorMapMode();
  m_starTypesContainer.ChangeColorMode();
  m_starDrawer.ChangeDrawMode();
}

inline auto FlyingStarsFx::FlyingStarsImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto FlyingStarsFx::FlyingStarsImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept
    -> void
{
  m_starTypesContainer.SetZoomMidpoint(zoomMidpoint);
}

/**
 * Ajoute de nouvelles particules au moment d'un evenement sonore.
 */
inline auto FlyingStarsFx::FlyingStarsImpl::SoundEventOccurred() noexcept -> void
{
  ChangeColorMapMode();
  AddStarClusters();
}

inline auto FlyingStarsFx::FlyingStarsImpl::ApplyMultiple() noexcept -> void
{
  ++m_counter;

  UpdatePixelBlender();
  CheckForStarEvents();
  DrawStars();
  RemoveDeadStars();
}

inline auto FlyingStarsFx::FlyingStarsImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->draw->SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

auto FlyingStarsFx::FlyingStarsImpl::CheckForStarEvents() noexcept -> void
{
  if ((not m_activeStars.empty()) and
      (m_fxHelper->goomInfo->GetSoundEvents().GetTimeSinceLastGoom() >= 1))
  {
    return;
  }

  SoundEventOccurred();

  if (static constexpr auto PROB_CHANGE_MAPS_AND_MODES = 1.0F / 20.0F;
      m_fxHelper->goomRand->ProbabilityOf(PROB_CHANGE_MAPS_AND_MODES))
  {
    ChangeMapsAndModes();
  }
  else if (m_counter > MAX_COUNT)
  {
    m_counter = 0;
    ChangeMapsAndModes();
  }
}

auto FlyingStarsFx::FlyingStarsImpl::DrawStars() noexcept -> void
{
  const auto speedFactor = m_fxHelper->goomRand->GetRandInRange(0.1F, 10.0F);

  for (auto& star : m_activeStars)
  {
    star.Update();

    if (star.IsTooOld())
    {
      continue;
    }

    m_starDrawer.DrawStar(star, speedFactor);
  }
}

auto FlyingStarsFx::FlyingStarsImpl::RemoveDeadStars() noexcept -> void
{
  const auto isDead = [this](const Star& star) { return IsStarDead(star); };
#if __cplusplus <= 201703L
  m_activeStars.erase(std::remove_if(begin(m_activeStars), end(m_activeStars), isDead),
                      end(m_activeStars));
#else
  const auto numRemoved = std::erase_if(m_activeStars, isDead);
#endif
}

auto FlyingStarsFx::FlyingStarsImpl::IsStarDead(const Star& star) const noexcept -> bool
{
  static constexpr auto DEAD_MARGIN = 64;

  if ((star.GetStartPos().x < -DEAD_MARGIN) ||
      (star.GetStartPos().x >
       static_cast<float>(m_fxHelper->goomInfo->GetDimensions().GetWidth() + DEAD_MARGIN)))
  {
    return true;
  }
  if ((star.GetStartPos().y < -DEAD_MARGIN) ||
      (star.GetStartPos().y >
       static_cast<float>(m_fxHelper->goomInfo->GetDimensions().GetHeight() + DEAD_MARGIN)))
  {
    return true;
  }

  return star.IsTooOld();
}

auto FlyingStarsFx::FlyingStarsImpl::AddStarClusters() -> void
{
  const auto numStarClusters =
      m_fxHelper->goomRand->GetRandInRange(MIN_NUM_STAR_CLUSTERS, MAX_NUM_STAR_CLUSTERS + 1);
  const auto totalNumActiveStars = m_fxHelper->goomRand->GetRandInRange(
      MIN_TOTAL_NUM_ACTIVE_STARS, MAX_TOTAL_NUM_ACTIVE_STARS + 1);

  for (auto i = 0U; i < numStarClusters; ++i)
  {
    auto& starType = m_starTypesContainer.GetRandomStarType();

    starType.UpdateWindAndGravity();
    starType.UpdateFixedColorMapNames();

    AddStarCluster(starType, totalNumActiveStars);
  }
}

auto FlyingStarsFx::FlyingStarsImpl::AddStarCluster(const IStarType& starType,
                                                    const uint32_t totalNumActiveStars) noexcept
    -> void
{
  if (totalNumActiveStars <= m_activeStars.size())
  {
    return;
  }

  m_starMaker.StartNewCluster(starType, GetNumStarsToAdd(totalNumActiveStars), GetStarProperties());
  while (m_starMaker.MoreStarsToMake())
  {
    m_activeStars.emplace_back(m_starMaker.MakeNewStar());
  }
}

auto FlyingStarsFx::FlyingStarsImpl::GetStarProperties() const noexcept -> StarMaker::StarProperties
{
  static constexpr auto NOMINAL_PATH_LENGTH_FACTOR = 1.5F;

  return StarMaker::StarProperties{
      /* .heightRatio = */ m_heightRatio,
      /* .defaultPathLength = */ (1.0F + m_fxHelper->goomInfo->GetSoundEvents().GetGoomPower()) *
          (m_fxHelper->goomRand->GetRandInRange(MIN_STAR_CLUSTER_HEIGHT, MAX_STAR_CLUSTER_HEIGHT) /
           MAX_STAR_CLUSTER_WIDTH),
      /* .nominalPathLengthFactor = */
      (m_fxHelper->goomInfo->GetSoundEvents().GetTimeSinceLastBigGoom() >= 1)
          ? 1.0F
          : NOMINAL_PATH_LENGTH_FACTOR};
}

auto FlyingStarsFx::FlyingStarsImpl::GetNumStarsToAdd(
    const uint32_t totalNumActiveStars) const noexcept -> uint32_t
{
  const auto numStarsThatCanBeAdded =
      static_cast<uint32_t>(totalNumActiveStars - m_activeStars.size());
  const auto maxStarsInACluster = GetMaxStarsInACluster();

  return std::min(maxStarsInACluster, numStarsThatCanBeAdded);
}

auto FlyingStarsFx::FlyingStarsImpl::GetMaxStarsInACluster() const noexcept -> uint32_t
{
  const auto maxStarsInACluster = static_cast<uint32_t>(
      m_heightRatio * (100.0F + ((m_fxHelper->goomInfo->GetSoundEvents().GetGoomPower() + 1.0F) *
                                 m_fxHelper->goomRand->GetRandInRange(0.0F, 150.0F))));

  if (m_fxHelper->goomInfo->GetSoundEvents().GetTimeSinceLastBigGoom() < 1)
  {
    return 2 * maxStarsInACluster;
  }

  return maxStarsInACluster;
}

} // namespace GOOM::VISUAL_FX
