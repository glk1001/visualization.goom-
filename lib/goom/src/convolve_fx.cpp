#include "convolve_fx.h"

#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "goomutils/colorutils.h"
#include "goomutils/parallel_utils.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>

namespace GOOM
{

using namespace GOOM::UTILS;

class ConvolveFx::ConvolveImpl
{
public:
  explicit ConvolveImpl(Parallel& p, std::shared_ptr<const PluginInfo> goomInfo) noexcept;
  ~ConvolveImpl() noexcept = default;
  ConvolveImpl(const ConvolveImpl&) noexcept = delete;
  ConvolveImpl(ConvolveImpl&&) noexcept = delete;
  auto operator=(const ConvolveImpl&) -> ConvolveImpl& = delete;
  auto operator=(ConvolveImpl&&) -> ConvolveImpl& = delete;

  void SetBuffSettings(const FXBuffSettings& settings);
  void SetAllowOverexposed(bool val);

  void Convolve(const PixelBuffer& currentBuff, PixelBuffer& outputBuff);

private:
  Parallel* const m_parallel;

  const std::shared_ptr<const PluginInfo> m_goomInfo;
  float m_screenBrightness = 100;
  float m_flashIntensity = 30;
  float m_factor = 0.5;
  FXBuffSettings m_buffSettings{FXBuffSettings::INITIAL_BUFF_INTENSITY, false};

  void CreateOutputWithBrightness(const PixelBuffer& srceBuff,
                                  PixelBuffer& destBuff,
                                  uint32_t flashInt);
};

ConvolveFx::ConvolveFx(Parallel& p, const std::shared_ptr<const PluginInfo>& info) noexcept
  : m_fxImpl{new ConvolveImpl{p, info}}
{
}

ConvolveFx::~ConvolveFx() noexcept = default;

void ConvolveFx::SetBuffSettings(const FXBuffSettings& settings)
{
  m_fxImpl->SetBuffSettings(settings);
}

void ConvolveFx::SetAllowOverexposed(const bool val)
{
  m_fxImpl->SetAllowOverexposed(val);
}


void ConvolveFx::Start()
{
}

void ConvolveFx::Finish()
{
}

auto ConvolveFx::GetFxName() const -> std::string
{
  return "Convolve FX";
}

void ConvolveFx::Convolve(const PixelBuffer& currentBuff, PixelBuffer& outputBuff)
{
  if (!m_enabled)
  {
    return;
  }

  m_fxImpl->Convolve(currentBuff, outputBuff);
}

ConvolveFx::ConvolveImpl::ConvolveImpl(Parallel& p, std::shared_ptr<const PluginInfo> info) noexcept
  : m_parallel{&p}, m_goomInfo{std::move(info)}
{
}

inline void ConvolveFx::ConvolveImpl::SetBuffSettings(const FXBuffSettings& settings)
{
  m_buffSettings = settings;
}

inline void ConvolveFx::ConvolveImpl::SetAllowOverexposed(const bool val)
{
  m_buffSettings.allowOverexposed = val;
}

void ConvolveFx::ConvolveImpl::Convolve(const PixelBuffer& currentBuff, PixelBuffer& outputBuff)
{
  currentBuff.CopyTo(outputBuff);
  return;

  const float flash = (m_factor * m_flashIntensity + m_screenBrightness) / 100.0F;
  const auto flashInt = static_cast<uint32_t>(std::round(flash * 256 + 0.0001F));
  constexpr float INCREASE_RATE = 1.3F;
  constexpr float DECAY_RATE = 0.955F;
  if (m_goomInfo->GetSoundInfo().GetTimeSinceLastGoom() == 0)
  {
    m_factor += m_goomInfo->GetSoundInfo().GetGoomPower() * INCREASE_RATE;
  }
  m_factor *= DECAY_RATE;

  constexpr float TOLERANCE = 0.02F;
  if (std::fabs(1.0 - flash) < TOLERANCE)
  {
    currentBuff.CopyTo(outputBuff);
  }
  else
  {
    CreateOutputWithBrightness(currentBuff, outputBuff, flashInt);
  }
}

void ConvolveFx::ConvolveImpl::CreateOutputWithBrightness(const PixelBuffer& srceBuff,
                                                          PixelBuffer& destBuff,
                                                          const uint32_t flashInt)
{
  const auto setDestPixelRow = [&](const uint32_t y) {
#if __cplusplus <= 201402L
    const auto srceRowIter = srceBuff.GetRowIter(y);
    const auto srceRowBegin = std::get<0>(srceRowIter);
    const auto srceRowEnd = std::get<1>(srceRowIter);
    const auto destRowIter = destBuff.GetRowIter(y);
    const auto destRowBegin = std::get<0>(destRowIter);
#else
    const auto [srceRowBegin, srceRowEnd] = srceBuff.GetRowIter(y);
    const auto [destRowBegin, destRowEnd] = destBuff.GetRowIter(y);
#endif
    std::transform(srceRowBegin, srceRowEnd, destRowBegin, [&](const Pixel& srce) {
      return GetBrighterColorInt(flashInt, srce, m_buffSettings.allowOverexposed);
    });
  };

  m_parallel->ForLoop(m_goomInfo->GetScreenInfo().height, setDestPixelRow);
}

} // namespace GOOM
