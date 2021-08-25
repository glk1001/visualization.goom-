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

  void Log(const GoomStats::LogStatsValueFunc& l) const;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  bool m_started = false;

  IZoomVector& m_zoomVector;
  ZoomFilterBuffers m_filterBuffers;
  void UpdateFilterBuffersSettings();

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

  void UpdateTranBuffer();
  void UpdateTranLerpFactor(int32_t switchIncr, float switchMult);
  void StartFreshTranBuffer();

  void LogState(const std::string& name) const;
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
  if (!m_enabled)
  {
    return;
  }

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
                    }},
    m_parallel{p}
{
  m_currentFilterSettings.middleX = m_screenWidth / 2;
  m_currentFilterSettings.middleY = m_screenHeight / 2;
}

void ZoomFilterFx::ZoomFilterImpl::Log(const GoomStats::LogStatsValueFunc& l) const
{
  m_zoomVector.UpdateLastStats();

  m_stats.SetLastZoomFilterSettings(m_currentFilterSettings);
  m_stats.SetLastJustChangedFilterSettings(m_pendingFilterSettings);
  m_stats.SetLastGeneralSpeed(m_currentFilterSettings.vitesse.GetRelativeSpeed());
  m_stats.SetLastPrevX(m_screenWidth);
  m_stats.SetLastPrevY(m_screenHeight);
  m_stats.SetLastTranBuffYLineStart(m_filterBuffers.GetTranBuffYLineStart());
  m_stats.SetLastTranDiffFactor(m_filterBuffers.GetTranLerpFactor());

  m_stats.Log(l);
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

  LogInfo("Starting ZoomFilterFastRgb, update {}", m_updateNum);
  LogInfo("switchIncr = {}, switchMult = {}", switchIncr, switchMult);

  m_stats.UpdateStart();
  m_stats.DoZoomFilterFastRgb();

  UpdateTranBuffer();
  UpdateTranLerpFactor(switchIncr, switchMult);

  LogState("Before CZoom");
  CZoom(pix1, pix2, numClipped);
  LogState("After CZoom");

  m_stats.UpdateEnd();
}

void ZoomFilterFx::ZoomFilterImpl::UpdateTranBuffer()
{
  m_stats.UpdateTranBufferStart();

  m_filterBuffers.UpdateTranBuffers();

  if (m_filterBuffers.GetTranBuffersState() ==
      ZoomFilterBuffers::TranBuffersState::RESET_TRAN_BUFFERS)
  {
    m_stats.DoResetTranBuffer();
  }
  else if (m_filterBuffers.GetTranBuffersState() ==
           ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS)
  {
    StartFreshTranBuffer();
  }

  m_stats.UpdateTranBufferEnd(m_currentFilterSettings.mode, m_filterBuffers.GetTranBuffersState());
}

void ZoomFilterFx::ZoomFilterImpl::StartFreshTranBuffer()
{
  // Don't start making new stripes until filter settings change.
  if (!m_pendingFilterSettings)
  {
    return;
  }

  m_stats.DoStartFreshTranBuffer();

  m_pendingFilterSettings = false;
  m_currentFilterSettings = m_nextFilterSettings;

  m_zoomVector.SetFilterSettings(m_currentFilterSettings);
  m_zoomVector.SetMaxSpeedCoeff(GetRandInRange(0.5F, 1.0F) * ZoomFilterData::MAX_MAX_SPEED_COEFF);

  UpdateFilterBuffersSettings();

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
  m_filterBuffers.SetBuffMidPoint({static_cast<int32_t>(m_currentFilterSettings.middleX),
                                   static_cast<int32_t>(m_currentFilterSettings.middleY)});
  m_filterBuffers.FilterSettingsChanged();
}

void ZoomFilterFx::ZoomFilterImpl::UpdateTranLerpFactor(const int32_t switchIncr,
                                                        const float switchMult)
{
  int32_t tranLerpFactor = m_filterBuffers.GetTranLerpFactor();

  LogInfo("before switchIncr = {} tranLerpFactor = {}", switchIncr, tranLerpFactor);
  if (switchIncr != 0)
  {
    m_stats.DoSwitchIncrNotZero();
    tranLerpFactor =
        stdnew::clamp(tranLerpFactor + switchIncr, 0, ZoomFilterBuffers::GetMaxTranLerpFactor());
  }
  LogInfo("after switchIncr = {} m_tranDiffFactor = {}", switchIncr, tranLerpFactor);

  if (!floats_equal(switchMult, 1.0F))
  {
    m_stats.DoSwitchMultNotOne();

    tranLerpFactor = static_cast<int32_t>(
        stdnew::lerp(static_cast<float>(ZoomFilterBuffers::GetMaxTranLerpFactor()),
                     static_cast<float>(tranLerpFactor), switchMult));
  }
  LogInfo("after switchMult = {} m_tranDiffFactor = {}", switchMult, tranLerpFactor);

  m_filterBuffers.SetTranLerpFactor(tranLerpFactor);
}

