#include "l_system_fx.h"

//#undef NO_LOGGING

#include "color/color_adjustment.h"
#include "color/color_maps.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "generator.h"
#include "goom_plugin_info.h"
#include "graphics_generator.h"
#include "interpret.h"
#include "l_sys_model.h"
#include "list.h"
#include "logging.h"
#include "module.h"
#include "parsed_model.h"
#include "point2d.h"
#include "rand.h"
#include "spimpl.h"
#include "utils/array_utils.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/parametric_functions2d.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"

#include <array>
#include <cstdint>
#include <memory>

namespace GOOM::VISUAL_FX
{

using COLOR::ColorAdjustment;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using DRAW::MakePixels;
using DRAW::MultiplePixels;
using UTILS::CreateArray;
using UTILS::IncrementedValue;
using UTILS::Logging; // NOLINT(misc-unused-using-decls)
using UTILS::TValue;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::DEGREES_360;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::OscillatingFunction;
using UTILS::MATH::OscillatingPath;
using UTILS::MATH::ToRadians;
using UTILS::MATH::U_HALF;

using L_SYSTEM::BoundingBox3d;
using L_SYSTEM::GetBoundingBox3d;
using L_SYSTEM::GetFinalProperties;
using L_SYSTEM::GraphicsGenerator;
using L_SYSTEM::Interpreter;
using L_SYSTEM::List;
using L_SYSTEM::LSysModel;
using L_SYSTEM::Module;
using L_SYSTEM::Point3dFlt;
using L_SYSTEM::Properties;
using L_SYSTEM::SetRandFunc;

struct BoundingBox2d
{
  GOOM::Point2dFlt min;
  GOOM::Point2dFlt max;
};
struct PolygonInfo
{
  std::vector<L_SYSTEM::Vector> polygon;
  int color;
  float lineWidth;
};

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

  auto ApplyMultiple() -> void;

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  const SmallImageBitmaps& m_smallBitmaps;
  const std::string m_resourcesDirectory;
  const Point2dInt m_screenMidpoint{U_HALF * m_goomInfo.GetScreenWidth(),
                                    U_HALF* m_goomInfo.GetScreenHeight()};

  SmallImageBitmaps::ImageNames m_currentBitmapName{};
  [[nodiscard]] auto GetImageBitmap(uint32_t size) const -> const ImageBitmap&;
  static constexpr auto MIN_DOT_SIZE = 5U;
  static constexpr auto MAX_DOT_SIZE = 17U;
  static_assert(MAX_DOT_SIZE <= SmallImageBitmaps::MAX_IMAGE_SIZE, "Max dot size mismatch.");

  static constexpr auto NUM_LSYS_COPIES = 5U;
  std::shared_ptr<const RandomColorMaps> m_mainColorMaps{};
  std::shared_ptr<const RandomColorMaps> m_lowColorMaps{};
  std::array<const IColorMap*, NUM_LSYS_COPIES> m_currentMainColorMaps{};
  std::array<const IColorMap*, NUM_LSYS_COPIES> m_currentLowColorMaps{};
  static constexpr auto MIN_NUM_COLOR_STEPS = 1000U;
  static constexpr auto MAX_NUM_COLOR_STEPS = 1000000U;
  std::array<TValue, NUM_LSYS_COPIES> m_currentColorTs{CreateArray<TValue, NUM_LSYS_COPIES>(
      TValue{TValue::StepType::CONTINUOUS_REVERSIBLE, MAX_NUM_COLOR_STEPS})};
  static constexpr auto GAMMA = 1.0F / 2.2F;
  const ColorAdjustment m_colorAdjust{GAMMA, ColorAdjustment::INCREASED_CHROMA_FACTOR};

  auto Update() noexcept -> void;
  auto ChangeColors() noexcept -> void;

