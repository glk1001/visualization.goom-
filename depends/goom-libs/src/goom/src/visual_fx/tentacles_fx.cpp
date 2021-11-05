#include "tentacles_fx.h"

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "draw/goom_draw.h"
#include "fx_helpers.h"
#include "goom/logging_control.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "tentacles/tentacle_driver.h"
#include "utils/goom_rand_base.h"
//#undef NO_LOGGING
#include "color/random_colormaps.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "utils/mathutils.h"

#undef NDEBUG
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
#else
namespace GOOM::VISUAL_FX
{
#endif

using COLOR::ColorMapGroup;
using COLOR::GetEvolvedColor;
using COLOR::GetLightenedColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using TENTACLES::CirclesTentacleLayout;
using TENTACLES::TentacleDriver;
using UTILS::IGoomRand;
using UTILS::m_half_pi;
using UTILS::m_pi;
using UTILS::m_two_pi;
using UTILS::Weights;

class TentaclesFx::TentaclesImpl
{
public:
  explicit TentaclesImpl(const FxHelpers& fxHelpers);
  TentaclesImpl(const TentaclesImpl&) noexcept = delete;
  TentaclesImpl(TentaclesImpl&&) noexcept = delete;
  ~TentaclesImpl() noexcept = default;
  auto operator=(const TentaclesImpl&) -> TentaclesImpl& = delete;
  auto operator=(TentaclesImpl&&) -> TentaclesImpl& = delete;

  void SetWeightedColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps);

  void Start();
  void FreshStart();

  void Update();
  void UpdateWithNoDraw();

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  IGoomRand& m_goomRand;
  std::shared_ptr<RandomColorMaps> m_colorMaps{};
  std::shared_ptr<const IColorMap> m_dominantColorMap{};
  Pixel m_dominantColor{};
  void ChangeDominantColor();

  const Weights<size_t> m_driverWeights;
  const std::vector<CirclesTentacleLayout> m_layouts;

  bool m_updatingWithDraw = false;
  float m_cycle = 0.0F;
  static constexpr float CYCLE_INC_MIN = 0.01F;
  static constexpr float CYCLE_INC_MAX = 0.05F;
  float m_cycleInc = CYCLE_INC_MIN;
  float m_lig = 1.15F;
  float m_ligs = 0.1F;
  float m_distt = 10.0F;
  static constexpr double DISTT_MIN = 106.0;
  static constexpr double DISTT_MAX = 286.0;
  float m_distt2 = 0.0F;
  static constexpr float DISTT2_MIN = 8.0F;
  static constexpr float DISTT2_MAX = 1000.0F;
  float m_distt2Offset = 0.0F;
  float m_rot = 0.0F; // entre 0 et m_two_pi
  float m_rotAtStartOfPrettyMove = 0.0F;
  bool m_doRotation = false;
  static auto GetStableRotationOffset(float cycleVal) -> float;
  bool m_isPrettyMoveHappening = false;
  int32_t m_prettyMoveHappeningTimer = 0;
  int32_t m_prettyMoveCheckStopMark = 0;
  static constexpr uint32_t PRETTY_MOVE_HAPPENING_MIN = 100;
  static constexpr uint32_t PRETTY_MOVE_HAPPENING_MAX = 200;
  float m_distt2OffsetPreStep = 0.0F;

  bool m_prettyMoveReadyToStart = false;
  static constexpr int32_t MIN_PRE_PRETTY_MOVE_LOCK = 200;
  static constexpr int32_t MAX_PRE_PRETTY_MOVE_LOCK = 500;
  int32_t m_prePrettyMoveLock = 0;
  int32_t m_postPrettyMoveLock = 0;
  float m_prettyMoveLerpMix = 1.0F / 16.0F; // original goom value
  void IsPrettyMoveHappeningUpdate(float acceleration);
  void PrettyMovePreStart();
  void PrettyMoveStart(float acceleration, int32_t timerVal = -1);
  void PrettyMoveFinish();
  void PrettyMove(float acceleration);
  void PrettyMoveWithNoDraw();
  auto GetModColors() -> std::tuple<Pixel, Pixel>;

  size_t m_countSinceHighAccelLastMarked = 0;
  size_t m_countSinceColorChangeLastMarked = 0;
  void IncCounters();