#ifdef NO_PARALLEL
// pure c version of the zoom filter
void ZoomFilterFx::ZoomFilterImpl::CZoom(const PixelBuffer& srceBuff,
                                         PixelBuffer& destBuff,
                                         uint32_t& numDestClipped) const
{
  //  CALLGRIND_START_INSTRUMENTATION;

  m_stats.DoCZoom();

  numDestClipped = 0;

  for (uint32_t destY = 0; destY < m_screenHeight; destY++)
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
      const uint32_t tranX = GetTranXBuffSrceDestLerp(destPos);
      const uint32_t tranY = GetTranYBuffSrceDestLerp(destPos);

      if ((tranX >= m_maxTranX) || (tranY >= m_maxTranY))
      {
        m_stats.DoTranPointClipped();
        *destRowBuff = Pixel::BLACK;
        numDestClipped++;
      }
      else
      {
#if __cplusplus <= 201402L
        const auto srceInfo = GetSourceInfo(tranX, tranY);
        const auto srceX = std::get<0>(srceInfo);
        const auto srceY = std::get<1>(srceInfo);
        const auto coeffs = std::get<2>(srceInfo);
#else
        const auto [srceX, srceY, coeffs] = GetSourceInfo(tranPx, tranPy);
#endif
        const NeighborhoodPixelArray pixelNeighbours = srceBuff.Get4RHBNeighbours(srceX, srceY);
        *destRowBuff = GetNewColor(coeffs, pixelNeighbours);
#ifndef NO_LOGGING
        if (43 < m_filterNum && m_filterNum < 51 && (*destRowBuff).Rgba() > 0xFF000000)
        {
          logInfo("destPos == {}", destPos);
          logInfo("srceX == {}", srceX);
          logInfo("srceY == {}", srceY);
          logInfo("tranX == {}", tranX);
          logInfo("tranY == {}", tranY);
          logInfo("coeffs[0] == {:x}", coeffs.c[0]);
          logInfo("coeffs[1] == {:x}", coeffs.c[1]);
          logInfo("coeffs[2] == {:x}", coeffs.c[2]);
          logInfo("coeffs[3] == {:x}", coeffs.c[3]);
          logInfo("(*destRowBuff).Rgba == {:x}", (*destRowBuff).Rgba());
        }
#endif
      }
      destPos++;
    }
  }

  //  CALLGRIND_STOP_INSTRUMENTATION;
  //  CALLGRIND_DUMP_STATS;
}
#endif

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
#ifndef NO_LOGGING
        if (43 < m_updateNum && m_updateNum < 51 && (*destRowBuff).Rgba() > 0xFF000000)
        {
          LogInfo("destPos == {}", destPos);
          LogInfo("srcePoint.x == {}", srcePoint.x);
          LogInfo("srcePoint.y == {}", srcePoint.y);
          LogInfo("tranPoint.x == {}", tranPoint.x);
          LogInfo("tranPoint.y == {}", tranPoint.y);
          LogInfo("coeffs[0] == {:x}", coeffs.c[0]);
          LogInfo("coeffs[1] == {:x}", coeffs.c[1]);
          LogInfo("coeffs[2] == {:x}", coeffs.c[2]);
          LogInfo("coeffs[3] == {:x}", coeffs.c[3]);
          LogInfo("(*destRowBuff).Rgba == {:x}", (*destRowBuff).Rgba());
        }
#endif
      }
      ++destPos;
    }
  };

  m_parallel.ForLoop(m_screenHeight, setDestPixelRow);
}

#ifdef NO_LOGGING
void ZoomFilterFx::ZoomFilterImpl::LogState([[maybe_unused]] const std::string& name) const
{
}
#else
void ZoomFilterFx::ZoomFilterImpl::LogState(const std::string& name) const
{
  LogInfo("=================================");
  LogInfo("Name: {}", name);

  LogInfo("m_screenWidth = {}", m_screenWidth);
  LogInfo("m_screenHeight = {}", m_screenHeight);
  LogInfo("m_buffSettings.allowOverexposed = {}", m_buffSettings.allowOverexposed);
  LogInfo("m_buffSettings.buffIntensity = {}", m_buffSettings.buffIntensity);
  LogInfo("m_resourcesDirectory = {}", m_resourcesDirectory);
  LogInfo("m_parallel->GetNumThreadsUsed() = {}", m_parallel->GetNumThreadsUsed());

  LogInfo("m_currentFilterSettings.mode = {}", EnumToString(m_currentFilterSettings.mode));
  LogInfo("m_currentFilterSettings.middleX = {}", m_currentFilterSettings.middleX);
  LogInfo("m_currentFilterSettings.middleY = {}", m_currentFilterSettings.middleY);
  LogInfo("m_currentFilterSettings.vitesse = {}", m_currentFilterSettings.vitesse.GetVitesse());
  LogInfo("m_currentFilterSettings.hPlaneEffect = {}", m_currentFilterSettings.hPlaneEffect);
  LogInfo("m_currentFilterSettings.hPlaneEffectAmplitude = {}",
          m_currentFilterSettings.hPlaneEffectAmplitude);
  LogInfo("m_currentFilterSettings.vPlaneEffect = {}", m_currentFilterSettings.vPlaneEffect);
  LogInfo("m_currentFilterSettings.vPlaneEffectAmplitude = {}",
          m_currentFilterSettings.vPlaneEffectAmplitude);
  LogInfo("m_currentFilterSettings.noisify = {}", m_currentFilterSettings.noisify);
  LogInfo("m_currentFilterSettings.noiseFactor = {}", m_currentFilterSettings.noiseFactor);
  LogInfo("m_currentFilterSettings.tanEffect = {}", m_currentFilterSettings.tanEffect);
  LogInfo("m_currentFilterSettings.rotateSpeed = {}", m_currentFilterSettings.rotateSpeed);
  LogInfo("m_currentFilterSettings.blockyWavy = {}", m_currentFilterSettings.blockyWavy);

  LogInfo("=================================");
}
#endif

} // namespace GOOM
