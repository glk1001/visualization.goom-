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

#include "zoom_filter_fx.h"

#include "filters/filter_buffer_row_color_info.h"
#include "filters/filter_buffers_service.h"
#include "filters/filter_colors_service.h"
#include "goom/logging_control.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
//#undef NO_LOGGING
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "utils/name_value_pairs.h"
#include "utils/parallel_utils.h"

#include <array>
#undef NDEBUG
#include <cstdint>
#include <memory>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
#else
namespace GOOM::VISUAL_FX
{
#endif

using UTILS::GetPair;
using UTILS::Logging;
using UTILS::MoveNameValuePairs;
using UTILS::NameValuePairs;
using UTILS::Parallel;
using VISUAL_FX::FILTERS::FilterBufferRowColorInfo;
using VISUAL_FX::FILTERS::FilterBuffersService;
using VISUAL_FX::FILTERS::FilterColorsService;
using VISUAL_FX::FILTERS::ZoomFilterBufferSettings;
using VISUAL_FX::FILTERS::ZoomFilterColorSettings;
using VISUAL_FX::FILTERS::ZoomFilterEffectsSettings;

class ZoomFilterFx::ZoomFilterImpl
{
public:
  ZoomFilterImpl() noexcept = delete;
  ZoomFilterImpl(Parallel& parallel,
                 const PluginInfo& goomInfo,
                 std::unique_ptr<FilterBuffersService> filterBuffersService,
                 std::unique_ptr<FilterColorsService> filterColorsService) noexcept;

  void SetBuffSettings(const FXBuffSettings& settings);

  void Start();

  [[nodiscard]] auto GetTranLerpFactor() const -> int32_t;

  void UpdateFilterBufferSettings(const ZoomFilterBufferSettings& filterBufferSettings);
  void UpdateFilterEffectsSettings(const ZoomFilterEffectsSettings& filterEffectsSettings);
  void UpdateFilterColorSettings(const ZoomFilterColorSettings& filterColorSettings);

  void ZoomFilterFastRgb(const PixelBuffer& srceBuff, PixelBuffer& destBuff);
  [[nodiscard]] auto GetLastFilterBufferColorInfo() const
      -> const std::vector<FilterBufferRowColorInfo>&;

  [[nodiscard]] auto GetNameValueParams() const -> NameValuePairs;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  Parallel& m_parallel;

  std::unique_ptr<FilterBuffersService> m_filterBuffersService;
  std::unique_ptr<FilterColorsService> m_filterColorsService;

  uint64_t m_updateNum = 0;

  std::vector<FilterBufferRowColorInfo> m_filterBufferColorInfo;
  void CZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff);
};