  static constexpr size_t NUM_DRIVERS = 4;
  std::vector<std::unique_ptr<TentacleDriver>> m_drivers{};
  TentacleDriver* m_currentDriver = nullptr;
  [[nodiscard]] auto GetNextDriver() const -> TentacleDriver*;

  void SetupDrivers();
  void SetDriverRandomColorMaps();
  void InitData();
  void DoTentaclesUpdate();
  void DoPrettyMoveBeforeDraw();
  void DoPrettyMoveWithoutDraw();

  [[nodiscard]] auto StartPrettyMoveEvent() -> bool;
  [[nodiscard]] auto ChangeRotationEvent() -> bool;
  [[nodiscard]] auto TurnRotationOnEvent() -> bool;
  [[nodiscard]] auto ChangeDominantColorMapEvent() -> bool;
  [[nodiscard]] auto ChangeDominantColorEvent() -> bool;
};

TentaclesFx::TentaclesFx(const FxHelpers& fxHelpers) noexcept
  : m_fxImpl{spimpl::make_unique_impl<TentaclesImpl>(fxHelpers)}
{
}

void TentaclesFx::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedColorMaps(weightedMaps);
}

void TentaclesFx::FreshStart()
{
  m_fxImpl->FreshStart();
}

void TentaclesFx::Start()
{
  m_fxImpl->Start();
}

void TentaclesFx::Resume()
{
  // nothing to do
}

void TentaclesFx::Suspend()
{
  // nothing to do
}

void TentaclesFx::Finish()
{
  // nothing to do
}

void TentaclesFx::ApplyMultiple()
{
  m_fxImpl->Update();
}

void TentaclesFx::ApplyNoDraw()
{
  m_fxImpl->UpdateWithNoDraw();
}

auto TentaclesFx::GetFxName() const -> std::string
{
  return "Tentacles FX";
}

TentaclesFx::TentaclesImpl::TentaclesImpl(const FxHelpers& fxHelpers)
  : m_draw{fxHelpers.GetDraw()},
    m_goomInfo{fxHelpers.GetGoomInfo()},
    m_goomRand{fxHelpers.GetGoomRand()},
    // clang-format off
    m_driverWeights{
      m_goomRand,
      {
          {0,  5},
          {1, 15},
          {2, 15},
          {3,  5},
      }},
    m_layouts{
        {10,  80, {16, 12,  8,  6, 4}, 0},
        {10,  80, {20, 16, 12,  6, 4}, 0},
        {10, 100, {30, 20, 14,  6, 4}, 0},
        {10, 110, {36, 26, 20, 12, 6}, 0},
    },
    // clang-format on
    m_rot{GetStableRotationOffset(0)}
{
}

inline auto TentaclesFx::TentaclesImpl::StartPrettyMoveEvent() -> bool
{
  return m_goomRand.ProbabilityOfMInN(0, 400);
}

inline auto TentaclesFx::TentaclesImpl::ChangeRotationEvent() -> bool
{
  return m_goomRand.ProbabilityOfMInN(1, 100);
}

inline auto TentaclesFx::TentaclesImpl::TurnRotationOnEvent() -> bool
{
  return m_goomRand.ProbabilityOfMInN(1, 2);
}

inline auto TentaclesFx::TentaclesImpl::ChangeDominantColorMapEvent() -> bool
{
  return m_goomRand.ProbabilityOfMInN(1, 50);
}

// IMPORTANT. Very delicate here - 1 in 30 seems just right
inline auto TentaclesFx::TentaclesImpl::ChangeDominantColorEvent() -> bool
{
  return m_goomRand.ProbabilityOfMInN(1, 30);
}

inline void TentaclesFx::TentaclesImpl::IncCounters()
{
  ++m_countSinceHighAccelLastMarked;
  ++m_countSinceColorChangeLastMarked;
}

inline void TentaclesFx::TentaclesImpl::SetWeightedColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_colorMaps = weightedMaps;

  m_dominantColorMap = m_colorMaps->GetRandomColorMapPtr(RandomColorMaps::ALL);
  m_dominantColor = RandomColorMaps{m_goomRand}.GetRandomColor(*m_dominantColorMap, 0.0F, 1.0F);

  SetDriverRandomColorMaps();
}

void TentaclesFx::TentaclesImpl::SetDriverRandomColorMaps()
{
  for (auto& drv : m_drivers)
  {
    drv->SetWeightedColorMaps(m_colorMaps);
  }
}

void TentaclesFx::TentaclesImpl::Start()
{
  SetupDrivers();
  InitData();
}

void TentaclesFx::TentaclesImpl::FreshStart()
{
  assert(m_currentDriver);
  m_currentDriver->FreshStart();
}

void TentaclesFx::TentaclesImpl::SetupDrivers()
{
  if (NUM_DRIVERS != m_layouts.size())
  {
    throw std::logic_error("numDrivers != layouts.size()");
  }

  /**
// Temp hack of weights
Weights<ColorMapGroup> colorGroupWeights = colorMaps.getWeights();
colorGroupWeights.ClearWeights(1);
colorGroupWeights.SetWeight(ColorMapGroup::misc, 30000);
colorMaps.setWeights(colorGroupWeights);
***/

  for (size_t i = 0; i < NUM_DRIVERS; ++i)
  {
    m_drivers.emplace_back(std::make_unique<TentacleDriver>(m_draw, m_goomRand));
  }

  if (NUM_DRIVERS != m_driverWeights.GetNumElements())
  {
    throw std::logic_error("numDrivers != driverWeights.GetNumElements()");
  }

  const ColorMapGroup initialColorMapGroup = m_colorMaps->GetRandomGroup();

  for (size_t i = 0; i < NUM_DRIVERS; ++i)
  {
    m_drivers[i]->Init(initialColorMapGroup, m_layouts[i]);
    m_drivers[i]->StartIterating();
  }

  SetDriverRandomColorMaps();

  m_currentDriver = GetNextDriver();
  assert(m_currentDriver);
  m_currentDriver->StartIterating();
}

void TentaclesFx::TentaclesImpl::InitData()
{
  assert(m_currentDriver);

  if (m_goomRand.ProbabilityOfMInN(1, 500))
  {
    m_currentDriver->SetColorMode(TentacleDriver::ColorModes::MINIMAL);
  }
  else if (m_goomRand.ProbabilityOfMInN(1, 500))
  {
    m_currentDriver->SetColorMode(TentacleDriver::ColorModes::MULTI_GROUPS);
  }
  else
  {
    m_currentDriver->SetColorMode(TentacleDriver::ColorModes::ONE_GROUP_FOR_ALL);
  }
  m_currentDriver->SetReverseColorMix(m_goomRand.ProbabilityOfMInN(3, 10));

  m_currentDriver->FreshStart();

  m_distt = static_cast<float>(stdnew::lerp(DISTT_MIN, DISTT_MAX, 0.3));
  m_distt2 = DISTT2_MIN;
  m_distt2Offset = 0;
  m_rot = GetStableRotationOffset(0);

  m_prePrettyMoveLock = 0;
  m_postPrettyMoveLock = 0;
  m_prettyMoveReadyToStart = false;
  if (m_goomRand.ProbabilityOfMInN(1, 5))
  {
    m_isPrettyMoveHappening = false;
    m_prettyMoveHappeningTimer = 0;
  }
  else
  {
    m_isPrettyMoveHappening = true;
    PrettyMoveStart(1.0, PRETTY_MOVE_HAPPENING_MIN / 2);
  }
}

void TentaclesFx::TentaclesImpl::UpdateWithNoDraw()
{
  if (!m_updatingWithDraw)
  {
    // already in tentacle no draw state
    return;
  }

  m_updatingWithDraw = false;

  if (m_ligs > 0.0F)
  {
    m_ligs = -m_ligs;
  }
  m_lig += m_ligs;

  ChangeDominantColor();
}

void TentaclesFx::TentaclesImpl::Update()
{
  IncCounters();

  m_lig += m_ligs;

  if (!m_updatingWithDraw)
  {
    m_updatingWithDraw = true;
    InitData();
  }

  if (m_lig <= 1.01F)
  {
    DoPrettyMoveWithoutDraw();
  }
  else
  {
    DoPrettyMoveBeforeDraw();
    DoTentaclesUpdate();
  }
}

void TentaclesFx::TentaclesImpl::DoPrettyMoveWithoutDraw()
{
  m_lig = 1.05F;
  if (m_ligs < 0.0F)
  {
    m_ligs = -m_ligs;
  }

  PrettyMove(m_goomInfo.GetSoundInfo().GetAcceleration());

  m_cycle += 10.0F * m_cycleInc;
  if (m_cycle > 1000.0F)
  {
    m_cycle = 0.0;
  }
}