  Properties m_LSysProperties{};
  std::unique_ptr<LSysModel> m_LSysModel{};
  std::unique_ptr<GraphicsGenerator> m_LSysGenerator{};
  std::unique_ptr<List<Module>> m_moduleList{};
  std::unique_ptr<Interpreter> m_LSysInterpreter{};
  Interpreter::DefaultParams m_defaultInterpreterParams{};
  uint32_t m_numModulesPerUpdate = 0U;
  struct LSystemFile
  {
    std::string mainFilename;
    std::string boundsFilename;
  };
  auto SetupNextLSysModel() -> void;
  auto SetupLSysModel(const LSystemFile& lSystemFile) -> void;
  auto SetNewLSysInterpreter() noexcept -> void;
  auto RestartLSysInterpreter() noexcept -> void;
  [[nodiscard]] auto ResetDefaultInterpreterParams() noexcept -> bool;
  [[nodiscard]] auto GetNextLSystemFile() const noexcept -> LSystemFile;
  [[nodiscard]] auto GetLSystemDirectory() const noexcept -> std::string;
  GraphicsGenerator::DrawFuncs m_drawFuncs;

  Point2dInt m_centreStart{};
  Point2dInt m_centreTarget{};
  struct PathsToAndFrom
  {
    OscillatingPath pathToTarget;
    OscillatingPath pathFromTarget;
  };
  std::unique_ptr<PathsToAndFrom> m_pathsToAndFrom = GetPathsToAndFrom();
  [[nodiscard]] auto GetPathsToAndFrom() const noexcept -> std::unique_ptr<PathsToAndFrom>;
  [[nodiscard]] auto GetPathParams() const noexcept -> OscillatingFunction::Params;
  enum class Direction
  {
    TO_TARGET,
    FROM_TARGET,
  };
  Direction m_direction  = Direction::TO_TARGET;
  bool m_newZoomMidpoint = false;
  auto IncrementPositionT() noexcept -> void;
  [[nodiscard]] auto GetNextPathPosition() const noexcept -> Point2dInt;

  const Vec2dFlt m_lSystemStartPoint{m_screenMidpoint.ToFlt()};
  float m_LSystemXScale = 1.0F;
  float m_LSystemYScale = 1.0F;
  struct TransformAdjust
  {
    float xScale;
    float yScale;
    float rotateDegrees;
    Vec2dFlt translate;
  };
  const std::array<TransformAdjust, NUM_LSYS_COPIES> m_transformAdjustArray =
      GetTransformAdjustArray();
  [[nodiscard]] auto GetTransformAdjustArray() const noexcept
      -> std::array<TransformAdjust, NUM_LSYS_COPIES>;
  Vec2dFlt m_translateAdjust{};
  float m_rotateSign = +1.0F;

  static constexpr auto NUM_ROTATE_DEGREES_STEPS = 200U;
  IncrementedValue<float> m_rotateDegreesAdjust{
      0.0F, DEGREES_360, TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_ROTATE_DEGREES_STEPS};

  static constexpr auto MAX_Y_SCALE_ADJUST       = 1.9F;
  static constexpr auto MIN_Y_SCALE_ADJUST       = 1.0F;
  static constexpr auto NUM_Y_SCALE_ADJUST_STEPS = 300U;
  IncrementedValue<float> m_yScaleAdjust{MIN_Y_SCALE_ADJUST,
                                         MAX_Y_SCALE_ADJUST,
                                         TValue::StepType::CONTINUOUS_REVERSIBLE,
                                         NUM_Y_SCALE_ADJUST_STEPS};

  static constexpr auto MAX_VERTICAL_MOVE       = +100.0F;
  static constexpr auto MIN_VERTICAL_MOVE       = -100.0F;
  static constexpr auto NUM_VERTICAL_MOVE_STEPS = 50U;
  IncrementedValue<float> m_verticalMove{MIN_VERTICAL_MOVE,
                                         MAX_VERTICAL_MOVE,
                                         TValue::StepType::CONTINUOUS_REVERSIBLE,
                                         NUM_VERTICAL_MOVE_STEPS};

  auto IncrementTs() noexcept -> void;

