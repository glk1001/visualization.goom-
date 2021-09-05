// --- CHUI EN TRAIN DE SUPPRIMER LES EXTERN RESOLX ET C_RESOLY ---
// --- CHUI EN TRAIN DE SUPPRIMER LES EXTERN RESOLX ET C_RESOLY ---

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
 *  - converted to C++14 2021-02-01 (glk)
 */

#include "filters.h"

#include "filters/filter_buffers_service.h"
#include "filters/filter_colors_service.h"
#include "filters/filter_settings.h"
#include "filters/goom_zoom_vector.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "goomutils/logging_control.h"
//#undef NO_LOGGING
#include "goomutils/logging.h"
#include "goomutils/parallel_utils.h"
#include "goomutils/spimpl.h"

//#include <valgrind/callgrind.h>

#include <array>
#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>

namespace GOOM
{

using FILTERS::FilterBuffersService;
using FILTERS::FilterColorsService;
using FILTERS::ZoomFilterBufferSettings;
using FILTERS::ZoomFilterEffectsSettings;
using UTILS::Logging;
using UTILS::Parallel;

class ZoomFilterFx::ZoomFilterImpl
{
public:
  ZoomFilterImpl() noexcept = delete;
  ZoomFilterImpl(Parallel& parallel,
                 const std::shared_ptr<const PluginInfo>& goomInfo,
                 std::unique_ptr<FilterBuffersService> filterBuffersService,
                 std::unique_ptr<FilterColorsService> filterColorsService) noexcept;

  void SetBuffSettings(const FXBuffSettings& settings);

  void Start();

  void UpdateFilterBufferSettings(const ZoomFilterBufferSettings& filterBufferSettings);
  void UpdateFilterEffectsSettings(const ZoomFilterEffectsSettings& filterEffectsSettings);

  void ZoomFilterFastRgb(const PixelBuffer& srceBuff, PixelBuffer& destBuff);

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  Parallel& m_parallel;

  std::unique_ptr<FilterBuffersService> m_filterBuffersService;
  std::unique_ptr<FilterColorsService> m_filterColorsService;

  uint64_t m_updateNum = 0;

  void CZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff) const;
};

ZoomFilterFx::ZoomFilterFx(Parallel& parallel,
                           const std::shared_ptr<const PluginInfo>& goomInfo,
                           std::unique_ptr<FilterBuffersService> filterBuffersService,
                           std::unique_ptr<FilterColorsService> filterColorsService) noexcept
  : m_fxImpl{spimpl::make_unique_impl<ZoomFilterImpl>(
        parallel, goomInfo, std::move(filterBuffersService), std::move(filterColorsService))}
{
}

void ZoomFilterFx::SetBuffSettings(const FXBuffSettings& settings)
{
  m_fxImpl->SetBuffSettings(settings);
}

void ZoomFilterFx::Start()
{
  m_fxImpl->Start();
}

void ZoomFilterFx::Finish()
{
  // No finish actions required
}

auto ZoomFilterFx::GetFxName() const -> std::string
{
  return "ZoomFilter FX";
}

void ZoomFilterFx::UpdateFilterEffectsSettings(
    const ZoomFilterEffectsSettings& filterEffectsSettings)
{
  m_fxImpl->UpdateFilterEffectsSettings(filterEffectsSettings);
}

void ZoomFilterFx::UpdateFilterBufferSettings(const ZoomFilterBufferSettings& filterBufferSettings)
{
  m_fxImpl->UpdateFilterBufferSettings(filterBufferSettings);
}

void ZoomFilterFx::ZoomFilterFastRgb(const PixelBuffer& srceBuff, PixelBuffer& destBuff)
{
  m_fxImpl->ZoomFilterFastRgb(srceBuff, destBuff);
}

ZoomFilterFx::ZoomFilterImpl::ZoomFilterImpl(
    Parallel& parallel,
    const std::shared_ptr<const PluginInfo>& goomInfo,
    std::unique_ptr<FilterBuffersService> filterBuffersService,
    std::unique_ptr<FilterColorsService> filterColorsService) noexcept
  : m_screenWidth{goomInfo->GetScreenInfo().width},
    m_screenHeight{goomInfo->GetScreenInfo().height},
    m_parallel{parallel},
    m_filterBuffersService{std::move(filterBuffersService)},
    m_filterColorsService{std::move(filterColorsService)}
{
}

inline void ZoomFilterFx::ZoomFilterImpl::SetBuffSettings(const FXBuffSettings& settings)
{
  m_filterColorsService->SetBuffSettings(settings);
}

void ZoomFilterFx::ZoomFilterImpl::UpdateFilterEffectsSettings(
    const ZoomFilterEffectsSettings& filterEffectsSettings)
{
  m_filterBuffersService->SetFilterEffectsSettings(filterEffectsSettings);
  m_filterColorsService->SetBlockyWavy(filterEffectsSettings.blockyWavy);
}

void ZoomFilterFx::ZoomFilterImpl::UpdateFilterBufferSettings(
    const ZoomFilterBufferSettings& filterBufferSettings)
{
  m_filterBuffersService->SetFilterBufferSettings(filterBufferSettings);
}

void ZoomFilterFx::ZoomFilterImpl::Start()
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
 *  So that is why you have this name, for the nostalgy of the first days of goom
 *  when it was just a tiny program writen in Turbo Pascal on my i486...
 */
void ZoomFilterFx::ZoomFilterImpl::ZoomFilterFastRgb(const PixelBuffer& srceBuff,
                                                     PixelBuffer& destBuff)
{
  ++m_updateNum;

  m_filterBuffersService->UpdateTranBuffers();

  CZoom(srceBuff, destBuff);
}

void ZoomFilterFx::ZoomFilterImpl::CZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff) const
{
  const auto setDestPixelRow = [&](const uint32_t destY)
  {
    uint32_t destPos = m_screenWidth * destY;
#if __cplusplus <= 201402L
    const auto destRowIter = destBuff.GetRowIter(destY);
    const auto destRowBegin = std::get<0>(destRowIter);
    const auto destRowEnd = std::get<1>(destRowIter);
#else
    const auto [destRowBegin, destRowEnd] = destBuff.GetRowIter(destY);
#endif
    for (auto destRowBuff = destRowBegin; destRowBuff != destRowEnd; ++destRowBuff)
    {
      const auto srceInfo = m_filterBuffersService->GetSourcePointInfo(destPos);

      *destRowBuff = m_filterColorsService->GetNewColor(srceBuff, srceInfo);

      ++destPos;
    }
  };

  m_parallel.ForLoop(m_screenHeight, setDestPixelRow);
}

} // namespace GOOM
