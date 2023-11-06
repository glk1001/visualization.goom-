#include "raindrops_fx.h"

#include "fx_helper.h"
#include "goom/goom_types.h"
#include "goom/spimpl.h"
#include "goom_visual_fx.h"
#include "raindrops/raindrops.h"
#include "visual_fx/fx_utils/random_pixel_blender.h"

#include <memory>
#include <string>
#include <vector>

namespace GOOM::VISUAL_FX
{

using FX_UTILS::RandomPixelBlender;
using RAINDROPS::Raindrops;

class RaindropsFx::RaindropsFxImpl
{
public:
  explicit RaindropsFxImpl(const FxHelper& fxHelper) noexcept;

  auto Start() noexcept -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;
  [[nodiscard]] static auto GetCurrentColorMapsNames() noexcept -> std::vector<std::string>;

  auto ApplyToImageBuffers() noexcept -> void;

private:
  const FxHelper* m_fxHelper;
  Point2dInt m_zoomMidpoint = m_fxHelper->goomInfo->GetDimensions().GetCentrePoint();

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  WeightedColorMaps m_weightedColorMaps{};
  std::unique_ptr<Raindrops> m_raindrops{};
  auto SetNumRaindrops() noexcept -> void;
  auto SetRaindropsWeightedColorMaps() noexcept -> void;
};

RaindropsFx::RaindropsFx(const FxHelper& fxHelper) noexcept
  : m_pimpl{spimpl::make_unique_impl<RaindropsFxImpl>(fxHelper)}
{
}

auto RaindropsFx::GetFxName() const noexcept -> std::string
{
  return "raindrops";
}

auto RaindropsFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto RaindropsFx::Finish() noexcept -> void
{
  // nothing to do
}

auto RaindropsFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto RaindropsFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto RaindropsFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto RaindropsFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto RaindropsFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

RaindropsFx::RaindropsFxImpl::RaindropsFxImpl(const FxHelper& fxHelper) noexcept
  : m_fxHelper{&fxHelper}, m_pixelBlender{*fxHelper.goomRand}
{
}

auto RaindropsFx::RaindropsFxImpl::Start() noexcept -> void
{
  // clang-format off
  const auto raindropRectangle = Rectangle2dInt{
      Point2dInt{0, 0},
      Point2dInt{m_fxHelper->goomInfo->GetDimensions().GetIntWidth() - 1,
                 m_fxHelper->goomInfo->GetDimensions().GetIntHeight() - 1}
  };
  // clang-format on

  m_raindrops = std::make_unique<Raindrops>(*m_fxHelper,
                                            Raindrops::NUM_START_RAINDROPS,
                                            m_weightedColorMaps.mainColorMaps,
                                            m_weightedColorMaps.lowColorMaps,
                                            raindropRectangle,
                                            m_zoomMidpoint);

  SetRaindropsWeightedColorMaps();
}

auto RaindropsFx::RaindropsFxImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto RaindropsFx::RaindropsFxImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->draw->SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

auto RaindropsFx::RaindropsFxImpl::SetZoomMidpoint(
    [[maybe_unused]] const Point2dInt& zoomMidpoint) noexcept -> void
{
  if (nullptr == m_raindrops)
  {
    return;
  }

  m_raindrops->SetRectangleWeightPoint(zoomMidpoint);
}

auto RaindropsFx::RaindropsFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_weightedColorMaps = weightedColorMaps;

  SetNumRaindrops();
  SetRaindropsWeightedColorMaps();
}

auto RaindropsFx::RaindropsFxImpl::SetNumRaindrops() noexcept -> void
{
  if (nullptr == m_raindrops)
  {
    return;
  }

  m_raindrops->SetNumRaindrops(m_fxHelper->goomRand->GetRandInRange(
      Raindrops::MIN_NUM_RAINDROPS, Raindrops::MAX_NUM_RAINDROPS + 1));
}

auto RaindropsFx::RaindropsFxImpl::SetRaindropsWeightedColorMaps() noexcept -> void
{
  if (nullptr == m_raindrops)
  {
    return;
  }

  m_raindrops->SetWeightedColorMaps(m_weightedColorMaps.mainColorMaps,
                                    m_weightedColorMaps.lowColorMaps);
}

auto RaindropsFx::RaindropsFxImpl::GetCurrentColorMapsNames() noexcept -> std::vector<std::string>
{
  return {};
}

auto RaindropsFx::RaindropsFxImpl::ApplyToImageBuffers() noexcept -> void
{
  m_raindrops->DrawRaindrops();
  m_raindrops->UpdateRaindrops();
}


} // namespace GOOM::VISUAL_FX