  auto DrawLSystem() noexcept -> void;
  auto DrawCachedLSystem() noexcept -> void;
  auto DrawInterpretedLSystem() noexcept -> void;
  [[nodiscard]] auto DrawLSystemBatch() noexcept -> bool;
  bool m_cachedPolygonsReady = false;
  std::vector<PolygonInfo> m_cachedPolygons{};
  auto DrawAndCacheLine(const L_SYSTEM::Vector& point1,
                        const L_SYSTEM::Vector& point2,
                        int color,
                        float lineWidth) noexcept -> void;
  auto DrawLine(const L_SYSTEM::Vector& point1,
                const L_SYSTEM::Vector& point2,
                int color,
                float lineWidth) noexcept -> void;
  auto DrawAndCachePolygon(const std::vector<L_SYSTEM::Vector>& polygon,
                           int color,
                           float lineWidth) noexcept -> void;
  auto DrawPolygon(const std::vector<L_SYSTEM::Vector>& polygon,
                   int color,
                   float lineWidth) noexcept -> void;
  [[nodiscard]] auto GetTransformedPoint(const Point2dFlt& point,
                                         const TransformAdjust& transformAdjust) const noexcept
      -> Point2dFlt;
  auto DrawJoinedVertices(const std::vector<Point3dFlt>& vertices, uint8_t lineWidth) noexcept
      -> void;
  static constexpr auto NUM_LINE_CHUNKS = 1U;
  auto DrawChunkedLine(const Point2dFlt& point1,
                       const Point2dFlt& point2,
                       uint32_t copyNum,
                       uint8_t lineWidth) noexcept -> void;
  [[nodiscard]] auto GetColors(uint32_t copyNum) const noexcept -> MultiplePixels;
  [[nodiscard]] static auto GetPerspectiveProjection(
      const std::vector<Point3dFlt>& points3d) noexcept -> std::vector<Point2dFlt>;
  [[nodiscard]] static auto GetPerspectivePoint(const Point3dFlt& point3d) noexcept -> Point2dFlt;
  [[nodiscard]] static auto GetPolygon3dFlt(const std::vector<L_SYSTEM::Vector>& polygon) noexcept
      -> std::vector<Point3dFlt>;
  [[nodiscard]] static auto GetPoint3dFlt(const L_SYSTEM::Vector& point) noexcept -> Point3dFlt;
  [[nodiscard]] static auto GetBoundingBox2d(const BoundingBox3d& boundingBox3d) noexcept
      -> BoundingBox2d;
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
  // nothing to do
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

auto LSystemFx::LSystemFxImpl::GetTransformAdjustArray() const noexcept
    -> std::array<TransformAdjust, NUM_LSYS_COPIES>
{
  auto transformAdjustArray = std::array<TransformAdjust, NUM_LSYS_COPIES>{};

  auto t = TValue{TValue::StepType::SINGLE_CYCLE, NUM_LSYS_COPIES};

  for (auto& transformAdjust : transformAdjustArray)
  {
    transformAdjust.rotateDegrees = t() * DEGREES_360;
    transformAdjust.xScale        = m_goomRand.GetRandInRange(0.9F, 1.1F);
    transformAdjust.yScale        = m_goomRand.GetRandInRange(0.9F, 1.1F);
    transformAdjust.translate     = {0.0F, 0.0F};

    t.Increment();
  }

  return transformAdjustArray;
}

auto LSystemFx::LSystemFxImpl::GetPathsToAndFrom() const noexcept -> std::unique_ptr<PathsToAndFrom>
{
  static constexpr auto NUM_PATH_STEPS = 100U;
  const auto pathParams                = GetPathParams();

  auto pathsToAndFrom = PathsToAndFrom{
      OscillatingPath{std::make_unique<TValue>(TValue::StepType::SINGLE_CYCLE, NUM_PATH_STEPS),
                      m_centreStart.ToFlt(),
                      m_centreTarget.ToFlt(),
                      pathParams},
      OscillatingPath{std::make_unique<TValue>(TValue::StepType::SINGLE_CYCLE, NUM_PATH_STEPS),
                      m_centreTarget.ToFlt(),
                      m_centreStart.ToFlt(),
                      pathParams}
  };

  return std::make_unique<PathsToAndFrom>(std::move(pathsToAndFrom));
}

auto LSystemFx::LSystemFxImpl::GetPathParams() const noexcept -> OscillatingFunction::Params
{
  static constexpr auto MIN_PATH_AMPLITUDE = 90.0F;
  static constexpr auto MAX_PATH_AMPLITUDE = 110.0F;
  static constexpr auto MIN_PATH_X_FREQ    = 0.9F;
  static constexpr auto MAX_PATH_X_FREQ    = 2.0F;
  static constexpr auto MIN_PATH_Y_FREQ    = 0.9F;
  static constexpr auto MAX_PATH_Y_FREQ    = 2.0F;

  return {
      m_goomRand.GetRandInRange(MIN_PATH_AMPLITUDE, MAX_PATH_AMPLITUDE),
      m_goomRand.GetRandInRange(MIN_PATH_X_FREQ, MAX_PATH_X_FREQ),
      m_goomRand.GetRandInRange(MIN_PATH_Y_FREQ, MAX_PATH_Y_FREQ),
  };
}

inline auto LSystemFx::LSystemFxImpl::IncrementPositionT() noexcept -> void
{
  if (m_direction == Direction::TO_TARGET)
  {
    m_pathsToAndFrom->pathToTarget.IncrementT();
    if (m_pathsToAndFrom->pathToTarget.IsStopped())
    {
      m_direction = Direction::FROM_TARGET;
      if (not m_newZoomMidpoint)
      {
        m_pathsToAndFrom->pathFromTarget.Reset(0.0F);
      }
      else
      {
        m_pathsToAndFrom  = GetPathsToAndFrom();
        m_newZoomMidpoint = false;
      }
    }
  }
  else
  {
    m_pathsToAndFrom->pathFromTarget.IncrementT();
    if (m_pathsToAndFrom->pathFromTarget.IsStopped())
    {
      m_direction = Direction::TO_TARGET;
      if (not m_newZoomMidpoint)
      {
        m_pathsToAndFrom->pathToTarget.Reset(0.0F);
      }
      else
      {
        m_pathsToAndFrom  = GetPathsToAndFrom();
        m_newZoomMidpoint = false;
      }
    }
  }
}

inline auto LSystemFx::LSystemFxImpl::GetNextPathPosition() const noexcept -> Point2dInt
{
  if (m_direction == Direction::TO_TARGET)
  {
    return m_pathsToAndFrom->pathToTarget.GetNextPoint();
  }

  return m_pathsToAndFrom->pathFromTarget.GetNextPoint();
}

auto LSystemFx::LSystemFxImpl::Start() -> void
{
  SetRandFunc([this]() { return m_goomRand.GetRandInRange(0.0, 1.0); });

  ChangeColors();
  SetupNextLSysModel();

  m_direction = Direction::TO_TARGET;
  m_pathsToAndFrom->pathToTarget.Reset(0.0F);
}

inline auto LSystemFx::LSystemFxImpl::GetImageBitmap(const uint32_t size) const
    -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(m_currentBitmapName,
                                       std::clamp(size, MIN_DOT_SIZE, MAX_DOT_SIZE));
}

