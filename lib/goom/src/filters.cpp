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
 *	-optimisation de la procedure de generation du buffer de transformation
 *		la vitesse est maintenant comprise dans [0..128] au lieu de [0..100]
 *
 *	- converted to C++14 2021-02-01 (glk)
 */

#include "filters.h"

#include "filter_data.h"
#include "filters/filter_buffers.h"
#include "filters/filter_normalized_coords.h"
#include "filters/goom_zoom_vector.h"
#include "filters/image_displacement.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "goom_stats.h"
#include "goomutils/goomrand.h"
#include "goomutils/logging_control.h"
//#undef NO_LOGGING
#include "goomutils/graphics/image_bitmaps.h"
#include "goomutils/logging.h"
#include "goomutils/mathutils.h"
#include "goomutils/parallel_utils.h"
#include "goomutils/spimpl.h"
#include "stats/filter_stats.h"
#include "v2d.h"

//#include <valgrind/callgrind.h>

#include <array>
#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <goomutils/enumutils.h>
#include <string>
#include <tuple>

namespace GOOM
{

using FILTERS::IZoomVector;
using FILTERS::NormalizedCoords;
using FILTERS::ZoomFilterBuffers;
using UTILS::EnumToString;
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
                 IZoomVector& zoomVector) noexcept;

  [[nodiscard]] auto GetResourcesDirectory() const -> const std::string&;
  void SetResourcesDirectory(const std::string& dirName);

  void SetBuffSettings(const FXBuffSettings& settings);
  [[nodiscard]] auto GetZoomVector() const -> IZoomVector&;

  void Start();

  [[nodiscard]] auto GetFilterSettings() const -> const ZoomFilterData&;
  [[nodiscard]] auto GetFilterSettingsArePending() const -> bool;

  [[nodiscard]] auto GetTranLerpFactor() const -> int32_t;

  void SetInitialFilterSettings(const ZoomFilterData& filterSettings);
  void ChangeFilterSettings(const ZoomFilterData& filterSettings);

  void ZoomFilterFastRgb(const PixelBuffer& pix1,
                         PixelBuffer& pix2,
                         int32_t switchIncr,
                         float switchMult,
                         uint32_t& numClipped);

  void Log(const GoomStats::LogStatsValueFunc& logValueFunc) const;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  bool m_started = false;

  IZoomVector& m_zoomVector;
  ZoomFilterBuffers m_filterBuffers;
  void UpdateFilterBuffersSettings();
  void UpdateImageDisplacementSettings();
  void UpdateZoomVectorSettings();

  ZoomFilterData m_currentFilterSettings{};
  ZoomFilterData m_nextFilterSettings{};
  bool m_pendingFilterSettings = false;

  FXBuffSettings m_buffSettings{};
  std::string m_resourcesDirectory{};

  Parallel& m_parallel;
  uint64_t m_updateNum = 0;
  mutable FilterStats m_stats{};

  using NeighborhoodCoeffArray = ZoomFilterBuffers::NeighborhoodCoeffArray;
  using NeighborhoodPixelArray = ZoomFilterBuffers::NeighborhoodPixelArray;
  [[nodiscard]] auto GetNewColor(const NeighborhoodCoeffArray& coeffs,
                                 const NeighborhoodPixelArray& pixels) const -> Pixel;
  [[nodiscard]] auto GetMixedColor(const NeighborhoodCoeffArray& coeffs,
                                   const NeighborhoodPixelArray& colors) const -> Pixel;
  [[nodiscard]] auto GetBlockyMixedColor(const NeighborhoodCoeffArray& coeffs,
                                         const NeighborhoodPixelArray& colors) const -> Pixel;

  void CZoom(const PixelBuffer& srceBuff, PixelBuffer& destBuff, uint32_t& numDestClipped) const;

  void UpdateTranBuffers();
  void UpdateTranLerpFactor(int32_t switchIncr, float switchMult);
  void StartFreshTranBuffers();
};

ZoomFilterFx::ZoomFilterFx(Parallel& p,
                           const std::shared_ptr<const PluginInfo>& goomInfo,
                           IZoomVector& zoomVector) noexcept
  : m_fxImpl{spimpl::make_unique_impl<ZoomFilterImpl>(p, goomInfo, zoomVector)}
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

