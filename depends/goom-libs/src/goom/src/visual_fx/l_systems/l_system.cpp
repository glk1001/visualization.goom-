#include "l_system.h"

//#undef NO_LOGGING

#include "draw/goom_draw.h"
#include "goom_logger.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/t_values.h"

#include <lsys/graphics_generator.h>
#include <lsys/interpret.h>
#include <lsys/list.h>
#include <lsys/module.h>
#include <lsys/parsed_model.h>
#include <memory>
#include <vector>

namespace GOOM::UTILS
{
using MATH::SMALL_FLOAT;

using DefaultParams = ::LSYS::Interpreter::DefaultParams;

template<>
inline auto IncrementedValue<DefaultParams>::LerpValues(const DefaultParams& val1,
                                                        const DefaultParams& val2,
                                                        const float t) noexcept -> DefaultParams
{
  return {
      STD20::lerp(val1.turnAngleInDegrees, val2.turnAngleInDegrees, t),
      STD20::lerp(val1.width, val2.width, t),
      STD20::lerp(val1.distance, val2.distance, t),
  };
}

template<>
inline auto IncrementedValue<DefaultParams>::GetMatchingT(const DefaultParams& val,
                                                          const DefaultParams& val1,
                                                          const DefaultParams& val2) noexcept
    -> float
{
  if (std::fabs(static_cast<float>(val2.turnAngleInDegrees) -
                static_cast<float>(val1.turnAngleInDegrees)) > SMALL_FLOAT)
  {
    return (
        (static_cast<float>(val.turnAngleInDegrees) - static_cast<float>(val1.turnAngleInDegrees)) /
        (static_cast<float>(val2.turnAngleInDegrees) -
         static_cast<float>(val1.turnAngleInDegrees)));
  }
  if (std::fabs(static_cast<float>(val2.width) - static_cast<float>(val1.width)) > SMALL_FLOAT)
  {
    return ((static_cast<float>(val.width) - static_cast<float>(val1.width)) /
            (static_cast<float>(val2.width) - static_cast<float>(val1.width)));
  }
  if (std::fabs(static_cast<float>(val2.distance) - static_cast<float>(val1.distance)) >
      SMALL_FLOAT)
  {
    return ((static_cast<float>(val.distance) - static_cast<float>(val1.distance)) /
            (static_cast<float>(val2.distance) - static_cast<float>(val1.distance)));
  }
  return 0.0F;
}

template<>
// NOLINTNEXTLINE(readability-identifier-naming)
inline auto IncrementedValue<DefaultParams>::clamp(const DefaultParams& val,
                                                   const DefaultParams& val1,
                                                   const DefaultParams& val2) noexcept
    -> DefaultParams
{
  return {std::clamp(val.turnAngleInDegrees, val1.turnAngleInDegrees, val2.turnAngleInDegrees),
          std::clamp(val.width, val1.width, val2.width),
          std::clamp(val.distance, val1.distance, val2.distance)};
}

} // namespace GOOM::UTILS

