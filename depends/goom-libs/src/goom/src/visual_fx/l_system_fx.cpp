#include "l_system_fx.h"

//#undef NO_LOGGING

#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom_logger.h"
#include "goom_plugin_info.h"
#include "l_systems/l_system.h"
#include "point2d.h"
#include "spimpl.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/timer.h"

#include <cstdint>
#include <lsys/rand.h>
#include <memory>
#include <string>
#include <vector>

namespace GOOM::VISUAL_FX
{

using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using L_SYSTEM::LSystem;
using UTILS::Timer;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;

using ::LSYS::SetRandFunc;

class LSystemFx::LSystemFxImpl
{
public:
  LSystemFxImpl(const FxHelper& fxHelper,
                const SmallImageBitmaps& smallBitmaps,
                const std::string& resourcesDirectory);

  auto Start() -> void;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto Resume() -> void;

  auto ApplyMultiple() -> void;

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  const SmallImageBitmaps& m_smallBitmaps;
  const std::string m_resourcesDirectory;
  const Point2dInt m_screenMidpoint{UTILS::MATH::U_HALF * m_goomInfo.GetScreenWidth(),
                                    UTILS::MATH::U_HALF* m_goomInfo.GetScreenHeight()};

  SmallImageBitmaps::ImageNames m_currentBitmapName{};
  [[nodiscard]] auto GetImageBitmap(uint32_t size) const -> const ImageBitmap&;
  static constexpr auto MIN_DOT_SIZE = 5U;
  static constexpr auto MAX_DOT_SIZE = 17U;
  static_assert(MAX_DOT_SIZE <= SmallImageBitmaps::MAX_IMAGE_SIZE, "Max dot size mismatch.");

  auto Update() noexcept -> void;
  auto ChangeColors() noexcept -> void;

  [[nodiscard]] static auto GetLSystemFileList() noexcept -> std::vector<LSystem::LSystemFile>;
  static inline const std::vector<LSystem::LSystemFile> L_SYS_FILE_LIST = GetLSystemFileList();
  std::vector<std::unique_ptr<LSystem>> m_lSystems                      = GetLSystems();
  [[nodiscard]] auto GetLSystems() const noexcept -> std::vector<std::unique_ptr<LSystem>>;
  static constexpr auto DEFAULT_BOUNDS_EXPAND_FACTOR = 2.0F;
  [[nodiscard]] auto GetLSystemDirectory() const noexcept -> std::string;

  std::vector<LSystem*> m_activeLSystems{m_lSystems.at(0).get()};
  static constexpr auto MIN_TIME_TO_KEEP_ACTIVE_LSYS = 200U;
  static constexpr auto MAX_TIME_TO_KEEP_ACTIVE_LSYS = 1000U;
  Timer m_timeForTheseActiveLSys{
      m_goomRand.GetRandInRange(MIN_TIME_TO_KEEP_ACTIVE_LSYS, MAX_TIME_TO_KEEP_ACTIVE_LSYS + 1U)};

