#pragma once

#include "bitmap_getter_base.h"
#include "color/colormaps.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "utils/goom_rand_base.h"
#include "utils/mathutils.h"
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

  Circle(const FxHelper& fxHelper, const Helper& helper, const Params& circleParams);

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps,
                            std::shared_ptr<COLOR::RandomColorMaps> weightedLowMaps);

  void SetZoomMidpoint(const Point2dInt& zoomMidpoint);

  void Start();
  void UpdateAndDraw();

private:
  DRAW::IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const UTILS::IGoomRand& m_goomRand;
  const Helper m_helper;
  const Point2dInt m_circleCentreTarget;

  std::shared_ptr<COLOR::RandomColorMaps> m_colorMaps;
  std::shared_ptr<COLOR::RandomColorMaps> m_lowColorMaps;
  [[nodiscard]] auto GetRandomColorMap() const -> const COLOR::IColorMap&;
  [[nodiscard]] auto GetRandomLowColorMap() const -> const COLOR::IColorMap&;

  uint64_t m_updateNum = 0;
  [[nodiscard]] auto IsSpecialUpdateNum() const -> bool;
  static constexpr uint32_t BLANK_TIME = 40;
  UTILS::Timer m_blankTimer{BLANK_TIME, true};
  void UpdateTime();

  static constexpr uint32_t DEFAULT_POSITION_STEPS = 100;
  static constexpr uint32_t MIN_POSITION_STEPS = 50;
  static constexpr uint32_t MAX_POSITION_STEPS = 250;
  static constexpr uint32_t DELAY_TIME_AT_CENTRE = 20;
  static constexpr uint32_t DELAY_TIME_AT_EDGE = 10;
  UTILS::TValue m_positionT{
      UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE,
      DEFAULT_POSITION_STEPS,
      {{0.0F, DELAY_TIME_AT_EDGE}, {1.0F, DELAY_TIME_AT_CENTRE}},
      0.0F
  };
  Point2dInt m_currentCircleCentreTarget;
  [[nodiscard]] auto GetRandomCircleCentreTargetPosition() const -> Point2dInt;
  void UpdateSpeeds();
  void UpdatePositionSpeed();
  void ResetCircleParams();

  static constexpr uint32_t NUM_DOT_PATHS = 30;
  static_assert(UTILS::IsEven(NUM_DOT_PATHS));
  std::array<uint32_t, NUM_DOT_PATHS> m_dotDiameters;
  [[nodiscard]] static auto GetInitialDotDiameters(uint32_t maxDotDiameter)
      -> std::array<uint32_t, NUM_DOT_PATHS>;
  const std::array<Point2dInt, NUM_DOT_PATHS> m_dotStartingPositions;
  [[nodiscard]] static auto GetStartingDotPositions(const UTILS::IGoomRand& goomRand,
                                                    const Point2dInt& centre,
                                                    float radius)
      -> std::array<Point2dInt, NUM_DOT_PATHS>;

  static constexpr float INNER_POSITION_MARGIN = 0.05F;
  void DrawNextCircle();
  void DrawNextCircleDots();
  void IncrementTs();
  [[nodiscard]] auto GetNextDotPositions() const -> std::array<Point2dInt, NUM_DOT_PATHS>;
  enum class DotOffsetType
  {
    SIN_OFFSET = 0,
    COS_OFFSET,
    SIN_COS_OFFSET,
    COS_SIN_OFFSET,
    _num
  };
  DotOffsetType m_dotOffsetType = DotOffsetType::SIN_OFFSET;
  void ChangeDotOffset();
  [[nodiscard]] auto GetNextDotOffset(float t) const -> Point2dInt;
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

  static constexpr uint32_t NUM_COLOR_STEPS = 20;
  static constexpr float T_LINE_COLOR_STEP = 1.0F / static_cast<float>(NUM_DOT_PATHS);
  std::vector<UTILS::TValue> m_colorTs;
  [[nodiscard]] static auto GetInitialColorTs() -> std::vector<UTILS::TValue>;
  void UpdateColorLerpSpeed();

  std::array<const COLOR::IColorMap*, NUM_DOT_PATHS> m_dotColorMaps{};
  std::array<const COLOR::IColorMap*, NUM_DOT_PATHS> m_dotLowColorMaps{};
  const COLOR::IColorMap* m_linesColorMap{};
  const COLOR::IColorMap* m_linesLowColorMap{};
  void ChangeDotColorMaps();
  void ChangeDotDiameters();
  [[nodiscard]] auto GetFinalColor(float brightness, const Pixel& color) const -> Pixel;
  [[nodiscard]] auto GetFinalLowColor(float brightness, const Pixel& lowColor) const -> Pixel;
  [[nodiscard]] auto GetCorrectedColor(float brightness, const Pixel& color) const -> Pixel;
};

} // namespace GOOM::VISUAL_FX::CIRCLES
