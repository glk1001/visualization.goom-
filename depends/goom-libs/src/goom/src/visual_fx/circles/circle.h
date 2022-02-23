#pragma once

#include "bitmap_getter_base.h"
#include "color/colormaps.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"
#include "utils/timer.h"
#include "visual_fx/fx_helper.h"

#include <array>
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
    uint32_t minDotDiameter;
    uint32_t maxDotDiameter;
    const IBitmapGetter& bitmapGetter;
    const COLOR::GammaCorrection& gammaCorrect;
  };
  struct Params
  {
    float circleRadius;
    Point2dInt circleCentreTarget;
  };

  Circle(const FxHelper& fxHelper,
         const Helper& helper,
         const Params& circleParams,
         const UTILS::MATH::PathParams& pathParams);
  Circle(const Circle&) noexcept = delete;
  Circle(Circle&&) noexcept = default;
  ~Circle() noexcept = default;
  auto operator=(const Circle&) -> Circle& = delete;
  auto operator=(Circle&&) -> Circle& = delete;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps,
                            std::shared_ptr<COLOR::RandomColorMaps> weightedLowMaps);
  void SetZoomMidpoint(const Point2dInt& zoomMidpoint);
  void SetPathParams(const UTILS::MATH::PathParams& params);

  void Start();
  void UpdateAndDraw();

private:
  DRAW::IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const UTILS::MATH::IGoomRand& m_goomRand;
  const Helper m_helper;
  const Point2dInt m_circleCentreTarget;
  Point2dInt m_currentCircleCentreTarget;
  [[nodiscard]] auto GetRandomCircleCentreTargetPosition() const -> Point2dInt;

  uint64_t m_updateNum = 0;
  [[nodiscard]] auto IsSpecialUpdateNum() const -> bool;
  static constexpr uint32_t BLANK_TIME = 40;
  UTILS::Timer m_blankTimer{BLANK_TIME, true};

  void UpdateTime();
  void UpdateSpeeds();
  void UpdatePositionSpeed();
  void ResetCircleParams();

  static constexpr uint32_t NUM_DOTS = 30;
  static_assert(UTILS::MATH::IsEven(NUM_DOTS));
  std::array<uint32_t, NUM_DOTS> m_dotDiameters;
  [[nodiscard]] static auto GetInitialDotDiameters(uint32_t maxDotDiameter)
      -> std::array<uint32_t, NUM_DOTS>;
  const std::array<Point2dInt, NUM_DOTS> m_dotStartingPositions;
  [[nodiscard]] static auto GetDotStartingPositions(const UTILS::MATH::IGoomRand& goomRand,
                                                    const Point2dInt& centre,
                                                    float radius)
      -> std::array<Point2dInt, NUM_DOTS>;
  void ChangeDotDiameters();
  static constexpr float PROB_FIXED_DIAMETER = 0.2F;
  void ChangeToFixedDotDiameters();
  void ChangeToVariableDotDiameters();

  static constexpr uint32_t DEFAULT_POSITION_STEPS = 100;
  static constexpr uint32_t MIN_POSITION_STEPS = 100;
  static constexpr uint32_t MAX_POSITION_STEPS = 500;
  static constexpr uint32_t DELAY_TIME_AT_CENTRE = 20;
  static constexpr uint32_t DELAY_TIME_AT_EDGE = 10;
  UTILS::TValue m_positionT{
      UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE,
      DEFAULT_POSITION_STEPS,
      {{0.0F, DELAY_TIME_AT_EDGE}, {1.0F, DELAY_TIME_AT_CENTRE}},
      0.0F
  };
  UTILS::MATH::PathParams m_pathParams;
  std::vector<UTILS::MATH::OscillatingPath> m_dotPaths;
  [[nodiscard]] auto GetDotPaths(const UTILS::MATH::PathParams& pathParams)
      -> std::vector<UTILS::MATH::OscillatingPath>;
  [[nodiscard]] auto GetNextDotPositions() const -> std::array<Point2dInt, NUM_DOTS>;

  void DrawNextCircle();
  void DrawNextCircleDots();
  void IncrementTs();

  [[nodiscard]] auto GetCurrentBrightness() const -> float;
  [[nodiscard]] auto GetDotBrightness(float brightness) const -> float;
  [[nodiscard]] auto GetLineBrightness(float brightness) const -> float;
  [[nodiscard]] auto GetDotColor(size_t i, float dotBrightness) const -> Pixel;
  [[nodiscard]] auto GetDotLowColor(size_t i, float dotBrightness) const -> Pixel;
  void DrawDot(const Point2dInt& pos, uint32_t diameter, const Pixel& color, const Pixel& lowColor);
  void DrawLine(const Point2dInt& pos1,
                const Point2dInt& pos2,
                float lineBrightness,
                float tLineColor);

  std::shared_ptr<COLOR::RandomColorMaps> m_colorMaps;
  std::shared_ptr<COLOR::RandomColorMaps> m_lowColorMaps;
  [[nodiscard]] auto GetRandomColorMap() const -> const COLOR::IColorMap&;
  [[nodiscard]] auto GetRandomLowColorMap() const -> const COLOR::IColorMap&;

  static constexpr uint32_t NUM_COLOR_STEPS = 20;
  static constexpr float T_LINE_COLOR_STEP = 1.0F / static_cast<float>(NUM_DOTS);
  std::vector<UTILS::TValue> m_colorTs;
  [[nodiscard]] static auto GetInitialColorTs() -> std::vector<UTILS::TValue>;
  void UpdateColorLerpSpeed();

  std::array<const COLOR::IColorMap*, NUM_DOTS> m_dotColorMaps{};
  std::array<const COLOR::IColorMap*, NUM_DOTS> m_dotLowColorMaps{};
  const COLOR::IColorMap* m_linesColorMap{};
  const COLOR::IColorMap* m_linesLowColorMap{};
  [[nodiscard]] auto GetFinalColor(float brightness, const Pixel& color) const -> Pixel;
  [[nodiscard]] auto GetFinalLowColor(float brightness, const Pixel& lowColor) const -> Pixel;
  [[nodiscard]] auto GetCorrectedColor(float brightness, const Pixel& color) const -> Pixel;
  void ChangeDotColorMaps();
};

} // namespace GOOM::VISUAL_FX::CIRCLES