  static constexpr auto MIN_NUM_ROTATE_DEGREES_STEPS = 50U;
  static constexpr auto MAX_NUM_ROTATE_DEGREES_STEPS = 500U;
  static constexpr auto MAX_VERTICAL_MOVE            = +100.0F;
  static constexpr auto MIN_VERTICAL_MOVE            = -100.0F;
  auto InitNextActiveLSystems() noexcept -> void;
  auto DrawLSystem() noexcept -> void;
};

LSystemFx::LSystemFx(const FxHelper& fxHelper,
                     const SmallImageBitmaps& smallBitmaps,
                     const std::string& resourcesDirectory) noexcept
  : m_pimpl{spimpl::make_unique_impl<LSystemFxImpl>(fxHelper, smallBitmaps, resourcesDirectory)}
{
}

auto LSystemFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto LSystemFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto LSystemFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto LSystemFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto LSystemFx::Finish() noexcept -> void
{
  // nothing to do
}

auto LSystemFx::Resume() noexcept -> void
{
  m_pimpl->Resume();
}

auto LSystemFx::Suspend() noexcept -> void
{
  // nothing to do
}

auto LSystemFx::GetFxName() const noexcept -> std::string
{
  return "L-System";
}

auto LSystemFx::ApplyMultiple() noexcept -> void
{
  m_pimpl->ApplyMultiple();
}

LSystemFx::LSystemFxImpl::LSystemFxImpl(const FxHelper& fxHelper,
                                        const SmallImageBitmaps& smallBitmaps,
                                        const std::string& resourcesDirectory)
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_smallBitmaps{smallBitmaps},
    m_resourcesDirectory{resourcesDirectory}
{
}

auto LSystemFx::LSystemFxImpl::GetLSystemFileList() noexcept -> std::vector<LSystem::LSystemFile>
{
  // TODO(glk) With C++20, can use constexpr here.
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  auto lSysFileList = std::vector{
      LSystem::LSystemFile{        "bourke_bush",
                           {
                           /*.minNumLSysCopies =*/1U,
                           /*.maxNumLSysCopies =*/5U,
                           /*.minMaxGen =*/3U,
                           /*.maxMaxGen =*/3U,
                           /*.lineWidthFactor =*/1.0F,
                           /*.expandBounds =*/DEFAULT_BOUNDS_EXPAND_FACTOR,
                           /*.minNumRotateDegreeSteps =*/MIN_NUM_ROTATE_DEGREES_STEPS,
                           /*.maxNumRotateDegreeSteps =*/MAX_NUM_ROTATE_DEGREES_STEPS,
                           /*.xScale =*/1.0F,
                           /*.yScale =*/0.5F,
                           /*.verticalMoveMin =*/MIN_VERTICAL_MOVE,
                           /*.verticalMoveMax =*/2.5F * MAX_VERTICAL_MOVE,
                           /*.minDefaultLineWidthFactor =*/1.0F,
                           /*.maxDefaultLineWidthFactor =*/2.01F,
                           /*.minDefaultDistanceFactor =*/1.0F,
                           /*.maxDefaultDistanceFactor =*/3.01F,
                           /*.minDefaultTurnAngleInDegreesFactor =*/0.1F,
                           /*.maxDefaultTurnAngleInDegreesFactor =*/1.0F,
                           /*.probabilityOfSimpleColors =*/0.75F,
                           /*.probabilityOfNoise =*/0.95F,
                           /*.namedArgs = */ {},
                           }},
      LSystem::LSystemFile{"bourke_pentaplexity",
                           {
                           /*.minNumLSysCopies =*/1U,
                           /*.maxNumLSysCopies =*/5U,
                           /*.minMaxGen =*/2U,
                           /*.maxMaxGen =*/3U,
                           /*.lineWidthFactor =*/1.0F,
                           /*.expandBounds =*/DEFAULT_BOUNDS_EXPAND_FACTOR,
                           /*.minNumRotateDegreeSteps =*/MIN_NUM_ROTATE_DEGREES_STEPS,
                           /*.maxNumRotateDegreeSteps =*/MAX_NUM_ROTATE_DEGREES_STEPS,
                           /*.xScale =*/1.0F,
                           /*.yScale =*/1.0F,
                           /*.verticalMoveMin =*/MIN_VERTICAL_MOVE,
                           /*.verticalMoveMax =*/2.5F * MAX_VERTICAL_MOVE,
                           /*.minDefaultLineWidthFactor =*/1.0F,
                           /*.maxDefaultLineWidthFactor =*/2.01F,
                           /*.minDefaultDistanceFactor =*/0.2F,
                           /*.maxDefaultDistanceFactor =*/1.1F,
                           /*.minDefaultTurnAngleInDegreesFactor =*/1.0F,
                           /*.maxDefaultTurnAngleInDegreesFactor =*/1.00001F,
                           /*.probabilityOfSimpleColors =*/0.8F,
                           /*.probabilityOfNoise =*/0.95F,
                           /*.namedArgs = */ {},
                           }},
      LSystem::LSystemFile{       "honda_tree_b",
                           {
                           /*.minNumLSysCopies =*/2U,
                           /*.maxNumLSysCopies =*/8U,
                           /*.minMaxGen =*/5U,
                           /*.maxMaxGen =*/10U,
                           /*.lineWidthFactor =*/1.0F,
                           /*.expandBounds =*/DEFAULT_BOUNDS_EXPAND_FACTOR,
                           /*.minNumRotateDegreeSteps =*/MIN_NUM_ROTATE_DEGREES_STEPS,
                           /*.maxNumRotateDegreeSteps =*/MAX_NUM_ROTATE_DEGREES_STEPS,
                           /*.xScale =*/1.0F,
                           /*.yScale =*/0.5F,
                           /*.verticalMoveMin =*/MIN_VERTICAL_MOVE,
                           /*.verticalMoveMax =*/2.5F * MAX_VERTICAL_MOVE,
                           /*.minDefaultLineWidthFactor =*/1.0F,
                           /*.maxDefaultLineWidthFactor =*/2.01F,
                           /*.minDefaultDistanceFactor =*/1.0F,
                           /*.maxDefaultDistanceFactor =*/2.01F,
                           /*.minDefaultTurnAngleInDegreesFactor =*/1.0F,
                           /*.maxDefaultTurnAngleInDegreesFactor =*/1.01F,
                           /*.probabilityOfSimpleColors =*/0.8F,
                           /*.probabilityOfNoise =*/0.95F,
                           /*.namedArgs = */
                           {
                           {"r2", 0.7F, 0.9F},
                           {"a0", 30.0F, 30.01F},
                           {"a2", 30.0F, 30.01F},
                           {"d", 137.5F, 137.51F},
                           },
                           }},
      LSystem::LSystemFile{     "ternary_tree_a",
                           {
                           /*.minNumLSysCopies =*/2U,
                           /*.maxNumLSysCopies =*/5U,
                           /*.minMaxGen =*/5U,
                           /*.maxMaxGen =*/6U,
                           /*.lineWidthFactor =*/1.1F,
                           /*.expandBounds =*/DEFAULT_BOUNDS_EXPAND_FACTOR,
                           /*.minNumRotateDegreeSteps =*/MIN_NUM_ROTATE_DEGREES_STEPS,
                           /*.maxNumRotateDegreeSteps =*/MAX_NUM_ROTATE_DEGREES_STEPS,
                           /*.xScale =*/0.8F,
                           /*.yScale =*/1.0F,
                           /*.verticalMoveMin =*/2.0F * MIN_VERTICAL_MOVE,
                           /*.verticalMoveMax =*/MAX_VERTICAL_MOVE,
                           /*.minDefaultLineWidthFactor =*/1.0F,
                           /*.maxDefaultLineWidthFactor =*/1.01F,
                           /*.minDefaultDistanceFactor =*/1.0F,
                           /*.maxDefaultDistanceFactor =*/1.01F,
                           /*.minDefaultTurnAngleInDegreesFactor =*/1.0F,
                           /*.maxDefaultTurnAngleInDegreesFactor =*/1.01F,
                           /*.probabilityOfSimpleColors =*/0.8F,
                           /*.probabilityOfNoise =*/0.0F, // too slow with noise
                           /*.namedArgs = */
                           {
                           {"lr", 1.109F, 1.209F},
                           },
                           }},
  };
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

  return lSysFileList;
}

auto LSystemFx::LSystemFxImpl::GetLSystems() const noexcept -> std::vector<std::unique_ptr<LSystem>>
{
  auto lSystem = std::vector<std::unique_ptr<LSystem>>{};

  for (const auto& lSysFile : L_SYS_FILE_LIST)
  {
    lSystem.emplace_back(
        std::make_unique<LSystem>(m_draw, m_goomInfo, m_goomRand, GetLSystemDirectory(), lSysFile));
  }

  return lSystem;
}

auto LSystemFx::LSystemFxImpl::InitNextActiveLSystems() noexcept -> void
{
  //LogInfo("Setting new active l-systems.");

  m_activeLSystems.clear();
  const auto lSystemIndex =
      m_goomRand.GetRandInRange(0U, static_cast<uint32_t>(L_SYS_FILE_LIST.size()));
  //  const auto lSystemIndex = 2U;
  // m_activeLSystems.push_back(m_lSystems.at(lSystemIndex).get());
  m_activeLSystems.push_back(m_lSystems.at(lSystemIndex).get());
  m_timeForTheseActiveLSys.SetTimeLimit(
      m_goomRand.GetRandInRange(MIN_TIME_TO_KEEP_ACTIVE_LSYS, MAX_TIME_TO_KEEP_ACTIVE_LSYS + 1U));
}

auto LSystemFx::LSystemFxImpl::Start() -> void
{
  SetRandFunc([this]() { return m_goomRand.GetRandInRange(0.0, 1.0); });

  std::for_each(begin(m_lSystems),
                end(m_lSystems),
                [this](auto& lSystem)
                {
                  lSystem->SetPathStart(m_screenMidpoint);
                  lSystem->Start();
                });

  InitNextActiveLSystems();
}

inline auto LSystemFx::LSystemFxImpl::GetImageBitmap(const uint32_t size) const
    -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(m_currentBitmapName,
                                       std::clamp(size, MIN_DOT_SIZE, MAX_DOT_SIZE));
}

