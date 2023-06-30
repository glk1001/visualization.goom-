/* filter.c version 0.7
 * contient les filtres applicable a un buffer
 * creation : 01/10/2000
 *  -ajout de sinFilter()
 *  -ajout de zoomFilter()
 *  -copie de zoomFilter() en zoomFilterRGB(), gerant les 3 couleurs
 *  -optimisation de sinFilter (utilisant une table de sin)
 *  -asm
 *  -optimisation de la procedure de generation du buffer de transformation
 *     la vitesse est maintenant comprise dans [0..128] au lieu de [0..100]
 *
 *  - converted to C++17 2021-02-01 (glk)
 */

//#undef NO_LOGGING

#include "zoom_filter_fx.h"

#include "filter_buffers_service.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_logger.h"
#include "goom_plugin_info.h"
#include "goom_types.h"
#include "spimpl.h"
#include "utils/name_value_pairs.h"
#include "utils/parallel_utils.h"
#include "utils/propagate_const.h"

#include <memory>

namespace GOOM::FILTER_FX
{

using std::experimental::propagate_const;
using UTILS::GetPair;
using UTILS::MoveNameValuePairs;
using UTILS::NameValuePairs;

class ZoomFilterFx::ZoomFilterImpl
{
public:
  ZoomFilterImpl(const PluginInfo& goomInfo,
                 std::unique_ptr<FilterBuffersService> filterBuffersService) noexcept;

  [[nodiscard]] auto IsTranBufferFltReady() const noexcept -> bool;
  auto CopyTranBufferFlt(std_spn::span<Point2dFlt>& destBuff) noexcept -> void;

  auto Start() noexcept -> void;

  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> uint32_t;

  [[nodiscard]] auto GetFilterEffectsSettings() const noexcept -> const ZoomFilterEffectsSettings&;
  auto UpdateFilterEffectsSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) noexcept
      -> void;
  auto UpdateFilterBufferSettings(const ZoomFilterBufferSettings& filterBufferSettings) noexcept
      -> void;
  auto UpdateFilterColorSettings(bool blockyWavy) noexcept -> void;

  auto ZoomFilterFastRgb(const PixelBuffer& srceBuff, PixelBuffer& destBuff) noexcept -> void;

  [[nodiscard]] auto GetNameValueParams() const noexcept -> NameValuePairs;

  [[nodiscard]] auto GetFilterBuffersService() noexcept -> FilterBuffersService&;

private:
  Dimensions m_dimensions;

  propagate_const<std::unique_ptr<FilterBuffersService>> m_filterBuffersService;

  uint64_t m_updateNum = 0;
};

ZoomFilterFx::ZoomFilterFx(const PluginInfo& goomInfo,
                           std::unique_ptr<FilterBuffersService> filterBuffersService) noexcept
  : m_pimpl{spimpl::make_unique_impl<ZoomFilterImpl>(goomInfo, std::move(filterBuffersService))}
{
}

auto ZoomFilterFx::IsTranBufferFltReady() const noexcept -> bool
{
  return m_pimpl->IsTranBufferFltReady();
}

auto ZoomFilterFx::CopyTranBufferFlt(std_spn::span<Point2dFlt>& destBuff) noexcept -> void
{
  m_pimpl->CopyTranBufferFlt(destBuff);
}

auto ZoomFilterFx::GetNameValueParams() const noexcept -> NameValuePairs
{
  return m_pimpl->GetNameValueParams();
}

auto ZoomFilterFx::GetFilterBuffersService() noexcept -> FilterBuffersService&
{
  return m_pimpl->GetFilterBuffersService();
}

auto ZoomFilterFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto ZoomFilterFx::Finish() noexcept -> void
{
  // No finish actions required
}

auto ZoomFilterFx::GetTranLerpFactor() const noexcept -> uint32_t
{
  return m_pimpl->GetTranLerpFactor();
}

auto ZoomFilterFx::UpdateFilterEffectsSettings(
    const ZoomFilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  m_pimpl->UpdateFilterEffectsSettings(filterEffectsSettings);
}

auto ZoomFilterFx::UpdateFilterBufferSettings(
    const ZoomFilterBufferSettings& filterBufferSettings) noexcept -> void
{
  m_pimpl->UpdateFilterBufferSettings(filterBufferSettings);
}