void TentaclesFx::TentaclesImpl::DoPrettyMoveBeforeDraw()
{
  if ((m_lig > 10.0F) || (m_lig < 1.1F))
  {
    m_ligs = -m_ligs;
  }

  PrettyMove(m_goomInfo.GetSoundInfo().GetAcceleration());
  m_cycle += m_cycleInc;

  if (m_isPrettyMoveHappening || ChangeDominantColorMapEvent())
  {
    // IMPORTANT. Very delicate here - seems the right time to change maps.
    m_dominantColorMap = m_colorMaps->GetRandomColorMapPtr(RandomColorMaps::ALL);
  }

  if ((m_isPrettyMoveHappening || (m_lig < 6.3F)) && ChangeDominantColorEvent())
  {
    ChangeDominantColor();
    m_countSinceColorChangeLastMarked = 0;
  }
}

void TentaclesFx::TentaclesImpl::DoTentaclesUpdate()
{
#if __cplusplus <= 201402L
  const auto modColors = GetModColors();
  const auto modColor = std::get<0>(modColors);
  const auto modLowColor = std::get<1>(modColors);
#else
  const auto [modColor, modLowColor] = GetModColors();
#endif

  if (m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom() != 0)
  {
  }
  else
  {
    if (m_countSinceHighAccelLastMarked > 100)
    {
      m_countSinceHighAccelLastMarked = 0;
      if (m_countSinceColorChangeLastMarked > 100)
      {
        ChangeDominantColor();
        m_countSinceColorChangeLastMarked = 0;
      }
    }
  }

  // Higher sound acceleration increases tentacle wave frequency.
  assert(m_currentDriver);
  const float tentacleWaveFreq =
      m_goomInfo.GetSoundInfo().GetAcceleration() < 0.3F
          ? 1.25F
          : (1.0F / (1.10F - m_goomInfo.GetSoundInfo().GetAcceleration()));
  m_currentDriver->MultiplyIterZeroYValWaveFreq(tentacleWaveFreq);

  m_currentDriver->Update(m_half_pi - m_rot, m_distt, m_distt2, modColor, modLowColor);
}

void TentaclesFx::TentaclesImpl::ChangeDominantColor()
{
  assert(m_dominantColorMap);
  const Pixel newColor =
      RandomColorMaps{m_goomRand}.GetRandomColor(*m_dominantColorMap, 0.0F, 1.0F);
  m_dominantColor = IColorMap::GetColorMix(m_dominantColor, newColor, 0.7F);
}

inline auto TentaclesFx::TentaclesImpl::GetModColors() -> std::tuple<Pixel, Pixel>
{
  // IMPORTANT. getEvolvedColor works just right - not sure why
  m_dominantColor = GetEvolvedColor(m_dominantColor);

  const Pixel modColor = GetLightenedColor(m_dominantColor, (m_lig * 2.0F) + 2.0F);
  const Pixel modLowColor = GetLightenedColor(modColor, (m_lig / 2.0F) + 0.67F);

  return std::make_tuple(modColor, modLowColor);
}

void TentaclesFx::TentaclesImpl::PrettyMovePreStart()
{
  m_prePrettyMoveLock =
      m_goomRand.GetRandInRange(MIN_PRE_PRETTY_MOVE_LOCK, MAX_PRE_PRETTY_MOVE_LOCK);
  m_distt2OffsetPreStep =
      stdnew::lerp(DISTT2_MIN, DISTT2_MAX, 0.2F) / static_cast<float>(m_prePrettyMoveLock);
  m_distt2Offset = 0;
}

void TentaclesFx::TentaclesImpl::PrettyMoveStart(const float acceleration, const int32_t timerVal)
{
  if (timerVal != -1)
  {
    m_prettyMoveHappeningTimer = timerVal;
  }
  else
  {
    m_prettyMoveHappeningTimer = static_cast<int>(
        m_goomRand.GetRandInRange(PRETTY_MOVE_HAPPENING_MIN, PRETTY_MOVE_HAPPENING_MAX));
  }
  m_prettyMoveCheckStopMark = m_prettyMoveHappeningTimer / 4;
  m_postPrettyMoveLock = (3 * m_prettyMoveHappeningTimer) / 2;

  m_distt2Offset =
      (1.0F / (1.10F - acceleration)) * m_goomRand.GetRandInRange(DISTT2_MIN, DISTT2_MAX);
  m_rotAtStartOfPrettyMove = m_rot;
  m_cycleInc = m_goomRand.GetRandInRange(CYCLE_INC_MIN, CYCLE_INC_MAX);
}

