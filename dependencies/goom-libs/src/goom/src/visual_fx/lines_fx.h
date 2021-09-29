#pragma once

#include "goom/spimpl.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "sound_info.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace GOOM
{

class PluginInfo;

namespace COLOR
{
class RandomColorMaps;
} // namespace COLOR

namespace UTILS
{
class SmallImageBitmaps;
} // namespace UTILS

namespace DRAW
{
class IGoomDraw;
} // namespace DRAW

namespace VISUAL_FX
{

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
  LinesFx(const DRAW::IGoomDraw& draw,
          const PluginInfo& goomInfo,
          const UTILS::SmallImageBitmaps& smallBitmaps,
          LineType srceLineType,
          float srceParam,
          const Pixel& srceColor,
          LineType destLineType,
          float destParam,
          const Pixel& destColor) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

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

  [[nodiscard]] static auto GetBlackLineColor() -> Pixel;
  [[nodiscard]] static auto GetGreenLineColor() -> Pixel;
  [[nodiscard]] static auto GetRedLineColor() -> Pixel;

private:
  class LinesImpl;
  spimpl::unique_impl_ptr<LinesImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM

