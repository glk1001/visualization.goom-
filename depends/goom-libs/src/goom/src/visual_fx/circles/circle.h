#pragma once

#include "bitmap_getter_base.h"
#include "color/color_adjustment.h"
#include "color/color_maps.h"
#include "color/color_maps_grids.h"
#include "color/random_color_maps.h"
#include "dot_diameters.h"
#include "dot_paths.h"
#include "draw/goom_draw.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/paths.h"
#include "utils/propagate_const.h"
#include "utils/t_values.h"
#include "utils/timer.h"
#include "visual_fx/fx_helper.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

class Circle
{
public:
  struct Helper
  {
    uint32_t lineDotDiameter;
    uint32_t minDotDiameter;
    uint32_t maxDotDiameter;
    const IBitmapGetter& bitmapGetter;
  };
  struct Params
  {
    float circleRadius;
    Point2dInt circleCentreTarget;
  };

  Circle() = delete;
  Circle(const FxHelper& fxHelper,
         const Helper& helper,
         const Params& circleParams,
         const UTILS::MATH::OscillatingFunction::Params& pathParams) noexcept;
  Circle(const Circle&) = delete;
  Circle(Circle&& other) noexcept;
  ~Circle() noexcept;
  auto operator=(const Circle&) -> Circle& = delete;
  auto operator=(Circle&&) -> Circle& = delete;

  void SetWeightedColorMaps(std::shared_ptr<const COLOR::RandomColorMaps> weightedMainMaps,
                            std::shared_ptr<const COLOR::RandomColorMaps> weightedLowMaps);
  void SetMovingTargetPoint(const Point2dInt& movingTargetPoint, float lerpTFromFixedTarget);
  void SetPathParams(const UTILS::MATH::OscillatingFunction::Params& pathParams);

  void Start();
  void UpdatePositionSpeed(uint32_t newNumSteps);
  void UpdateAndDraw();

  [[nodiscard]] auto HasPositionTJustHitABoundary() const noexcept -> bool;
  [[nodiscard]] auto HasPositionTJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto GetCircleCentreFixedTarget() const noexcept -> Point2dInt;
  [[nodiscard]] auto GetLastDrawnCircleDots() const -> const std::vector<Point2dInt>&;

private:
  DRAW::IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const UTILS::MATH::IGoomRand& m_goomRand;
  const Helper m_helper;
  const Point2dInt m_circleCentreFixedTarget;
  Point2dInt m_circleCentreTarget{m_circleCentreFixedTarget};
  DotPaths m_dotPaths;
  DotDiameters m_dotDiameters{m_goomRand, NUM_DOTS, m_helper.minDotDiameter,
                              m_helper.maxDotDiameter};
  [[nodiscard]] static auto GetDotStartingPositions(const Point2dInt& centre, float radius)
      -> std::vector<Point2dInt>;

  uint64_t m_updateNum = 0;
  uint32_t m_dotAttributeOffset = 0;
  [[nodiscard]] auto IsSpecialUpdateNum() const -> bool;
  [[nodiscard]] auto IsSpecialLineUpdateNum() const -> bool;
  static constexpr uint32_t BLANK_TIME = 20;
  UTILS::Timer m_blankTimer{BLANK_TIME, true};

  void UpdateTime();
  void ResetNumSteps();
  void ResetCircleParams();

  static constexpr uint32_t NUM_DOTS = 30;
  static_assert(UTILS::MATH::IsEven(NUM_DOTS));
  std::vector<Point2dInt> m_lastDrawnDots{NUM_DOTS};
  uint32_t m_newNumSteps = 0;

  void DrawNextCircle();
  void DrawNextCircleDots();
  void IncrementTs();

  static constexpr uint32_t MIN_NUM_COLOR_ADJUSTMENT_STEPS = 10;
  static constexpr uint32_t MAX_NUM_COLOR_ADJUSTMENT_STEPS = 500;
  [[nodiscard]] auto GetRandomNumColorAdjustmentSteps() const noexcept -> uint32_t;
  static constexpr float COLOR_ADJUSTMENT_STARTING_T = 0.5F;
  UTILS::TValue m_colorAdjustmentT{UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE,
                                   GetRandomNumColorAdjustmentSteps(), COLOR_ADJUSTMENT_STARTING_T};
  static constexpr float GAMMA = 1.0F / 2.2F;
  COLOR::ColorAdjustment m_colorAdjustment{GAMMA, COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR};
  auto UpdateColorAdjustment() noexcept -> void;

  [[nodiscard]] auto GetCurrentBrightness() const -> float;
  [[nodiscard]] auto GetDotBrightness(float brightness) const -> float;
  [[nodiscard]] auto GetLineBrightness(float brightness) const -> float;

  class DotDrawer;
  std::experimental::propagate_const<std::unique_ptr<DotDrawer>> m_dotDrawer;
  bool m_alternateMainLowDotColors = false;
  bool m_showLine = false;
  static constexpr float T_LINE_COLOR_STEP = 1.0F / static_cast<float>(NUM_DOTS);
  auto DrawLine(const Point2dInt& position1,
                const Point2dInt& position2,
                float lineBrightness,
                float tLineColor) -> void;
  [[nodiscard]] auto DrawLineDots(const Point2dInt& position1,
                                  const Point2dInt& position2,
                                  float lineBrightness,
                                  float tLineColor) -> float;
  auto DrawDot(uint32_t dotNum,
               const Point2dInt& pos,
               const Pixel& mainColor,
               const Pixel& lowColor) -> void;
  auto DrawConnectingLine(const Point2dInt& position1,
                          const Point2dInt& position2,
                          float lineBrightness,
                          float tDotColor) -> void;

  std::shared_ptr<const COLOR::RandomColorMaps> m_mainColorMaps;
  std::shared_ptr<const COLOR::RandomColorMaps> m_lowColorMaps;
  const COLOR::IColorMap* m_linesMainColorMap{};
  const COLOR::IColorMap* m_linesLowColorMap{};

  COLOR::ColorMapsGrid m_mainColorMapsGrid;
  COLOR::ColorMapsGrid m_lowColorMapsGrid;
  static constexpr float DEFAULT_COLOR_GRID_MIX_T = 0.5F;
  float m_currentColorGridMixT = DEFAULT_COLOR_GRID_MIX_T;
  [[nodiscard]] auto GetVerticalMainColorMaps() const -> std::vector<const COLOR::IColorMap*>;
  [[nodiscard]] auto GetVerticalLowColorMaps() const -> std::vector<const COLOR::IColorMap*>;
  [[nodiscard]] auto GetColorMixT(size_t colorIndex) const -> float;
  [[nodiscard]] auto GetDotMainColors(float dotBrightness) const -> std::vector<Pixel>;
  [[nodiscard]] auto GetDotLowColors(float dotBrightness) const -> std::vector<Pixel>;

  [[nodiscard]] auto GetFinalMainColor(float brightness, const Pixel& mainColor) const -> Pixel;
  [[nodiscard]] auto GetFinalLowColor(float brightness, const Pixel& lowColor) const -> Pixel;
  [[nodiscard]] auto GetCorrectedColor(float brightness, const Pixel& color) const -> Pixel;
};

inline void Circle::UpdatePositionSpeed(const uint32_t newNumSteps)
{
  m_newNumSteps = newNumSteps;
}

inline auto Circle::HasPositionTJustHitABoundary() const noexcept -> bool
{
  return m_dotPaths.HasPositionTJustHitStartBoundary() or
         m_dotPaths.HasPositionTJustHitEndBoundary();
}

inline auto Circle::HasPositionTJustHitStartBoundary() const noexcept -> bool
{
  return m_dotPaths.HasPositionTJustHitStartBoundary();
}

inline auto Circle::GetCircleCentreFixedTarget() const noexcept -> Point2dInt
{
  return m_circleCentreFixedTarget;
}

inline auto Circle::GetLastDrawnCircleDots() const -> const std::vector<Point2dInt>&
{
  return m_lastDrawnDots;
}

} // namespace GOOM::VISUAL_FX::CIRCLES