inline auto LSystemFx::LSystemFxImpl::ChangeColors() noexcept -> void
{
  //LogInfo("Changing colors.");
  std::for_each(begin(m_activeLSystems),
                end(m_activeLSystems),
                [](auto* lSystem) { lSystem->ChangeColors(); });
}

inline auto LSystemFx::LSystemFxImpl::GetCurrentColorMapsNames() const noexcept
    -> std::vector<std::string>
{
  return {};
}

inline auto LSystemFx::LSystemFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  std::for_each(begin(m_activeLSystems),
                end(m_activeLSystems),
                [&weightedColorMaps](auto* lSystem)
                { lSystem->SetWeightedColorMaps(weightedColorMaps); });

  //ChangeColors();
}

inline auto LSystemFx::LSystemFxImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept
    -> void
{
  LogInfo("Setting new zoom midpoint ({}, {}).", zoomMidpoint.x, zoomMidpoint.y);
  std::for_each(begin(m_activeLSystems),
                end(m_activeLSystems),
                [&zoomMidpoint](auto* lSystem) { lSystem->SetPathTarget(zoomMidpoint); });
}

inline auto LSystemFx::LSystemFxImpl::Resume() -> void
{
  InitNextActiveLSystems();
}

inline auto LSystemFx::LSystemFxImpl::ApplyMultiple() -> void
{
  Update();
  DrawLSystem();
}

