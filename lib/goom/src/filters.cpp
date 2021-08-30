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

#include "filter_data.h"
#include "filters/filter_buffers_service.h"
#include "filters/filter_normalized_coords.h"
#include "filters/filter_zoom_colors.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "goomutils/goomrand.h"
#include "goomutils/logging_control.h"
//#undef NO_LOGGING
#include "goomutils/logging.h"
#include "goomutils/mathutils.h"
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

using FILTERS::NormalizedCoords;
using FILTERS::ZoomFilterBuffersService;
using FILTERS::ZoomFilterColors;
using UTILS::floats_equal;
using UTILS::GetRandInRange;
using UTILS::Logging;
using UTILS::Parallel;

class ZoomFilterFx::ZoomFilterImpl
{
public:
  ZoomFilterImpl() noexcept = delete;
  ZoomFilterImpl(Parallel& p,
                 const std::shared_ptr<const PluginInfo>& goomInfo,
                 ZoomFilterBuffersService& filterBuffersService) noexcept;

  [[nodiscard]] auto GetResourcesDirectory() const -> const std::string&;
  void SetResourcesDirectory(const std::string& dirName);

  void SetBuffSettings(const FXBuffSettings& settings);

  void Start();

  [[nodiscard]] auto GetFilterSettings() const -> const ZoomFilterData&;

  void SetInitialFilterSettings(const ZoomFilterData& filterSettings);
  void ChangeFilterSettings(const ZoomFilterData& filterSettings);

  void ZoomFilterFastRgb(const PixelBuffer& srceBuff,
                         PixelBuffer& destBuff,
                         int32_t switchIncr,
                         float switchMult);

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  bool m_started = false;
  uint64_t m_updateNum = 0;

  ZoomFilterBuffersService& m_filterBuffersService;
  ZoomFilterColors m_filterColors{};

  ZoomFilterData m_currentFilterSettings{};
  std::string m_resourcesDirectory{};

  Parallel& m_parallel;
  void CZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff) const;
};

ZoomFilterFx::ZoomFilterFx(Parallel& p,
                           const std::shared_ptr<const PluginInfo>& goomInfo,
                           FILTERS::ZoomFilterBuffersService& filterBuffersService) noexcept
  : m_fxImpl{spimpl::make_unique_impl<ZoomFilterImpl>(p, goomInfo, filterBuffersService)}
{
}

auto ZoomFilterFx::GetResourcesDirectory() const -> const std::string&
{
  return m_fxImpl->GetResourcesDirectory();
}

void ZoomFilterFx::SetResourcesDirectory(const std::string& dirName)
{
  m_fxImpl->SetResourcesDirectory(dirName);
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
}

auto ZoomFilterFx::GetFxName() const -> std::string
{
  return "ZoomFilter FX";
}

void ZoomFilterFx::SetInitialFilterSettings(const ZoomFilterData& filterSettings)
{
  m_fxImpl->SetInitialFilterSettings(filterSettings);
}

void ZoomFilterFx::ChangeFilterSettings(const ZoomFilterData& filterSettings)
{
  m_fxImpl->ChangeFilterSettings(filterSettings);
}

void ZoomFilterFx::ZoomFilterFastRgb(const PixelBuffer& srceBuff,
                                     PixelBuffer& destBuff,
                                     const int switchIncr,
                                     const float switchMult)
{
  m_fxImpl->ZoomFilterFastRgb(srceBuff, destBuff, switchIncr, switchMult);
}

auto ZoomFilterFx::GetFilterSettings() const -> const ZoomFilterData&
{
  return m_fxImpl->GetFilterSettings();
}

ZoomFilterFx::ZoomFilterImpl::ZoomFilterImpl(
    Parallel& p,
    const std::shared_ptr<const PluginInfo>& goomInfo,
    ZoomFilterBuffersService& filterBuffersService) noexcept
  : m_screenWidth{goomInfo->GetScreenInfo().width},
    m_screenHeight{goomInfo->GetScreenInfo().height},
    m_filterBuffersService{filterBuffersService},
    m_parallel{p}
{
}

inline auto ZoomFilterFx::ZoomFilterImpl::GetResourcesDirectory() const -> const std::string&
{
  return m_resourcesDirectory;
}

inline void ZoomFilterFx::ZoomFilterImpl::SetResourcesDirectory(const std::string& dirName)
{
  m_resourcesDirectory = dirName;
}

inline void ZoomFilterFx::ZoomFilterImpl::SetBuffSettings(const FXBuffSettings& settings)
{
  m_filterColors.SetBuffSettings(settings);
}

auto ZoomFilterFx::ZoomFilterImpl::GetFilterSettings() const -> const ZoomFilterData&
{
  return m_currentFilterSettings;
}

// TODO Is this necessary?
void ZoomFilterFx::ZoomFilterImpl::SetInitialFilterSettings(const ZoomFilterData& filterSettings)
{
  assert(!m_started);

  m_currentFilterSettings = filterSettings;
  m_filterColors.SetBlockWavy(filterSettings.blockyWavy);
}

void ZoomFilterFx::ZoomFilterImpl::ChangeFilterSettings(const ZoomFilterData& filterSettings)
{
  assert(m_started);

  m_currentFilterSettings = filterSettings;
  m_filterBuffersService.ChangeFilterSettings(filterSettings);
  m_filterColors.SetBlockWavy(filterSettings.blockyWavy);
}

void ZoomFilterFx::ZoomFilterImpl::Start()
{
  m_started = true;

  m_filterBuffersService.Start();

  ChangeFilterSettings(m_currentFilterSettings);
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
                                                     PixelBuffer& destBuff,
                                                     const int32_t switchIncr,
                                                     const float switchMult)
{
  ++m_updateNum;

  m_filterBuffersService.UpdateTranBuffers();
  m_filterBuffersService.UpdateTranLerpFactor(switchIncr, switchMult);

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
      const auto srceInfo = m_filterBuffersService.GetSourcePointInfo(destPos);

      *destRowBuff = m_filterColors.GetNewColor(srceBuff, srceInfo);

      ++destPos;
    }
  };

  m_parallel.ForLoop(m_screenHeight, setDestPixelRow);
}

} // namespace GOOM