auto ZoomFilterFx::UpdateFilterColorSettings(const bool blockyWavy) noexcept -> void
{
  m_pimpl->UpdateFilterColorSettings(blockyWavy);
}

auto ZoomFilterFx::ZoomFilterFastRgb(const PixelBuffer& srceBuff, PixelBuffer& destBuff) noexcept
    -> void
{
  m_pimpl->ZoomFilterFastRgb(srceBuff, destBuff);
}

ZoomFilterFx::ZoomFilterImpl::ZoomFilterImpl(
    const PluginInfo& goomInfo, std::unique_ptr<FilterBuffersService> filterBuffersService) noexcept
  : m_dimensions{goomInfo.GetDimensions()}, m_filterBuffersService{std::move(filterBuffersService)}
{
}


inline auto ZoomFilterFx::ZoomFilterImpl::IsTranBufferFltReady() const noexcept -> bool
{
  return m_filterBuffersService->IsTranBufferFltReady();
}

inline auto ZoomFilterFx::ZoomFilterImpl::CopyTranBufferFlt(
    std_spn::span<Point2dFlt>& destBuff) noexcept -> void
{
  m_filterBuffersService->CopyTranBufferFlt(destBuff);
}

inline auto ZoomFilterFx::ZoomFilterImpl::GetTranLerpFactor() const noexcept -> uint32_t
{
  return m_filterBuffersService->GetTranLerpFactor();
}

inline auto ZoomFilterFx::ZoomFilterImpl::GetFilterEffectsSettings() const noexcept
    -> const ZoomFilterEffectsSettings&
{
  return m_filterBuffersService->GetCurrentFilterEffectsSettings();
}

inline auto ZoomFilterFx::ZoomFilterImpl::UpdateFilterEffectsSettings(
    const ZoomFilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  m_filterBuffersService->SetFilterEffectsSettings(filterEffectsSettings);
}

inline auto ZoomFilterFx::ZoomFilterImpl::UpdateFilterColorSettings(
    [[maybe_unused]] const bool blockyWavy) noexcept -> void
{
}

inline auto ZoomFilterFx::ZoomFilterImpl::UpdateFilterBufferSettings(
    const ZoomFilterBufferSettings& filterBufferSettings) noexcept -> void
{
  m_filterBuffersService->SetFilterBufferSettings(filterBufferSettings);
}

auto ZoomFilterFx::ZoomFilterImpl::GetNameValueParams() const noexcept -> NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "ZoomFilterFx";

  auto nameValuePairs = NameValuePairs{GetPair(PARAM_GROUP, "tranLerpFactor", GetTranLerpFactor())};

  MoveNameValuePairs(m_filterBuffersService->GetNameValueParams(PARAM_GROUP), nameValuePairs);

  return nameValuePairs;
}

inline auto ZoomFilterFx::ZoomFilterImpl::GetFilterBuffersService() noexcept
    -> FilterBuffersService&
{
  return *m_filterBuffersService;
}

inline auto ZoomFilterFx::ZoomFilterImpl::Start() noexcept -> void
{
  m_filterBuffersService->Start();
}

/**
 * Main work for the dynamic displacement map.
 *
 * Reads data from pix1, write to pix2.
 *
 * Useful data for this FX are stored in ZoomFilterData.
 *
 * If you think that this is a strange function name, let me say that a long time ago,
 *  there has been a slow version and a gray-level only one. Then came these function,
 *  fast and working in RGB colorspace ! nice but it only was applying a zoom to the image.
 *  So that is why you have this name, for the nostalgia of the first days of goom
 *  when it was just a tiny program writen in Turbo Pascal on my i486...
 */
inline auto ZoomFilterFx::ZoomFilterImpl::ZoomFilterFastRgb(
    [[maybe_unused]] const PixelBuffer& srceBuff, [[maybe_unused]] PixelBuffer& destBuff) noexcept
    -> void
{
  ++m_updateNum;

  m_filterBuffersService->UpdateTranBuffers();

  //  CZoom(srceBuff, destBuff);
}

#if __clang_major__ >= 16
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif

#if __clang_major__ >= 16
#pragma GCC diagnostic pop
#endif

} // namespace GOOM::FILTER_FX
