module;

#include "goom/point2d.h"

#include <cstdint>
#include <string>
#include <vector>

export module Goom.VisualFx.VisualFxBase;

import Goom.Color.RandomColorMaps;
import Goom.Utils.Math.Misc;
import Goom.VisualFx.FxUtils;
import Goom.Lib.FrameData;
import Goom.Lib.GoomGraphic;
import Goom.Lib.SoundInfo;

export namespace GOOM::VISUAL_FX
{

class IVisualFx
{
public:
  IVisualFx() noexcept                           = default;
  IVisualFx(const IVisualFx&) noexcept           = delete;
  IVisualFx(IVisualFx&&) noexcept                = delete;
  virtual ~IVisualFx() noexcept                  = default;
  auto operator=(const IVisualFx&) -> IVisualFx& = delete;
  auto operator=(IVisualFx&&) -> IVisualFx&      = delete;

  [[nodiscard]] virtual auto GetFxName() const noexcept -> std::string = 0;

  virtual auto Start() noexcept -> void  = 0;
  virtual auto Finish() noexcept -> void = 0;

  virtual auto Resume() noexcept -> void;
  virtual auto Suspend() noexcept -> void;

  virtual auto Refresh() noexcept -> void;

  using PixelBlenderParams = FX_UTILS::RandomPixelBlender::PixelBlenderParams;
  virtual auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept
      -> void = 0;
  virtual auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;
  virtual auto SetSoundData(const AudioSamples& soundData) noexcept -> void;

  [[nodiscard]] virtual auto GetCurrentColorMapsNames() const noexcept
      -> std::vector<std::string> = 0;
  struct WeightedColorMaps
  {
    uint32_t id = 0;
    COLOR::WeightedRandomColorMaps mainColorMaps{};
    COLOR::WeightedRandomColorMaps lowColorMaps{};
    COLOR::WeightedRandomColorMaps extraColorMaps{};
  };
  virtual auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  virtual auto SetFrameMiscData(MiscData& miscData) noexcept -> void;

  virtual auto ApplyToImageBuffers() noexcept -> void;
};

inline constexpr auto DEFAULT_VISUAL_FX_ALPHA =
    static_cast<PixelChannelType>((3U * static_cast<uint32_t>(MAX_ALPHA)) / 4U);

[[nodiscard]] inline auto GetWeightedColorMapsWithNewAlpha(
    const IVisualFx::WeightedColorMaps& weightedColorMaps,
    const PixelChannelType newDefaultAlpha) noexcept -> IVisualFx::WeightedColorMaps
{
  return {
      weightedColorMaps.id,
      not weightedColorMaps.mainColorMaps.IsActive()
          ? weightedColorMaps.mainColorMaps
          : COLOR::WeightedRandomColorMaps{ weightedColorMaps.mainColorMaps, newDefaultAlpha},
      not weightedColorMaps.lowColorMaps.IsActive()
          ? weightedColorMaps.lowColorMaps
          : COLOR::WeightedRandomColorMaps{  weightedColorMaps.lowColorMaps, newDefaultAlpha},
      not weightedColorMaps.extraColorMaps.IsActive()
          ? weightedColorMaps.extraColorMaps
          : COLOR::WeightedRandomColorMaps{weightedColorMaps.extraColorMaps, newDefaultAlpha}
  };
}

inline auto IVisualFx::Resume() noexcept -> void
{
  // default does nothing
}

inline auto IVisualFx::Suspend() noexcept -> void
{
  // default does nothing
}

inline auto IVisualFx::Refresh() noexcept -> void
{
  // default does nothing
}

inline auto IVisualFx::SetZoomMidpoint([[maybe_unused]] const Point2dInt& zoomMidpoint) noexcept
    -> void
{
  // default does nothing
}

inline auto IVisualFx::SetWeightedColorMaps(
    [[maybe_unused]] const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  // default does nothing
}

inline auto IVisualFx::SetSoundData([[maybe_unused]] const AudioSamples& soundData) noexcept -> void
{
  // default does nothing
}

inline auto IVisualFx::SetFrameMiscData([[maybe_unused]] MiscData& miscData) noexcept -> void
{
  // default does nothing
}

inline auto IVisualFx::ApplyToImageBuffers() noexcept -> void
{
  // default does nothing
}

} // namespace GOOM::VISUAL_FX
