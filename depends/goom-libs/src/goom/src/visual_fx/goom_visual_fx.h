#pragma once

#include "color/random_color_maps.h"
#include "visual_fx/fx_utils/random_pixel_blender.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace GOOM
{
struct Point2dInt;
class AudioSamples;
}

namespace GOOM::VISUAL_FX
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
    COLOR::WeightedColorMaps mainColorMaps{};
    COLOR::WeightedColorMaps lowColorMaps{};
    COLOR::WeightedColorMaps extraColorMaps{};
  };
  virtual auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  virtual auto ApplySingle() noexcept -> void;
  virtual auto ApplyMultiple() noexcept -> void;
};

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

inline auto IVisualFx::ApplySingle() noexcept -> void
{
  // default does nothing
}

inline auto IVisualFx::ApplyMultiple() noexcept -> void
{
  // default does nothing
}

} // namespace GOOM::VISUAL_FX