void ZoomFilterFx::Log(const GoomStats::LogStatsValueFunc& logValueFunc) const
{
  m_fxImpl->Log(logValueFunc);
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

void ZoomFilterFx::ZoomFilterFastRgb(const PixelBuffer& pix1,
                                     PixelBuffer& pix2,
                                     const int switchIncr,
                                     const float switchMult,
                                     uint32_t& numClipped)
{
  m_fxImpl->ZoomFilterFastRgb(pix1, pix2, switchIncr, switchMult, numClipped);
}

auto ZoomFilterFx::GetFilterSettings() const -> const ZoomFilterData&
{
  return m_fxImpl->GetFilterSettings();
}

auto ZoomFilterFx::GetFilterSettingsArePending() const -> bool
{
  return m_fxImpl->GetFilterSettingsArePending();
}

auto ZoomFilterFx::GetTranLerpFactor() const -> int32_t
{
  return m_fxImpl->GetTranLerpFactor();
}

auto ZoomFilterFx::GetZoomVector() const -> IZoomVector&
{
  return m_fxImpl->GetZoomVector();
}

ZoomFilterFx::ZoomFilterImpl::ZoomFilterImpl(Parallel& p,
                                             const std::shared_ptr<const PluginInfo>& goomInfo,
                                             IZoomVector& zoomVector) noexcept
  : m_screenWidth{goomInfo->GetScreenInfo().width},
    m_screenHeight{goomInfo->GetScreenInfo().height},
    m_zoomVector{zoomVector},
    m_filterBuffers{p, goomInfo,
                    [this](const NormalizedCoords& normalizedCoords) {
                      return m_zoomVector.GetZoomPoint(normalizedCoords);
                    },
                    m_stats},
    m_parallel{p}
{
}

void ZoomFilterFx::ZoomFilterImpl::Log(const GoomStats::LogStatsValueFunc& logValueFunc) const
{
  m_zoomVector.UpdateLastStats();

  m_stats.SetLastZoomFilterSettings(m_currentFilterSettings);
  m_stats.SetLastJustChangedFilterSettings(m_pendingFilterSettings);
  m_stats.SetLastGeneralSpeed(m_currentFilterSettings.vitesse.GetRelativeSpeed());
  m_stats.SetLastPrevX(m_screenWidth);
  m_stats.SetLastPrevY(m_screenHeight);
  m_stats.SetLastTranBuffYLineStart(m_filterBuffers.GetTranBuffYLineStart());
  m_stats.SetLastTranDiffFactor(m_filterBuffers.GetTranLerpFactor());

  m_stats.Log(logValueFunc);
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
  m_buffSettings = settings;
}

auto ZoomFilterFx::ZoomFilterImpl::GetFilterSettings() const -> const ZoomFilterData&
{
  return m_currentFilterSettings;
}

auto ZoomFilterFx::ZoomFilterImpl::GetFilterSettingsArePending() const -> bool
{
  return m_pendingFilterSettings;
}

auto ZoomFilterFx::ZoomFilterImpl::GetTranLerpFactor() const -> int32_t
{
  return m_filterBuffers.GetTranLerpFactor();
}

auto ZoomFilterFx::ZoomFilterImpl::GetZoomVector() const -> IZoomVector&
{
  return m_zoomVector;
}

inline auto ZoomFilterFx::ZoomFilterImpl::GetNewColor(const NeighborhoodCoeffArray& coeffs,
                                                      const NeighborhoodPixelArray& pixels) const
    -> Pixel
{
  if (m_currentFilterSettings.blockyWavy)
  {
    return GetBlockyMixedColor(coeffs, pixels);
  }

  return GetMixedColor(coeffs, pixels);
}

inline auto ZoomFilterFx::ZoomFilterImpl::GetBlockyMixedColor(
    const NeighborhoodCoeffArray& coeffs, const NeighborhoodPixelArray& colors) const -> Pixel
{
  // Changing the color order gives a strange blocky, wavy look.
  // The order col4, col3, col2, col1 gave a black tear - no so good.
  static_assert(4 == ZoomFilterBuffers::NUM_NEIGHBOR_COEFFS, "NUM_NEIGHBOR_COEFFS must be 4.");
  const NeighborhoodPixelArray reorderedColors{colors[0], colors[2], colors[1], colors[3]};
  return GetMixedColor(coeffs, reorderedColors);
}

inline auto ZoomFilterFx::ZoomFilterImpl::GetMixedColor(const NeighborhoodCoeffArray& coeffs,
                                                        const NeighborhoodPixelArray& colors) const
    -> Pixel
{
  if (coeffs.isZero)
  {
    return Pixel::BLACK;
  }

  uint32_t multR = 0;
  uint32_t multG = 0;
  uint32_t multB = 0;
  for (size_t i = 0; i < ZoomFilterBuffers::NUM_NEIGHBOR_COEFFS; ++i)
  {
    const uint32_t& coeff = coeffs.val[i];
    const auto& color = colors[i];
    multR += static_cast<uint32_t>(color.R()) * coeff;
    multG += static_cast<uint32_t>(color.G()) * coeff;
    multB += static_cast<uint32_t>(color.B()) * coeff;
  }
  uint32_t newR = multR >> 8;
  uint32_t newG = multG >> 8;
  uint32_t newB = multB >> 8;

  constexpr uint32_t MAX_CHANNEL_COLOR = channel_limits<uint32_t>::max();
  constexpr uint8_t MAX_ALPHA = 0xFF;
  if (m_buffSettings.allowOverexposed)
  {
    return Pixel{{/*.r = */ static_cast<uint8_t>(std::min(MAX_CHANNEL_COLOR, newR)),
                  /*.g = */ static_cast<uint8_t>(std::min(MAX_CHANNEL_COLOR, newG)),
                  /*.b = */ static_cast<uint8_t>(std::min(MAX_CHANNEL_COLOR, newB)),
                  /*.a = */ MAX_ALPHA}};
  }

  const uint32_t maxVal = std::max({newR, newG, newB});
  if (maxVal > channel_limits<uint32_t>::max())
  {
    // scale all channels back
    newR = multR / maxVal;
    newG = multG / maxVal;
    newB = multB / maxVal;
  }

  return Pixel{{/*.r = */ static_cast<uint8_t>(newR),
                /*.g = */ static_cast<uint8_t>(newG),
                /*.b = */ static_cast<uint8_t>(newB),
                /*.a = */ MAX_ALPHA}};
}

void ZoomFilterFx::ZoomFilterImpl::SetInitialFilterSettings(const ZoomFilterData& filterSettings)
{
  assert(!m_started);

  m_stats.DoChangeFilterSettings(filterSettings);

  m_currentFilterSettings = filterSettings;
  m_pendingFilterSettings = false;
}

void ZoomFilterFx::ZoomFilterImpl::ChangeFilterSettings(const ZoomFilterData& filterSettings)
{
  assert(m_started);

  m_stats.DoChangeFilterSettings(filterSettings);

  m_nextFilterSettings = filterSettings;
  m_pendingFilterSettings = true;
}

void ZoomFilterFx::ZoomFilterImpl::Start()
{
  m_started = true;

  ChangeFilterSettings(m_currentFilterSettings);

  m_zoomVector.SetFilterSettings(m_currentFilterSettings);
  m_zoomVector.SetFilterStats(m_stats);

  UpdateFilterBuffersSettings();
  m_filterBuffers.Start();
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
void ZoomFilterFx::ZoomFilterImpl::ZoomFilterFastRgb(const PixelBuffer& pix1,
                                                     PixelBuffer& pix2,
                                                     const int32_t switchIncr,
                                                     const float switchMult,
                                                     uint32_t& numClipped)
{
  ++m_updateNum;

  m_stats.UpdateStart();
  m_stats.DoZoomFilterFastRgb();

  UpdateTranBuffers();
  UpdateTranLerpFactor(switchIncr, switchMult);

  CZoom(pix1, pix2, numClipped);

  m_stats.UpdateEnd();
}

void ZoomFilterFx::ZoomFilterImpl::UpdateTranBuffers()
{
  m_stats.UpdateTranBuffersStart();

  m_filterBuffers.UpdateTranBuffers();

  if (m_filterBuffers.GetTranBuffersState() ==
           ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS)
  {
    StartFreshTranBuffers();
  }

  m_stats.UpdateTranBuffersEnd(m_currentFilterSettings.mode, m_filterBuffers.GetTranBuffersState());
}

void ZoomFilterFx::ZoomFilterImpl::StartFreshTranBuffers()
{
  // Don't start making new stripes until filter settings change.
  if (!m_pendingFilterSettings)
  {
    return;
  }

  m_pendingFilterSettings = false;
  m_currentFilterSettings = m_nextFilterSettings;

  UpdateZoomVectorSettings();
  UpdateFilterBuffersSettings();
  UpdateImageDisplacementSettings();
}

inline void ZoomFilterFx::ZoomFilterImpl::UpdateZoomVectorSettings()
{
  m_zoomVector.SetFilterSettings(m_currentFilterSettings);
  m_zoomVector.SetMaxSpeedCoeff(GetRandInRange(0.5F, 1.0F) * ZoomFilterData::MAX_MAX_SPEED_COEFF);
}

inline void ZoomFilterFx::ZoomFilterImpl::UpdateImageDisplacementSettings()
{
  if (m_currentFilterSettings.imageDisplacement != nullptr)
  {
    m_currentFilterSettings.imageDisplacement->SetAmplitude(
        m_currentFilterSettings.imageDisplacementAmplitude);
    m_currentFilterSettings.imageDisplacement->SetXyColorCutoffs(
        m_currentFilterSettings.imageDisplacementXColorCutoff,
        m_currentFilterSettings.imageDisplacementXColorCutoff);
    m_currentFilterSettings.imageDisplacement->SetZoomFactor(
        m_currentFilterSettings.imageDisplacementZoomFactor);
  }
}

inline void ZoomFilterFx::ZoomFilterImpl::UpdateFilterBuffersSettings()
{
  m_filterBuffers.SetBuffMidPoint(m_currentFilterSettings.zoomMidPoint);
  m_filterBuffers.FilterSettingsChanged();
}

void ZoomFilterFx::ZoomFilterImpl::UpdateTranLerpFactor(const int32_t switchIncr,
                                                        const float switchMult)
{
  int32_t tranLerpFactor = m_filterBuffers.GetTranLerpFactor();

  if (switchIncr != 0)
  {
    m_stats.DoSwitchIncrNotZero();
    tranLerpFactor =
        stdnew::clamp(tranLerpFactor + switchIncr, 0, ZoomFilterBuffers::GetMaxTranLerpFactor());
  }

  if (!floats_equal(switchMult, 1.0F))
  {
    m_stats.DoSwitchMultNotOne();

    tranLerpFactor = static_cast<int32_t>(
        stdnew::lerp(static_cast<float>(ZoomFilterBuffers::GetMaxTranLerpFactor()),
                     static_cast<float>(tranLerpFactor), switchMult));
  }

  m_filterBuffers.SetTranLerpFactor(tranLerpFactor);
}

void ZoomFilterFx::ZoomFilterImpl::CZoom(const PixelBuffer& srceBuff,
                                         PixelBuffer& destBuff,
                                         uint32_t& numDestClipped) const
{
  m_stats.DoCZoom();

  numDestClipped = 0;

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
      const V2dInt tranPoint = m_filterBuffers.GetZoomBufferTranPoint(destPos);

      if (m_filterBuffers.IsTranPointClipped(tranPoint))
      {
        m_stats.DoTranPointClipped();
        *destRowBuff = Pixel::BLACK;
        ++numDestClipped;
      }
      else
      {
#if __cplusplus <= 201402L
        const auto srceInfo = m_filterBuffers.GetSourcePointInfo(tranPoint);
        const V2dInt srcePoint = std::get<0>(srceInfo);
        const auto coeffs = std::get<1>(srceInfo);
#else
        const auto [srcePoint, coeffs] = GetSourceInfo(tranPoint);
#endif
        const NeighborhoodPixelArray pixelNeighbours = srceBuff.Get4RHBNeighbours(
            static_cast<size_t>(srcePoint.x), static_cast<size_t>(srcePoint.y));
        *destRowBuff = GetNewColor(coeffs, pixelNeighbours);
      }
      ++destPos;
    }
  };

  m_parallel.ForLoop(m_screenHeight, setDestPixelRow);
}

} // namespace GOOM