void TentaclesFx::TentaclesImpl::PrettyMoveFinish()
{
  m_prettyMoveHappeningTimer = 0;
  m_distt2Offset = 0.0;
}

void TentaclesFx::TentaclesImpl::IsPrettyMoveHappeningUpdate(const float acceleration)
{
  // Are we in a prettyMove?
  if (m_prettyMoveHappeningTimer > 0)
  {
    m_prettyMoveHappeningTimer -= 1;
    return;
  }

  // Not in a pretty move. Have we just finished?
  if (m_isPrettyMoveHappening)
  {
    m_isPrettyMoveHappening = false;
    PrettyMoveFinish();
    return;
  }

  // Are we locked after prettyMove finished?
  if (m_postPrettyMoveLock != 0)
  {
    --m_postPrettyMoveLock;
    return;
  }

  // Are we locked after prettyMove start signal?
  if (m_prePrettyMoveLock != 0)
  {
    --m_prePrettyMoveLock;
    m_distt2Offset += m_distt2OffsetPreStep;
    return;
  }

  // Are we ready to start a prettyMove?
  if (m_prettyMoveReadyToStart)
  {
    m_prettyMoveReadyToStart = false;
    m_isPrettyMoveHappening = true;
    PrettyMoveStart(acceleration);
    return;
  }

  // Are we ready to signal a prettyMove start?
  if (StartPrettyMoveEvent())
  {
    m_prettyMoveReadyToStart = true;
    PrettyMovePreStart();
    return;
  }
}

inline auto TentaclesFx::TentaclesImpl::GetNextDriver() const -> TentacleDriver*
{
  const size_t driverIndex = m_driverWeights.GetRandomWeighted();
  return m_drivers[driverIndex].get();
}

inline auto TentaclesFx::TentaclesImpl::GetStableRotationOffset(const float cycleVal) -> float
{
  return (1.5F + (std::sin(cycleVal) / 32.0F)) * m_pi;
}

void TentaclesFx::TentaclesImpl::PrettyMove(const float acceleration)
{
  IsPrettyMoveHappeningUpdate(acceleration);

  if (m_isPrettyMoveHappening && (m_prettyMoveHappeningTimer == m_prettyMoveCheckStopMark))
  {
    m_currentDriver = GetNextDriver();
    assert(m_currentDriver);
  }

  m_distt2 = stdnew::lerp(m_distt2, m_distt2Offset, m_prettyMoveLerpMix);

  // Bigger offset here means tentacles start further back behind screen.
  auto disttOffset = static_cast<float>(stdnew::lerp(
      DISTT_MIN, DISTT_MAX, 0.5 * (1.0 - std::sin(static_cast<double>(m_cycle) * (19.0 / 20.0)))));
  if (m_isPrettyMoveHappening)
  {
    disttOffset *= 0.6F;
  }
  m_distt = stdnew::lerp(m_distt, disttOffset, 4.0F * m_prettyMoveLerpMix);

  float rotOffset = 0.0F;
  if (!m_isPrettyMoveHappening)
  {
    rotOffset = GetStableRotationOffset(m_cycle);
  }
  else
  {
    float currentCycle = m_cycle;
    if (ChangeRotationEvent())
    {
      m_doRotation = TurnRotationOnEvent();
    }
    if (m_doRotation)
    {
      currentCycle *= m_two_pi;
    }
    else
    {
      currentCycle *= -m_pi;
    }
    rotOffset = std::fmod(currentCycle, m_two_pi);
    if (rotOffset < 0.0F)
    {
      rotOffset += m_two_pi;
    }

    if (m_prettyMoveHappeningTimer < m_prettyMoveCheckStopMark)
    {
      // If close enough to initialStableRotationOffset, then stop the happening.
      if (std::fabs(m_rot - m_rotAtStartOfPrettyMove) < 0.1F)
      {
        m_isPrettyMoveHappening = false;
        PrettyMoveFinish();
      }
    }

    if (!((0.0F <= rotOffset) && (rotOffset <= m_two_pi)))
    {
      throw std::logic_error(std20::format(
          "rotOffset {:.3f} not in [0, 2pi]: currentCycle = {:.3f}", rotOffset, currentCycle));
    }
  }

  m_rot = stdnew::clamp(stdnew::lerp(m_rot, rotOffset, m_prettyMoveLerpMix), 0.0F, m_two_pi);
}

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