namespace GOOM::VISUAL_FX::L_SYSTEM
{

using UTILS::MATH::IGoomRand;

using ::LSYS::BoundingBox3d;
using ::LSYS::GetBoundingBox3d;
using ::LSYS::GetFinalProperties;
using ::LSYS::GraphicsGenerator;
using ::LSYS::Interpreter;
using ::LSYS::List;
using ::LSYS::Module;
using ::LSYS::Value;

LSystem::LSystem(DRAW::IGoomDraw& draw,
                 const PluginInfo& goomInfo,
                 const IGoomRand& goomRand,
                 const std::string& lSystemDirectory,
                 const LSystemFile& lSystemFile) noexcept
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_lineDrawerManager{draw, goomRand},
    m_lSysModelSet{GetLSysModelSet(lSystemDirectory, lSystemFile)},
    m_lSysGeometry{goomRand,
                   m_lSysModelSet.lSystemXScale * lSystemFile.overrides.xScale,
                   m_lSysModelSet.lSystemYScale * lSystemFile.overrides.yScale},
    m_lSysDraw{m_lSysGeometry, m_lSysColors, lSystemFile.overrides.lineWidthFactor}
{
}

auto LSystem::GetLSysDrawFuncs() noexcept -> GraphicsGenerator::DrawFuncs
{
  const auto drawLine = [this](const ::LSYS::Vector& point1,
                               const ::LSYS::Vector& point2,
                               const uint32_t lSysColor,
                               const float lineWidth)
  {
    m_lSysDraw.DrawLine(lSysColor, point1, point2, lineWidth);
    m_lineDrawerManager.Update();
  };
  const auto drawPolygon = [this](const std::vector<::LSYS::Vector>& polygon,
                                  const uint32_t lSysColor,
                                  const float lineWidth)
  {
    m_lSysDraw.DrawPolygon(lSysColor, polygon, lineWidth);
    m_lineDrawerManager.Update();
  };

  return {drawLine, drawPolygon};
}

auto LSystem::Start() noexcept -> void
{
  InitNextLSysInterpreter();
  RestartLSysInterpreter();

  m_lSysPath.Init();

  StartBrightnessTimer();
}

auto LSystem::StartBrightnessTimer() noexcept -> void
{
  m_brightnessOnOffTimer.SetActions(
      [this]()
      {
        m_lSysColors.SetGlobalBrightness(ON_BRIGHTNESS);
        return true;
      },
      [this]()
      {
        m_lSysColors.SetGlobalBrightness(OFF_BRIGHTNESS);
        return true;
        ;
      });

  m_brightnessOnOffTimer.StartOnTimer();
}

auto LSystem::Update() noexcept -> void
{
  m_lSysGeometry.SetTranslateAdjust(Vec2dFlt{m_lSysPath.GetNextPathPosition().ToFlt()});

  IncrementTs();

  m_lSysGeometry.UpdateCurrentTransformArray();
}

inline auto LSystem::IncrementTs() noexcept -> void
{
  m_lSysGeometry.IncrementTs();
  m_defaultInterpreterParams.Increment();
  m_timeForThisLSysInterpreter.Increment();
  m_lSysPath.IncrementPositionT();
  m_brightnessOnOffTimer.Increment();
}

auto LSystem::GetLSysModelSet(const std::string& lSysDirectory,
                              const LSystemFile& lSystemFile) const -> LSysModelSet
{
  auto lSysModelSet = LSysModelSet{};

  lSysModelSet.lSysOverrides                = lSystemFile.overrides;
  lSysModelSet.lSysProperties.inputFilename = GetLSystemFilename(lSysDirectory, lSystemFile);
  //LogInfo("L-System file = \"{}\".", lSystemFile.filename);

  const auto boundingBox3d = GetBoundingBox3d(GetBoundsFilename(lSysDirectory, lSystemFile));
  const auto boundingBox2d =
      GetBoundingBox2d(lSysModelSet.lSysOverrides.expandBounds, boundingBox3d);
  //LogInfo("boundingBox2d = ({:.2f}, {:.2f}), ({:.2f}, {:.2f})",
  //        boundingBox2d.min.x,
  //        boundingBox2d.min.y,
  //        boundingBox2d.max.x,
  //        boundingBox2d.max.y);

  lSysModelSet.lSystemXScale = static_cast<float>(m_goomInfo->GetScreenWidth()) /
                               (boundingBox2d.max.x - boundingBox2d.min.x);
  //TODO(glk) How to handle scale issues
  lSysModelSet.lSystemYScale = static_cast<float>(m_goomInfo->GetScreenHeight()) /
                               (boundingBox2d.max.y - boundingBox2d.min.y);

  lSysModelSet.lSysModel = GetParsedModel(lSysModelSet.lSysProperties);
  lSysModelSet.lSysProperties =
      GetFinalProperties(lSysModelSet.lSysModel->GetSymbolTable(), lSysModelSet.lSysProperties);
  //LogInfo("L-System properties.maxGen = {}.", lSysModelSet.lSysProperties.maxGen);
  //LogInfo("L-System properties.lineWidth = {}.", lSysModelSet.lSysProperties.lineWidth);
  //LogInfo("L-System properties.lineDistance = {}.", lSysModelSet.lSysProperties.lineDistance);
  //LogInfo("L-System properties.turnAngle = {}.", lSysModelSet.lSysProperties.turnAngle);

  return lSysModelSet;
}

auto LSystem::UpdateLSysModel() noexcept -> void
{
  //LogInfo("Update L-System model.");

  if (static constexpr auto PROB_NEW_LSYS_INTERPRETER = 0.5F;
      m_timeForThisLSysInterpreter.Finished() and
      m_goomRand->ProbabilityOf(PROB_NEW_LSYS_INTERPRETER))
  {
    InitNextLSysInterpreter();
  }
  else
  {
    ResetLSysParams();
  }

  RestartLSysInterpreter();
}

auto LSystem::InitNextLSysInterpreter() -> void
{
  //LogInfo("Init next interpreter.");

  m_lSysModuleList = nullptr;

  const auto numLSysCopies =
      m_goomRand->GetRandInRange(m_lSysModelSet.lSysOverrides.minNumLSysCopies,
                                 m_lSysModelSet.lSysOverrides.maxNumLSysCopies + 1U);

  m_lSysDraw.SetNumLSysCopies(numLSysCopies);
  SwitchLineDrawers();

  m_lSysGeometry.SetNumLSysCopies(numLSysCopies);
  m_lSysGeometry.SetVerticalMoveMaxMin(m_lSysModelSet.lSysOverrides.verticalMoveMin,
                                       m_lSysModelSet.lSysOverrides.verticalMoveMax);
  m_lSysGeometry.SetVerticalMoveNumSteps(
      m_goomRand->GetRandInRange(MIN_VERTICAL_MOVE_STEPS, MAX_VERTICAL_MOVE_STEPS + 1U));
  m_lSysGeometry.SetYScaleNumSteps(
      m_goomRand->GetRandInRange(MIN_Y_SCALE_ADJUST_STEPS, MAX_Y_SCALE_ADJUST_STEPS + 1U));
  m_lSysGeometry.SetRotateDegreesAdjustNumSteps(
      m_goomRand->GetRandInRange(m_lSysModelSet.lSysOverrides.minNumRotateDegreeSteps,
                                 m_lSysModelSet.lSysOverrides.maxNumRotateDegreeSteps + 1));

  m_lSysPath.SetPathNumSteps(
      m_goomRand->GetRandInRange(MIN_PATH_NUM_STEPS, MAX_PATH_NUM_STEPS + 1U));

  m_lSysColors.SetProbabilityOfSimpleColors(m_lSysModelSet.lSysOverrides.probabilityOfSimpleColors);
  m_lSysColors.SetNumColors(numLSysCopies);
  m_lSysColors.ChangeColors();

  m_maxGen = m_goomRand->GetRandInRange(m_lSysModelSet.lSysOverrides.minMaxGen,
                                        m_lSysModelSet.lSysOverrides.maxMaxGen + 1U);

  SetNewDefaultInterpreterParams();
  ResetModelNamedArgs();

  m_lSysInterpreter = std::make_unique<Interpreter>(*m_LSysGenerator);
  m_lSysInterpreter->SetDefaults(m_defaultInterpreterParams());

  m_timeForThisLSysInterpreter.ResetToZero();
}

inline auto LSystem::RegenerateModuleList() noexcept -> void
{
  //LogInfo("Regenerate module list.");

  m_lSysModuleList =
      std::make_unique<List<Module>>(*m_lSysModelSet.lSysModel->GetStartModuleList());

  for (auto gen = 0U; gen < m_maxGen; ++gen)
  {
    m_lSysModuleList = m_lSysModelSet.lSysModel->Generate(m_lSysModuleList.get());
  }

  Ensures(m_lSysModuleList != nullptr);

  //LogInfo("Regenerated module list - num = {}.", m_lSysModuleList->size());
}

inline auto LSystem::RestartLSysInterpreter() noexcept -> void
{
  //LogInfo("Restart interpreter.");
  RegenerateModuleList();
  m_lSysInterpreter->Start(*m_lSysModuleList);
}

inline auto LSystem::UpdateInterpreterParams() noexcept -> void
{
  if (not m_defaultInterpreterParams.GetT().IsStopped())
  {
    return;
  }

  if (static constexpr auto PROB_NEW_DEFAULT_PARAMS = 0.5F;
      m_goomRand->ProbabilityOf(PROB_NEW_DEFAULT_PARAMS))
  {
    m_defaultInterpreterParams.SetValues(m_defaultInterpreterParams(),
                                         GetRandomDefaultInterpreterParams());
    ResetModelNamedArgs();
  }
  else
  {
    m_defaultInterpreterParams.ReverseValues();
  }

  static constexpr auto RESET_START_T = 0.1F;
  m_defaultInterpreterParams.ResetT(RESET_START_T);
  m_defaultInterpreterParams.Increment();
}

inline auto LSystem::SetNewDefaultInterpreterParams() noexcept -> void
{
  const auto currentDefaultParams = GetRandomDefaultInterpreterParams();
  const auto nextDefaultParams    = GetRandomDefaultInterpreterParams();
  m_defaultInterpreterParams.SetValues(currentDefaultParams, nextDefaultParams);
  m_defaultInterpreterParams.ResetT();
}

inline auto LSystem::GetRandomDefaultInterpreterParams() const noexcept
    -> Interpreter::DefaultParams
{
  return {
      m_lSysModelSet.lSysProperties.turnAngle *
          m_goomRand->GetRandInRange(
              m_lSysModelSet.lSysOverrides.minDefaultTurnAngleInDegreesFactor,
              m_lSysModelSet.lSysOverrides.maxDefaultTurnAngleInDegreesFactor),
      m_lSysModelSet.lSysProperties.lineWidth *
          m_goomRand->GetRandInRange(m_lSysModelSet.lSysOverrides.minDefaultLineWidthFactor,
                                     m_lSysModelSet.lSysOverrides.maxDefaultLineWidthFactor),
      m_lSysModelSet.lSysProperties.lineDistance *
          m_goomRand->GetRandInRange(m_lSysModelSet.lSysOverrides.minDefaultDistanceFactor,
                                     m_lSysModelSet.lSysOverrides.maxDefaultDistanceFactor),
  };
}

inline auto LSystem::ResetLSysParams() noexcept -> void
{
  //LogInfo("Reset lsys params.");

  if (static constexpr auto PROB_CHANGE_ROTATE_DIRECTION = 0.001F;
      m_goomRand->ProbabilityOf(PROB_CHANGE_ROTATE_DIRECTION))
  {
    m_lSysGeometry.ReverseRotateDirection();
  }

  UpdateInterpreterParams();

  m_lSysInterpreter->SetDefaults(m_defaultInterpreterParams());
  //LogInfo("L-System default width = {}.", m_defaultInterpreterParams().width);
  //LogInfo("L-System default distance = {}.", m_defaultInterpreterParams().distance);
  //LogInfo("L-System default turnAngleInDegrees = {}.",
  //        m_defaultInterpreterParams().turnAngleInDegrees);
}

inline auto LSystem::GetLSystemFilename(const std::string& lSystemDirectory,
                                        const LSystemFile& lSystemFile) noexcept -> std::string
{
  return lSystemDirectory + "/" + lSystemFile.filename + ".in";
}

inline auto LSystem::GetBoundsFilename(const std::string& lSystemDirectory,
                                       const LSystemFile& lSystemFile) noexcept -> std::string
{
  return lSystemDirectory + "/" + lSystemFile.filename + ".bnds";
}

inline auto LSystem::ResetModelNamedArgs() -> void
{
  if (m_lSysModelSet.lSysOverrides.namedArgs.empty())
  {
    return;
  }

  for (const auto& namedArg : m_lSysModelSet.lSysOverrides.namedArgs)
  {
    m_lSysModelSet.lSysModel->ResetArgument(
        namedArg.name, Value{m_goomRand->GetRandInRange(namedArg.min, namedArg.max)});
  }
}

auto LSystem::DrawLSystem() noexcept -> void
{
  //LogInfo("Start L-System interpreted draw. Num modules = {}.", m_lSysModuleList->size());

  DrawLSystemBatch();
  UpdateLSysModel();
}

inline auto LSystem::DrawLSystemBatch() noexcept -> void
{
  const auto numModules = m_lSysModuleList->size();
  for (auto i = 0U; i < numModules; ++i)
  {
    if (not m_lSysInterpreter->InterpretNext())
    {
      //LogInfo("Unexpected finish to interpreting modules.");
      FailFast();
    }
  }

  Expects(not m_lSysInterpreter->InterpretNext());
  //LogInfo("Finished interpreting all modules.");
}

inline auto LSystem::GetBoundingBox2d(const float expandBounds,
                                      const BoundingBox3d& boundingBox3d) noexcept -> BoundingBox2d
{
  auto boundingBox2d =
      BoundingBox2d{expandBounds * LSysDraw::GetPerspectivePoint(boundingBox3d.min),
                    expandBounds * LSysDraw::GetPerspectivePoint(boundingBox3d.max)};
  return boundingBox2d;
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
