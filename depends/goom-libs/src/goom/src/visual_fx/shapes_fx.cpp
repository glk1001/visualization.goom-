#include "shapes_fx.h"

//#undef NO_LOGGING

#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "goom_plugin_info.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/paths.h"
#include "utils/math/transform2d.h"
#include "utils/t_values.h"

#include <array>
#include <memory>

namespace GOOM::VISUAL_FX
{

using COLOR::GetAllMapsUnweighted;
using COLOR::GetBrighterColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using COLOR::RandomColorMapsManager;
using DRAW::IGoomDraw;
using UTILS::Logging;
using UTILS::TValue;
using UTILS::MATH::AngleParams;
using UTILS::MATH::CirclePath;
using UTILS::MATH::Epicycloid;
using UTILS::MATH::Hypotrochoid;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::IPath;
using UTILS::MATH::LissajousPath;
using UTILS::MATH::Transform2d;
using UTILS::MATH::TransformedPath;
using UTILS::MATH::U_HALF;

class ShapesFx::ShapesFxImpl
{
public:
  explicit ShapesFxImpl(const FxHelper& fxHelper) noexcept;

  void Start();

  void SetWeightedColorMaps(uint32_t shapeNum, std::shared_ptr<RandomColorMaps> weightedMaps);
  void SetWeightedLowColorMaps(uint32_t shapeNum, std::shared_ptr<RandomColorMaps> weightedMaps);

  void ApplyMultiple();

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  const Point2dInt m_screenMidpoint;

  struct ShapePath
  {
    std::shared_ptr<TValue> positionT;
    std::shared_ptr<UTILS::MATH::IPath> path;
  };
  std::array<ShapePath, NUM_SHAPES> m_shapePaths;
  [[nodiscard]] auto GetRandomizedShapePaths(const Point2dInt& screenMidpoint) const
      -> std::array<ShapePath, NUM_SHAPES>;
  [[nodiscard]] static auto GetShapePaths(std::shared_ptr<TValue> basePositionT,
                                          std::shared_ptr<IPath> basePath,
                                          const Point2dInt& screenMidpoint)
      -> std::array<ShapePath, NUM_SHAPES>;
  static constexpr float MIN_SPEED = 0.0001F;
  static constexpr float MAX_SPEED = 0.0005F;
  static constexpr float INITIAL_SPEED = 0.0005F;
  static_assert((MIN_SPEED <= INITIAL_SPEED) && (INITIAL_SPEED <= MAX_SPEED));
  auto SetShapesSpeed(float t) -> void;