ZoomFilterFx::ZoomFilterFx(Parallel& parallel,
                           const PluginInfo& goomInfo,
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

auto ZoomFilterFx::GetNameValueParams() const -> NameValuePairs
{
  return m_fxImpl->GetNameValueParams();
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

auto ZoomFilterFx::GetTranLerpFactor() const -> int32_t
{
  return m_fxImpl->GetTranLerpFactor();
}

void ZoomFilterFx::UpdateFilterBufferSettings(const ZoomFilterBufferSettings& filterBufferSettings)
{
  m_fxImpl->UpdateFilterBufferSettings(filterBufferSettings);
}

void ZoomFilterFx::UpdateFilterEffectsSettings(
    const ZoomFilterEffectsSettings& filterEffectsSettings)
{
  m_fxImpl->UpdateFilterEffectsSettings(filterEffectsSettings);
}

void ZoomFilterFx::UpdateFilterColorSettings(const ZoomFilterColorSettings& filterColorSettings)
{
  m_fxImpl->UpdateFilterColorSettings(filterColorSettings);
}

void ZoomFilterFx::ZoomFilterFastRgb(const PixelBuffer& srceBuff, PixelBuffer& destBuff)
{
  m_fxImpl->ZoomFilterFastRgb(srceBuff, destBuff);
}

auto ZoomFilterFx::GetLastFilterBufferColorInfo() const
    -> const std::vector<FilterBufferRowColorInfo>&
{
  return m_fxImpl->GetLastFilterBufferColorInfo();
}

ZoomFilterFx::ZoomFilterImpl::ZoomFilterImpl(
    Parallel& parallel,
    const PluginInfo& goomInfo,
    std::unique_ptr<FilterBuffersService> filterBuffersService,
    std::unique_ptr<FilterColorsService> filterColorsService) noexcept
  : m_screenWidth{goomInfo.GetScreenInfo().width},
    m_screenHeight{goomInfo.GetScreenInfo().height},
    m_parallel{parallel},
    m_filterBuffersService{std::move(filterBuffersService)},
    m_filterColorsService{std::move(filterColorsService)},
    m_filterBufferColorInfo(m_screenHeight)
{
}

inline void ZoomFilterFx::ZoomFilterImpl::SetBuffSettings(const FXBuffSettings& settings)
{
  m_filterColorsService->SetBuffSettings(settings);
}

inline auto ZoomFilterFx::ZoomFilterImpl::GetTranLerpFactor() const -> int32_t
{
  return m_filterBuffersService->GetTranLerpFactor();
}

inline void ZoomFilterFx::ZoomFilterImpl::UpdateFilterEffectsSettings(
    const ZoomFilterEffectsSettings& filterEffectsSettings)
{
  m_filterBuffersService->SetFilterEffectsSettings(filterEffectsSettings);
}

inline void ZoomFilterFx::ZoomFilterImpl::UpdateFilterColorSettings(
    const ZoomFilterColorSettings& filterColorSettings)
{
  m_filterColorsService->SetBlockyWavy(filterColorSettings.blockyWavy);
}

inline void ZoomFilterFx::ZoomFilterImpl::UpdateFilterBufferSettings(
    const ZoomFilterBufferSettings& filterBufferSettings)
{
  m_filterBuffersService->SetFilterBufferSettings(filterBufferSettings);
}

auto ZoomFilterFx::ZoomFilterImpl::GetNameValueParams() const -> NameValuePairs
{
  constexpr const char* PARAM_GROUP = "ZoomFilterFx";

  NameValuePairs nameValuePairs{GetPair(PARAM_GROUP, "tranLerpFactor", GetTranLerpFactor())};

  MoveNameValuePairs(m_filterColorsService->GetNameValueParams(PARAM_GROUP), nameValuePairs);
  MoveNameValuePairs(m_filterBuffersService->GetNameValueParams(PARAM_GROUP), nameValuePairs);

  return nameValuePairs;
}

inline void ZoomFilterFx::ZoomFilterImpl::Start()
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
inline void ZoomFilterFx::ZoomFilterImpl::ZoomFilterFastRgb(const PixelBuffer& srceBuff,
                                                            PixelBuffer& destBuff)
{
  ++m_updateNum;

  m_filterBuffersService->UpdateTranBuffers();

  CZoom(srceBuff, destBuff);
}

inline auto ZoomFilterFx::ZoomFilterImpl::GetLastFilterBufferColorInfo() const
    -> const std::vector<FilterBufferRowColorInfo>&
{
  return m_filterBufferColorInfo;
}

void ZoomFilterFx::ZoomFilterImpl::CZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff)
{
  const auto setDestPixelRow = [&](const size_t destY)
  {
    uint32_t destPos = m_screenWidth * static_cast<uint32_t>(destY);
#if __cplusplus <= 201402L
    const auto destRowIter = destBuff.GetRowIter(destY);
    const auto destRowBegin = std::get<0>(destRowIter);
    const auto destRowEnd = std::get<1>(destRowIter);
#else
    const auto [destRowBegin, destRowEnd] = destBuff.GetRowIter(destY);
#endif
    FilterBufferRowColorInfo& filterBufferRowColorInfo = m_filterBufferColorInfo[destY];
    filterBufferRowColorInfo.Reset();

    for (auto destRowBuff = destRowBegin; destRowBuff != destRowEnd; ++destRowBuff)
    {
      *destRowBuff = m_filterColorsService->GetNewColor(
          srceBuff, m_filterBuffersService->GetSourcePointInfo(destPos));

      filterBufferRowColorInfo.UpdateColor(*destRowBuff);

      ++destPos;
    }
  };

  m_parallel.ForLoop(m_screenHeight, setDestPixelRow);
}

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