inline auto LSystemFx::LSystemFxImpl::ChangeColors() noexcept -> void
{
  std::for_each(begin(m_currentMainColorMaps),
                end(m_currentMainColorMaps),
                [this](auto& colorMap) { colorMap = &m_mainColorMaps->GetRandomColorMap(); });
  std::for_each(begin(m_currentLowColorMaps),
                end(m_currentLowColorMaps),
                [this](auto& colorMap) { colorMap = &m_lowColorMaps->GetRandomColorMap(); });

  const auto numColorSteps =
      m_goomRand.GetRandInRange(MIN_NUM_COLOR_STEPS, MAX_NUM_COLOR_STEPS + 1U);
  std::for_each(begin(m_currentColorTs),
                end(m_currentColorTs),
                [&numColorSteps](auto& colorT) { colorT.SetNumSteps(numColorSteps); });

  std::for_each(
      begin(m_currentColorTs), end(m_currentColorTs), [](auto& colorT) { colorT.Reset(0.0F); });
}

inline auto LSystemFx::LSystemFxImpl::GetCurrentColorMapsNames() const noexcept
    -> std::vector<std::string>
{
  return {};
}

auto LSystemFx::LSystemFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  Expects(weightedColorMaps.mainColorMaps != nullptr);
  m_mainColorMaps = weightedColorMaps.mainColorMaps;

  Expects(weightedColorMaps.lowColorMaps != nullptr);
  m_lowColorMaps = weightedColorMaps.lowColorMaps;
}

inline auto LSystemFx::LSystemFxImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept
    -> void
{
  const auto newCentreTarget = m_screenMidpoint - Vec2dInt{zoomMidpoint};
  if (newCentreTarget == m_centreTarget)
  {
    return;
  }

  m_newZoomMidpoint = true;
  m_centreTarget    = newCentreTarget;
}

inline auto LSystemFx::LSystemFxImpl::ApplyMultiple() -> void
{
  Update();

  DrawLSystem();
}

auto LSystemFx::LSystemFxImpl::Update() noexcept -> void
{
  if (static constexpr auto PROB_CHANGE_COLORS = 0.33F;
      (0 == m_goomInfo.GetSoundEvents().GetTimeSinceLastGoom()) or
      m_goomRand.ProbabilityOf(PROB_CHANGE_COLORS))
  {
    ChangeColors();
  }

  ++m_numModulesPerUpdate;

  m_translateAdjust = Vec2dFlt{GetNextPathPosition().ToFlt()};

  IncrementTs();
}

inline auto LSystemFx::LSystemFxImpl::IncrementTs() noexcept -> void
{
  m_rotateDegreesAdjust.Increment();
  m_yScaleAdjust.Increment();
  m_verticalMove.Increment();
  IncrementPositionT();
}


inline auto LSystemFx::LSystemFxImpl::SetupNextLSysModel() -> void
{
  SetupLSysModel(GetNextLSystemFile());
  SetNewLSysInterpreter();
}

inline auto LSystemFx::LSystemFxImpl::GetLSystemDirectory() const noexcept -> std::string
{
  return m_resourcesDirectory + "/l-systems";
}

inline auto LSystemFx::LSystemFxImpl::GetNextLSystemFile() const noexcept -> LSystemFile
{
  static constexpr std::array FILENAMES{
      //"bush_b",
      //"hogeweg_plant_a",
      "honda_tree_b", // good
      "ternary_tree_a", // good
      // "ternary_tree_b", // too slow
  };

  const auto* const nextFilename =
      FILENAMES.at(m_goomRand.GetRandInRange(0U, static_cast<uint32_t>(FILENAMES.size())));

  const auto fileRoot = std::string{GetLSystemDirectory() + "/" + nextFilename};
  return {fileRoot + ".in", fileRoot + ".bnds"};
}

auto LSystemFx::LSystemFxImpl::SetupLSysModel(const LSystemFile& lSystemFile) -> void
{
  LogInfo("Setting up L-System.");

  const auto boundingBox3d = GetBoundingBox3d(lSystemFile.boundsFilename);
  const auto boundingBox2d = GetBoundingBox2d(boundingBox3d);
  LogInfo("boundingBox2d = ({:.2f}, {:.2f}), ({:.2f}, {:.2f})",
          boundingBox2d.min.x,
          boundingBox2d.min.y,
          boundingBox2d.max.x,
          boundingBox2d.max.y);

  m_LSystemXScale =
      static_cast<float>(m_goomInfo.GetScreenWidth()) / (boundingBox2d.max.x - boundingBox2d.min.x);
  //TODO(glk) How to handle scale issues
  m_LSystemYScale = static_cast<float>(m_goomInfo.GetScreenHeight()) /
                    (boundingBox2d.max.y - boundingBox2d.min.y);

  const auto drawLine = [this](const L_SYSTEM::Vector& point1,
                               const L_SYSTEM::Vector& point2,
                               const int color,
                               const float lineWidth)
  { DrawAndCacheLine(point1, point2, color, lineWidth); };
  const auto drawPolygon = [this](const std::vector<L_SYSTEM::Vector>& polygon,
                                  [[maybe_unused]] const int color,
                                  const float lineWidth)
  { DrawAndCachePolygon(polygon, color, lineWidth); };

  m_drawFuncs = {drawLine, drawPolygon};

  m_LSysProperties.inputFilename = lSystemFile.mainFilename;

  m_LSysModel      = GetParsedModel(m_LSysProperties);
  m_LSysProperties = GetFinalProperties(m_LSysModel->GetSymbolTable(), m_LSysProperties);
  m_moduleList     = nullptr;

  m_LSysGenerator = std::make_unique<GraphicsGenerator>(m_drawFuncs);
  m_LSysGenerator->SetName("On the Fly");

  LogInfo("Finished setting up L-System.");
}

