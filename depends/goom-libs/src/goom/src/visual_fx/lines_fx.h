#pragma once

#include "goom/spimpl.h"
#include "goom_graphic.h"
#include "sound_info.h"

#include <memory>

namespace GOOM
{

namespace COLOR
{
class RandomColorMaps;
}

namespace UTILS::GRAPHICS
{
class SmallImageBitmaps;
}

namespace VISUAL_FX
{
class FxHelper;

class LinesFx
{
public:
  enum class LineType
  {
    CIRCLE = 0, // (param = radius)
    H_LINE, // (param = y)
    V_LINE, // (param = x)
    _num // must be last - gives number of enums
  };

  LinesFx() noexcept = delete;

  // construit un effet de line (une ligne horitontale pour commencer)
  LinesFx(const FxHelper& fxHelper,
          const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
          LineType srceLineType,
          float srceParam,
          const Pixel& srceColor,
          LineType destLineType,
          float destParam,
          const Pixel& destColor) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  auto Start() noexcept -> void;
  auto Finish() noexcept -> void;

  [[nodiscard]] auto GetLineColorPower() const noexcept -> float;
  auto SetLineColorPower(float val) noexcept -> void;

  static constexpr uint32_t MIN_LINE_DURATION = 80;
  [[nodiscard]] auto CanResetDestLine() const noexcept -> bool;
  auto ResetDestLine(LineType newLineType,
                     float newParam,
                     float newAmplitude,
                     const Pixel& newColor) noexcept -> void;

  auto DrawLines(const AudioSamples::SampleArray& soundData,
                 const AudioSamples::MaxMinValues& soundMinMax) noexcept -> void;

  [[nodiscard]] auto GetRandomLineColor() const -> Pixel;
  [[nodiscard]] static auto GetBlackLineColor() -> Pixel;
  [[nodiscard]] static auto GetGreenLineColor() -> Pixel;
  [[nodiscard]] static auto GetRedLineColor() -> Pixel;

private:
  class LinesImpl;
  spimpl::unique_impl_ptr<LinesImpl> m_pimpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