  std::array<std::shared_ptr<RandomColorMaps>, NUM_SHAPES> m_colorMaps{};
  std::array<std::shared_ptr<RandomColorMaps>, NUM_SHAPES> m_lowColorMaps{};
  std::array<std::shared_ptr<RandomColorMaps>, NUM_SHAPES> m_innerColorMaps{};
  std::array<RandomColorMapsManager, NUM_SHAPES> m_colorMapsManagers{};
  std::array<RandomColorMapsManager::ColorMapId, NUM_SHAPES> m_colorMapIds{};
  std::array<RandomColorMapsManager::ColorMapId, NUM_SHAPES> m_lowColorMapIds{};
  std::array<RandomColorMapsManager::ColorMapId, NUM_SHAPES> m_innerColorMapIds{};
  std::array<float, NUM_SHAPES> m_innerColorMixes{};
  auto UpdateColorMaps() -> void;
  auto UpdateInnerColorMaps() -> void;
  auto DrawShapes() -> void;
  auto DrawShape(size_t shapeNum) -> void;
};

ShapesFx::ShapesFx(const FxHelper& fxHelper) noexcept
  : m_fxImpl{spimpl::make_unique_impl<ShapesFxImpl>(fxHelper)}
{
}

void ShapesFx::Start()
{
  m_fxImpl->Start();
}

void ShapesFx::Finish()
{
  // nothing to do
}

auto ShapesFx::GetFxName() const -> std::string
{
  return "shapes";
}

void ShapesFx::SetWeightedColorMaps(const uint32_t shapeNum,
                                    const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedColorMaps(shapeNum, weightedMaps);
}

void ShapesFx::SetWeightedLowColorMaps(const uint32_t shapeNum,
                                       const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedLowColorMaps(shapeNum, weightedMaps);
}

void ShapesFx::ApplyMultiple()
{
  m_fxImpl->ApplyMultiple();
}

ShapesFx::ShapesFxImpl::ShapesFxImpl(const FxHelper& fxHelper) noexcept
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_screenMidpoint{U_HALF * m_goomInfo.GetScreenInfo().width,
                     U_HALF * m_goomInfo.GetScreenInfo().height},
    m_shapePaths{GetRandomizedShapePaths(m_screenMidpoint)}
{
}

inline auto ShapesFx::ShapesFxImpl::GetRandomizedShapePaths(const Point2dInt& screenMidpoint) const
    -> std::array<ShapePath, NUM_SHAPES>
{
  const Hypotrochoid::Params params = {
      m_goomRand.GetRandInRange(7.0F, 10.F),
      m_goomRand.GetRandInRange(3.0F, 5.0F),
      m_goomRand.GetRandInRange(5.0F, 7.F),
      m_goomRand.GetRandInRange(35.0F, 45.F),
  };
  const auto basePositionT =
      std::make_shared<TValue>(TValue::StepType::CONTINUOUS_REVERSIBLE, INITIAL_SPEED);
  const auto baseHypotrochoid =
      std::make_shared<Hypotrochoid>(Point2dInt{0, 0}, *basePositionT, params);

  return GetShapePaths(basePositionT, baseHypotrochoid, screenMidpoint);
}

auto ShapesFx::ShapesFxImpl::GetShapePaths(const std::shared_ptr<TValue> basePositionT,
                                           const std::shared_ptr<IPath> basePath,
                                           const Point2dInt& screenMidpoint)
    -> std::array<ShapePath, NUM_SHAPES>
{
  const Vec2dFlt screenMidpointFlt{screenMidpoint.ToFlt()};

  std::array<ShapePath, NUM_SHAPES> shapePaths{};
  Transform2d transform{};

  static constexpr float ROTATE0 = 0.0F;
  static constexpr float SCALE0 = 1.5F;
  transform = Transform2d{ROTATE0, SCALE0, screenMidpointFlt};
  shapePaths[0].positionT = basePositionT;
  shapePaths[0].path = std::make_shared<TransformedPath>(basePath, transform);

  static constexpr float ROTATE1 = 45.0F;
  static constexpr float SCALE1 = 1.6F;
  transform = Transform2d{ROTATE1, SCALE1, screenMidpointFlt};
  shapePaths[1].positionT = basePositionT;
  shapePaths[1].path = std::make_shared<TransformedPath>(basePath, transform);

  static constexpr float ROTATE2 = 90.0F;
  static constexpr float SCALE2 = 1.7F;
  transform = Transform2d{ROTATE2, SCALE2, screenMidpointFlt};
  shapePaths[2].positionT = basePositionT;
  shapePaths[2].path = std::make_shared<TransformedPath>(basePath, transform);

  static constexpr float ROTATE3 = 135.0F;
  static constexpr float SCALE3 = 1.6F;
  transform = Transform2d{ROTATE3, SCALE3, screenMidpointFlt};
  shapePaths[3].positionT = basePositionT;
  shapePaths[3].path = std::make_shared<TransformedPath>(basePath, transform);

  const AngleParams circleAngleParams{0.0F, 360.0F};
  shapePaths[4].positionT = basePositionT;
  shapePaths[4].path =
      std::make_shared<CirclePath>(screenMidpoint, *basePositionT, 300.0F, circleAngleParams);

  return shapePaths;
}

inline void ShapesFx::ShapesFxImpl::Start()
{
  for (auto& shapePath : m_shapePaths)
  {
    shapePath.positionT->Reset();
  }

  UpdateInnerColorMaps();
}

inline void ShapesFx::ShapesFxImpl::SetWeightedColorMaps(
    const uint32_t shapeNum, const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_colorMaps.at(shapeNum) = weightedMaps;
  m_colorMapsManagers.at(shapeNum).RemoveColorMapInfo(m_colorMapIds.at(shapeNum));
  m_colorMapIds.at(shapeNum) = m_colorMapsManagers.at(shapeNum).AddColorMapInfo(
      {m_colorMaps.at(shapeNum),
       m_colorMaps.at(shapeNum)->GetRandomColorMapName(m_colorMaps.at(shapeNum)->GetRandomGroup()),
       RandomColorMaps::ALL_COLOR_MAP_TYPES});
}

inline void ShapesFx::ShapesFxImpl::SetWeightedLowColorMaps(
    const uint32_t shapeNum, const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_lowColorMaps.at(shapeNum) = weightedMaps;
  m_colorMapsManagers.at(shapeNum).RemoveColorMapInfo(m_lowColorMapIds.at(shapeNum));
  m_lowColorMapIds.at(shapeNum) = m_colorMapsManagers.at(shapeNum).AddColorMapInfo(
      {m_lowColorMaps.at(shapeNum),
       m_lowColorMaps.at(shapeNum)->GetRandomColorMapName(
           m_lowColorMaps.at(shapeNum)->GetRandomGroup()),
       RandomColorMaps::ALL_COLOR_MAP_TYPES});
}

inline auto ShapesFx::ShapesFxImpl::UpdateColorMaps() -> void
{
  UpdateInnerColorMaps();

  for (auto& colorMapsManager : m_colorMapsManagers)
  {
    colorMapsManager.ChangeAllColorMapsNow();
  }
}

inline auto ShapesFx::ShapesFxImpl::UpdateInnerColorMaps() -> void
{
  for (size_t shapeNum = 0; shapeNum < NUM_SHAPES; ++shapeNum)
  {
    m_innerColorMixes.at(shapeNum) = m_goomRand.GetRandInRange(0.1F, 0.6F);
    m_innerColorMaps.at(shapeNum) = GetAllMapsUnweighted(m_goomRand);
    m_colorMapsManagers.at(shapeNum).RemoveColorMapInfo(m_innerColorMapIds.at(shapeNum));
    m_innerColorMapIds.at(shapeNum) = m_colorMapsManagers.at(shapeNum).AddColorMapInfo(
        {m_innerColorMaps.at(shapeNum),
         m_innerColorMaps.at(shapeNum)->GetRandomColorMapName(
             m_innerColorMaps.at(shapeNum)->GetRandomGroup()),
         RandomColorMaps::ALL_COLOR_MAP_TYPES});
  }
}

inline void ShapesFx::ShapesFxImpl::ApplyMultiple()
{
  DrawShapes();
}

inline auto ShapesFx::ShapesFxImpl::SetShapesSpeed(const float t) -> void
{
  for (auto& shapePath : m_shapePaths)
  {
    shapePath.positionT->SetStepSize(STD20::lerp(MIN_SPEED, MAX_SPEED, t));
  }
}

inline auto ShapesFx::ShapesFxImpl::DrawShapes() -> void
{
  for (size_t i = 0; i < NUM_SHAPES; ++i)
  {
    DrawShape(i);
    m_shapePaths.at(i).positionT->Increment();
  }

  if (m_shapePaths.at(0).positionT->HasJustHitStartBoundary())
  {
    m_shapePaths = GetRandomizedShapePaths(m_screenMidpoint);
    SetShapesSpeed(m_goomRand.GetRandInRange(0.0F, 1.0F));
    UpdateColorMaps();
  }
}

auto ShapesFx::ShapesFxImpl::DrawShape(const size_t shapeNum) -> void
{
  const Point2dInt point = m_shapePaths.at(shapeNum).path->GetNextPoint();

  const Pixel shapeColor = m_colorMapsManagers.at(shapeNum)
                               .GetColorMap(m_colorMapIds.at(shapeNum))
                               .GetColor((*m_shapePaths.at(shapeNum).positionT)());
  const Pixel shapeLowColor = m_colorMapsManagers.at(shapeNum)
                                  .GetColorMap(m_lowColorMapIds.at(shapeNum))
                                  .GetColor((*m_shapePaths.at(shapeNum).positionT)());

  static constexpr int32_t MAX_RADIUS = 20;

  TValue innerColorT{UTILS::TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(MAX_RADIUS - 1)};

  static constexpr float BRIGHTNESS0 = 10.0F;
  float brightness = BRIGHTNESS0;

  for (int32_t radius = MAX_RADIUS; radius > 1; --radius)
  {
    const Pixel innerColor = m_colorMapsManagers.at(shapeNum)
                                 .GetColorMap(m_innerColorMapIds.at(shapeNum))
                                 .GetColor(innerColorT());
    const Pixel color = GetBrighterColor(
        brightness, IColorMap::GetColorMix(shapeColor, innerColor, m_innerColorMixes.at(shapeNum)));
    const Pixel lowColor =
        GetBrighterColor(0.5F * brightness, IColorMap::GetColorMix(shapeLowColor, innerColor,
                                                                   m_innerColorMixes.at(shapeNum)));

    m_draw.Circle(point, radius, {color, lowColor});

    brightness += 1.0F;
    innerColorT.Increment();
  }
}

} // namespace GOOM::VISUAL_FX