inline auto LSystemFx::LSystemFxImpl::SetNewLSysInterpreter() noexcept -> void
{
  LogInfo("Set new LSysInterpreter.");

  m_LSysInterpreter = std::make_unique<Interpreter>(*m_LSysGenerator);
  m_moduleList      = m_LSysModel->GetStartModuleList()->Clone();
//  std::cerr << "Start mod = " << m_moduleList->GetFirst() << "\n";

  for (auto gen = 1; gen < m_LSysProperties.maxGen; ++gen)
  {
    m_moduleList = m_LSysModel->Generate(m_moduleList.get());
  }
  Ensures(m_moduleList != nullptr);

  m_moduleList = m_LSysModel->Generate(m_moduleList.get());
  Ensures(m_moduleList != nullptr);

  m_numModulesPerUpdate = static_cast<uint32_t>(m_moduleList->size());

  RestartLSysInterpreter();
}

inline auto LSystemFx::LSystemFxImpl::RestartLSysInterpreter() noexcept -> void
{
  if (not ResetDefaultInterpreterParams())
  {
    m_cachedPolygonsReady = not m_cachedPolygons.empty();
    m_cachedPolygons.shrink_to_fit();
    return;
  }

  LogInfo("Reset default params. Emptying cached data.");
  m_cachedPolygonsReady = false;
  m_cachedPolygons.clear();
  m_cachedPolygons.reserve(m_moduleList->size());
  m_LSysInterpreter->Start(*m_moduleList);
}

inline auto LSystemFx::LSystemFxImpl::ResetDefaultInterpreterParams() noexcept -> bool
{
  if (static constexpr auto PROB_CHANGE_ROTATE_DIRECTION = 0.001F;
      m_goomRand.ProbabilityOf(PROB_CHANGE_ROTATE_DIRECTION))
  {
    m_rotateSign = -m_rotateSign;
  }

  static constexpr auto PROB_NEW_DEFAULT_PARAMS = 0.5F;
  const bool resetDefaultParams                 = m_goomRand.ProbabilityOf(PROB_NEW_DEFAULT_PARAMS);

  if (resetDefaultParams)
  {
    m_defaultInterpreterParams = {
        m_goomRand.GetRandInRange(0.0F, DEGREES_360),
        5.0F * m_goomRand.GetRandInRange(1.0F, 5.0F),
        m_goomRand.GetRandInRange(1.0F, 50.0F),
    };
  }

  m_LSysInterpreter->SetDefaults(m_defaultInterpreterParams);

  return resetDefaultParams;
}

inline auto LSystemFx::LSystemFxImpl::DrawLSystem() noexcept -> void
{
  LogInfo("Start draw L-System.");

  if (m_cachedPolygonsReady)
  {
    DrawCachedLSystem();
  }
  else
  {
    DrawInterpretedLSystem();
  }
}

inline auto LSystemFx::LSystemFxImpl::DrawCachedLSystem() noexcept -> void
{
  LogInfo("Start L-System cached draw. Num polygons = {}.", m_cachedPolygons.size());

  for (const auto& polygonInfo : m_cachedPolygons)
  {
    DrawPolygon(polygonInfo.polygon, polygonInfo.color, polygonInfo.lineWidth);
  }

  RestartLSysInterpreter();
}

inline auto LSystemFx::LSystemFxImpl::DrawInterpretedLSystem() noexcept -> void
{
  LogInfo("Start L-System interpreted draw. Num modules = {}.", m_moduleList->size());

  if (DrawLSystemBatch())
  {
    return;
  }

  RestartLSysInterpreter();
}

inline auto LSystemFx::LSystemFxImpl::DrawLSystemBatch() noexcept -> bool
{
  for (auto i = 0U; i < m_numModulesPerUpdate; ++i)
  {
    if (not m_LSysInterpreter->InterpretNext())
    {
      LogInfo("Finished interpreting all modules.");
      return false;
    }
  }

  return true;
}

inline auto LSystemFx::LSystemFxImpl::DrawAndCacheLine(const L_SYSTEM::Vector& point1,
                                                       const L_SYSTEM::Vector& point2,
                                                       const int color,
                                                       const float lineWidth) noexcept -> void
{
  m_cachedPolygons.emplace_back(PolygonInfo{
      std::vector<L_SYSTEM::Vector>{point1, point2},
      color, lineWidth
  });

  DrawLine(point1, point2, color, lineWidth);
}

inline auto LSystemFx::LSystemFxImpl::DrawLine(const L_SYSTEM::Vector& point1,
                                               const L_SYSTEM::Vector& point2,
                                               [[maybe_unused]] const int color,
                                               const float lineWidth) noexcept -> void
{
  const auto iLineWidth = static_cast<uint8_t>(std::clamp(0.5F * lineWidth, 1.0F, 5.0F));

  DrawJoinedVertices({GetPoint3dFlt(point1), GetPoint3dFlt(point2)}, iLineWidth);
}

