#ifndef VISUALIZATION_GOOM_LINES_FX_H
#define VISUALIZATION_GOOM_LINES_FX_H

#include "goom_config.h"
#include "goom_graphic.h"
#include "goomutils/spimpl.h"
#include "sound_info.h"

#include <cstddef>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

namespace GOOM
{

class IGoomDraw;
class PluginInfo;

namespace UTILS
{
class RandomColorMaps;
class SmallImageBitmaps;
} // namespace UTILS

auto GetBlackLineColor() -> Pixel;
auto GetGreenLineColor() -> Pixel;
auto GetRedLineColor() -> Pixel;

class LinesFx
{
public:
  enum class LineType
  {
    CIRCLE = 0, // (param = radius)
    H_LINE, // (param = y)
    V_LINE, // (param = x)
    _NUM // must be last - gives number of enums
  };
  static constexpr size_t NUM_LINE_TYPES = static_cast<size_t>(LineType::_NUM);

  LinesFx() noexcept = delete;

  // construit un effet de line (une ligne horitontale pour commencer)
  LinesFx(const IGoomDraw& draw,
          const std::shared_ptr<const PluginInfo>& goomInfo,
          const UTILS::SmallImageBitmaps& smallBitmaps,
          LineType srceLineType,
          float srceParam,
          const Pixel& srceColor,
          LineType destLineType,
          float destParam,
          const Pixel& destColor) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string;

  void SetWeightedColorMaps(std::shared_ptr<UTILS::RandomColorMaps> weightedMaps);

  void Start();
  auto GetRandomLineColor() const -> Pixel;

  [[nodiscard]] auto GetPower() const -> float;
  void SetPower(float val);

  static constexpr uint32_t MIN_LINE_DURATION = 80;
  [[nodiscard]] auto CanResetDestLine() const -> bool;
  void ResetDestLine(LineType newLineType,
                     float newParam,
                     float newAmplitude,
                     const Pixel& newColor);

  void DrawLines(const std::vector<int16_t>& soundData,
                 const AudioSamples::MaxMinValues& soundMinMax);

  void Finish();

private:
  bool m_enabled = true;
  class LinesImpl;
  spimpl::unique_impl_ptr<LinesImpl> m_fxImpl;
};

} // namespace GOOM
#endif
