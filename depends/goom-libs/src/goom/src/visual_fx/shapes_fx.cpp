#include "shapes_fx.h"

//#undef NO_LOGGING

#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "goom_plugin_info.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/paths.h"
#include "utils/math/transform2d.h"
#include "utils/t_values.h"

#include <array>
#include <memory>

namespace GOOM::VISUAL_FX
{

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

  void ApplyMultiple();

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  const Point2dInt m_screenMidpoint;

  static constexpr uint32_t NUM_SHAPES = 5;
  struct ShapePath
  {
    std::shared_ptr<TValue> positionT;
    std::shared_ptr<UTILS::MATH::IPath> path;
  };
  std::array<ShapePath, NUM_SHAPES> m_shapePaths;
  [[nodiscard]] static auto GetShapePaths(const Point2dInt& screenMidpoint)
      -> std::array<ShapePath, NUM_SHAPES>;
  std::array<std::shared_ptr<RandomColorMaps>, NUM_SHAPES> m_colorMaps{};
  std::array<RandomColorMapsManager, NUM_SHAPES> m_colorMapsManagers{};
  std::array<uint32_t, NUM_SHAPES> m_colorMapIds{};
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
    m_shapePaths{GetShapePaths(m_screenMidpoint)}
{
}


auto ShapesFx::ShapesFxImpl::GetShapePaths(const Point2dInt& screenMidpoint)
    -> std::array<ShapePath, NUM_SHAPES>
{
  std::array<ShapePath, NUM_SHAPES> shapePaths{};

  const auto basePositionT =
      std::make_shared<TValue>(TValue::StepType::CONTINUOUS_REVERSIBLE, 0.001F);
  auto baseHypotrochoid = std::make_shared<Hypotrochoid>(
      Point2dInt{0, 0}, *basePositionT, Hypotrochoid::Params{7.0F, 3.0F, 5.0F, 40.0F});

  const Vec2dFlt screenMidpointFlt{screenMidpoint.ToFlt()};

  Transform2d transform{};

  transform = Transform2d{0.0F, 1.0F, screenMidpointFlt};
  shapePaths[0].positionT = basePositionT;
  shapePaths[0].path = std::make_shared<TransformedPath>(baseHypotrochoid, transform);

  transform = Transform2d{45.0F, 1.1F, screenMidpointFlt};
  shapePaths[1].positionT = basePositionT;
  shapePaths[1].path = std::make_shared<TransformedPath>(baseHypotrochoid, transform);

  transform = Transform2d{90.0F, 1.2F, screenMidpointFlt};
  shapePaths[2].positionT = basePositionT;
  shapePaths[2].path = std::make_shared<TransformedPath>(baseHypotrochoid, transform);

  transform = Transform2d{135.0F, 1.1F, screenMidpointFlt};
  shapePaths[3].positionT = basePositionT;
  shapePaths[3].path = std::make_shared<TransformedPath>(baseHypotrochoid, transform);

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
}

inline void ShapesFx::ShapesFxImpl::SetWeightedColorMaps(
    const uint32_t shapeNum, const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_colorMaps.at(shapeNum) = weightedMaps;
  m_colorMapIds.at(shapeNum) = m_colorMapsManagers.at(shapeNum).AddColorMapInfo(
      {m_colorMaps.at(shapeNum),
       m_colorMaps.at(shapeNum)->GetRandomColorMapName(m_colorMaps.at(shapeNum)->GetRandomGroup()),
       RandomColorMaps::ALL_COLOR_MAP_TYPES});
}

inline void ShapesFx::ShapesFxImpl::ApplyMultiple()
{
  for (size_t i = 0; i < NUM_SHAPES; ++i)
  {
    const Point2dInt point = m_shapePaths.at(i).path->GetNextPoint();
    const Pixel color = m_colorMapsManagers.at(i)
                            .GetColorMap(m_colorMapIds.at(i))
                            .GetColor((*m_shapePaths.at(i).positionT)());

    m_draw.Circle(point, 20,
                  {COLOR::GetBrighterColor(20.0F, color), COLOR::GetBrighterColor(20.0F, color)});

    m_shapePaths.at(i).positionT->Increment();
  }
}

} // namespace GOOM::VISUAL_FX