inline auto LSystemFx::LSystemFxImpl::DrawAndCachePolygon(
    const std::vector<L_SYSTEM::Vector>& polygon, const int color, const float lineWidth) noexcept
    -> void
{
  m_cachedPolygons.emplace_back(PolygonInfo{polygon, color, lineWidth});

  DrawPolygon(polygon, color, lineWidth);
}

inline auto LSystemFx::LSystemFxImpl::DrawPolygon(const std::vector<L_SYSTEM::Vector>& polygon,
                                                  [[maybe_unused]] const int color,
                                                  const float lineWidth) noexcept -> void
{
  const auto iLineWidth = static_cast<uint8_t>(std::clamp(2.0F * lineWidth, 1.0F, 5.0F));

  DrawJoinedVertices(GetPolygon3dFlt(polygon), iLineWidth);
}

auto LSystemFx::LSystemFxImpl::GetTransformedPoint(
    const Point2dFlt& point, const TransformAdjust& transformAdjust) const noexcept -> Point2dFlt
{
  auto transformedPoint = point;

  // TODO(glk) Consolidate operations
  transformedPoint.Scale(m_LSystemXScale * transformAdjust.xScale,
                         (m_LSystemYScale * transformAdjust.yScale) * m_yScaleAdjust());
  transformedPoint.Translate(Vec2dFlt{0.0F, m_verticalMove()});
  transformedPoint.Rotate(m_rotateSign *
                          ToRadians(transformAdjust.rotateDegrees + m_rotateDegreesAdjust()));
  transformedPoint.Translate(m_lSystemStartPoint + transformAdjust.translate + m_translateAdjust);

  return transformedPoint;
}

auto LSystemFx::LSystemFxImpl::DrawJoinedVertices(const std::vector<Point3dFlt>& vertices,
                                                  const uint8_t lineWidth) noexcept -> void
{
  const auto numVertices = vertices.size();
  Expects(numVertices > 1);

  const auto points2d = GetPerspectiveProjection(vertices);

  //LogInfo("numVertices = {}", numVertices);
  auto point2d0 = points2d.at(0);
  for (auto i = 1U; i < numVertices; ++i)
  {
    const auto point0 = point2d0;
    const auto point1 = points2d.at(i);

    //LogInfo("point0 = ({}, {}), point1 = ({}, {})", point0.x, point0.y, point1.x, point1.y);

    for (auto copyNum = 0U; copyNum < NUM_LSYS_COPIES; ++copyNum)
    {
      auto tPoint0 = GetTransformedPoint(point0, m_transformAdjustArray.at(copyNum));
      auto tPoint1 = GetTransformedPoint(point1, m_transformAdjustArray.at(copyNum));

      //LogInfo("Post point0 = ({}, {}), point1 = ({}, {})", point0.x, point0.y, point1.x, point1.y);

      // TODO(Glk) put in transform
      tPoint0.x = static_cast<float>(m_goomInfo.GetScreenWidth() - 1) - tPoint0.x;
      tPoint1.x = static_cast<float>(m_goomInfo.GetScreenWidth() - 1) - tPoint1.x;
      tPoint0.y = static_cast<float>(m_goomInfo.GetScreenHeight() - 1) - tPoint0.y;
      tPoint1.y = static_cast<float>(m_goomInfo.GetScreenHeight() - 1) - tPoint1.y;

      DrawChunkedLine(tPoint0, tPoint1, copyNum, lineWidth);

      //LogInfo("m_currentColorTs.at({}) = {}", copyNum, m_currentColorTs.at(copyNum)());
    }

    point2d0 = points2d.at(i);
  }
}