inline auto LSystemFx::LSystemFxImpl::Update() noexcept -> void
{
  //LogInfo("Doing update.");

  if (static constexpr auto PROB_CHANGE_COLORS = 0.01F;
      (0 == m_goomInfo.GetSoundEvents().GetTimeSinceLastGoom()) or
      m_goomRand.ProbabilityOf(PROB_CHANGE_COLORS))
  {
    ChangeColors();
  }

  m_timeForTheseActiveLSys.Increment();
  if (m_timeForTheseActiveLSys.Finished())
  {
    //LogInfo("Active l-system time finished.");
    InitNextActiveLSystems();
  }

  std::for_each(
      begin(m_activeLSystems), end(m_activeLSystems), [](auto* lSystem) { lSystem->Update(); });
}

inline auto LSystemFx::LSystemFxImpl::DrawLSystem() noexcept -> void
{
  //LogInfo("Start L-System draw.");

  std::for_each(begin(m_activeLSystems),
                end(m_activeLSystems),
                [](auto* lSystem) { lSystem->DrawLSystem(); });
}

inline auto LSystemFx::LSystemFxImpl::GetLSystemDirectory() const noexcept -> std::string
{
  return m_resourcesDirectory + "/l-systems";
}

} // namespace GOOM::VISUAL_FX