inline auto LSystemFx::LSystemFxImpl::DrawChunkedLine(const Point2dFlt& point1,
                                                      const Point2dFlt& point2,
                                                      const uint32_t copyNum,
                                                      const uint8_t lineWidth) noexcept -> void
{
  if constexpr (1U == NUM_LINE_CHUNKS)
  {
    m_draw.Line(point1.ToInt(), point2.ToInt(), GetColors(copyNum), lineWidth);
    m_currentColorTs.at(copyNum).Increment();
    return;
  }

  auto pointOnLine =
      IncrementedValue{point1, point2, TValue::StepType::SINGLE_CYCLE, NUM_LINE_CHUNKS};

  for (auto i = 0U; i < NUM_LINE_CHUNKS; ++i)
  {
    const auto tempPoint1 = pointOnLine();
    pointOnLine.Increment();
    const auto tempPoint2 = pointOnLine();

    m_draw.Line(tempPoint1.ToInt(), tempPoint2.ToInt(), GetColors(copyNum), lineWidth);

    m_currentColorTs.at(copyNum).Increment();
  }
}

inline auto LSystemFx::LSystemFxImpl::GetColors(const uint32_t copyNum) const noexcept
    -> MultiplePixels
{
  static constexpr auto MAIN_BRIGHTNESS = 1.0F;
  static constexpr auto LOW_BRIGHTNESS  = 1.5F * MAIN_BRIGHTNESS;

  const auto colorT = m_currentColorTs.at(copyNum)();

  const auto mainColor = m_colorAdjust.GetAdjustment(
      MAIN_BRIGHTNESS, m_currentMainColorMaps.at(copyNum)->GetColor(colorT));
  const auto lowColor = m_colorAdjust.GetAdjustment(
      LOW_BRIGHTNESS, m_currentLowColorMaps.at(copyNum)->GetColor(colorT));

  return MakePixels(mainColor, lowColor);
}

auto LSystemFx::LSystemFxImpl::GetPerspectiveProjection(
    const std::vector<Point3dFlt>& points3d) noexcept -> std::vector<Point2dFlt>
{
  const auto numPoints = points3d.size();
  auto points2d        = std::vector<Point2dFlt>(numPoints);

  for (auto i = 0U; i < numPoints; ++i)
  {
    points2d[i] = GetPerspectivePoint(points3d[i]);
  }

  return points2d;
}

inline auto LSystemFx::LSystemFxImpl::GetPerspectivePoint(const Point3dFlt& point3d) noexcept
    -> Point2dFlt
{
  return {point3d.x, point3d.y};

  const auto point = Point3dFlt{point3d.x, point3d.y, point3d.z};

  const auto zOffset = point.z + 10.0F;
  if (std::fabs(zOffset) < UTILS::MATH::SMALL_FLOAT)
  {
    return {0.0F, 0.0F};
  }
  static constexpr auto PROJECTION_DISTANCE = 10.0F;
  const auto perspectiveFactor              = PROJECTION_DISTANCE / zOffset;
  const auto xProj                          = perspectiveFactor * point.x;
  const auto yProj                          = perspectiveFactor * point.y;

  return Point2dFlt{xProj, yProj};
}

inline auto LSystemFx::LSystemFxImpl::GetPolygon3dFlt(
    const std::vector<L_SYSTEM::Vector>& polygon) noexcept -> std::vector<Point3dFlt>
{
  const auto numVertices = polygon.size();
  auto polygon3dFlt      = std::vector<Point3dFlt>(numVertices);

  for (auto i = 0U; i < numVertices; ++i)
  {
    polygon3dFlt[i] = GetPoint3dFlt(polygon[i]);
  }

  return polygon3dFlt;
}

inline auto LSystemFx::LSystemFxImpl::GetPoint3dFlt(const L_SYSTEM::Vector& point) noexcept
    -> Point3dFlt
{
  return {point(0), point(1), point(2)};
}

inline auto LSystemFx::LSystemFxImpl::GetBoundingBox2d(const BoundingBox3d& boundingBox3d) noexcept
    -> BoundingBox2d
{
  static constexpr auto EXPAND_FACTOR = 3.1F;
  const auto boundingBox2d = BoundingBox2d{EXPAND_FACTOR * GetPerspectivePoint(boundingBox3d.min),
                                           EXPAND_FACTOR * GetPerspectivePoint(boundingBox3d.max)};
  return boundingBox2d;

  const auto minVal = std::min(boundingBox2d.min.x, boundingBox3d.min.y);
  const auto maxVal = std::max(boundingBox2d.max.x, boundingBox3d.max.y);

  return {
      {minVal, minVal},
      {maxVal, maxVal}
  };
}

} // namespace GOOM::VISUAL_FX
