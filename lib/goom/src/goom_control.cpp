/**
* file: goom_control.c (onetime goom_core.c)
 * author: Jean-Christophe Hoelt (which is not so proud of it)
 *
 * Contains the core of goom's work.
 *
 * (c)2000-2003, by iOS-software.
 *
 *  - converted to C++14 2021-02-01 (glk)
 *
 */

//#define SHOW_STATE_TEXT_ON_SCREEN

#include "goom_control.h"

#include "control/goom_all_visual_fx.h"
#include "control/goom_events.h"
#include "control/goom_image_buffers.h"
#include "control/goom_message_displayer.h"
#include "control/goom_music_settings_reactor.h"
#include "control/goom_title_displayer.h"
#include "draw/goom_draw_to_buffer.h"
#include "filters/filter_buffers_service.h"
#include "filters/filter_colors_service.h"
#include "filters/filter_settings_service.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "goomutils/graphics/small_image_bitmaps.h"
#include "goomutils/logging_control.h"
#undef NO_LOGGING
#include "goomutils/logging.h"
#ifdef SHOW_STATE_TEXT_ON_SCREEN
#include "goomutils/name_value_pairs.h"
#endif
#include "goomutils/parallel_utils.h"
#include "goomutils/spimpl.h"

#include <cstdint>
#if __cplusplus > 201402L
#include <filesystem>
#endif
//#undef NDEBUG
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#if __cplusplus > 201402L
#include <variant>
#endif
#include <vector>

namespace GOOM
{

using CONTROL::GoomAllVisualFx;
using CONTROL::GoomDrawable;
using CONTROL::GoomEvents;
using CONTROL::GoomImageBuffers;
using CONTROL::GoomMessageDisplayer;
using CONTROL::GoomMusicSettingsReactor;
using CONTROL::GoomTitleDisplayer;
using DRAW::GoomDrawToBuffer;
using FILTERS::FilterBuffersService;
using FILTERS::FilterColorsService;
using FILTERS::FilterSettingsService;
#ifdef SHOW_STATE_TEXT_ON_SCREEN
using UTILS::GetNameValuesString;
#endif
using UTILS::Logging;
using UTILS::Parallel;
using UTILS::SmallImageBitmaps;

class GoomControl::GoomControlImpl
{
public:
  GoomControlImpl(uint32_t screenWidth, uint32_t screenHeight, std::string resourcesDirectory);

  void Swap(GoomControl::GoomControlImpl& other) noexcept = delete;

  void SetScreenBuffer(const std::shared_ptr<PixelBuffer>& buffer);

  [[nodiscard]] auto GetScreenWidth() const -> uint32_t;
  [[nodiscard]] auto GetScreenHeight() const -> uint32_t;

  void Start();
  void Finish();

  void Update(const AudioSamples& soundData,
              float fps,
              const std::string& songTitle,
              const std::string& message);

private:
  Parallel m_parallel{-1}; // max cores - 1
  WritablePluginInfo m_goomInfo;
  GoomDrawToBuffer m_multiBufferDraw;
  GoomImageBuffers m_imageBuffers;
  const std::string m_resourcesDirectory;
  FilterSettingsService m_filterSettingsService;
  const SmallImageBitmaps m_smallBitmaps;
  GoomAllVisualFx m_visualFx;
  GoomEvents m_goomEvents{};

  GoomMusicSettingsReactor m_musicSettingsReactor;

  void ProcessAudio(const AudioSamples& soundData);

  void UpdateFilterSettings();
  void ApplyZoom();
  void UpdateBuffers();
  void RotateDrawBuffers();
  [[nodiscard]] auto GetCurrentBuffers() const -> std::vector<PixelBuffer*>;
  void ResetDrawBuffSettings(const FXBuffSettings& settings);

  void ApplyCurrentStateToSingleBuffer();
  void ApplyCurrentStateToMultipleBuffers();

  void DisplayLinesIfInAGoom(const AudioSamples& soundData);
  void DisplayLines(const AudioSamples& soundData);

  std::string m_currentSongTitle{};
  GoomDrawToBuffer m_goomTextOutput;
  GoomTitleDisplayer m_goomTitleDisplayer;
  GoomMessageDisplayer m_messageDisplayer;
  auto GetFontDirectory() const -> std::string;
  void DisplayTitle(const std::string& songTitle, const std::string& message, float fps);
  void UpdateMessages(const std::string& messages);
  auto GetMessagesFontFile() const -> std::string;
#ifdef SHOW_STATE_TEXT_ON_SCREEN
  void DisplayStateText();
#endif
};

auto GoomControl::GetRandSeed() -> uint64_t
{
  return GOOM::UTILS::GetRandSeed();
}

void GoomControl::SetRandSeed(const uint64_t seed)
{
  LogDebug("Set goom seed = {}.", seed);
  GOOM::UTILS::SetRandSeed(seed);
}

GoomControl::GoomControl(const uint32_t width,
                         const uint32_t height,
                         const std::string& resourcesDirectory)
  : m_controller{spimpl::make_unique_impl<GoomControlImpl>(width, height, resourcesDirectory)}
{
}

void GoomControl::SetScreenBuffer(const std::shared_ptr<PixelBuffer>& buffer)
{
  m_controller->SetScreenBuffer(buffer);
}

void GoomControl::Start()
{
  m_controller->Start();
}

void GoomControl::Finish()
{
  m_controller->Finish();
}

void GoomControl::Update(const AudioSamples& s,
                         const float fps,
                         const std::string& songTitle,
                         const std::string& message)
{
  m_controller->Update(s, fps, songTitle, message);
}

GoomControl::GoomControlImpl::GoomControlImpl(const uint32_t screenWidth,
                                              const uint32_t screenHeight,
                                              std::string resourcesDirectory)
  : m_goomInfo{screenWidth, screenHeight},
    m_multiBufferDraw{screenWidth, screenHeight},
    m_imageBuffers{screenWidth, screenHeight},
    m_resourcesDirectory{std::move(resourcesDirectory)},
    m_filterSettingsService{m_parallel, m_goomInfo, m_resourcesDirectory},
    m_smallBitmaps{m_resourcesDirectory},
    m_visualFx{m_parallel,
               m_multiBufferDraw,
               m_goomInfo,
               m_smallBitmaps,
               m_filterSettingsService.GetFilterBuffersService(),
               FilterSettingsService::GetFilterColorsService()},
    m_musicSettingsReactor{m_goomInfo, m_visualFx, m_goomEvents, m_filterSettingsService},
    m_goomTextOutput{screenWidth, screenHeight},
    m_goomTitleDisplayer{m_goomTextOutput, GetFontDirectory()},
    m_messageDisplayer{m_goomTextOutput, GetMessagesFontFile()}
{
  RotateDrawBuffers();
}

inline auto GoomControl::GoomControlImpl::GetFontDirectory() const -> std::string
{
  return m_resourcesDirectory + PATH_SEP + FONTS_DIR;
}

inline auto GoomControl::GoomControlImpl::GetMessagesFontFile() const -> std::string
{
  return GetFontDirectory() + PATH_SEP + "verdana.ttf";
}

inline void GoomControl::GoomControlImpl::SetScreenBuffer(
    const std::shared_ptr<PixelBuffer>& buffer)
{
  m_imageBuffers.SetOutputBuff(buffer);
}

inline auto GoomControl::GoomControlImpl::GetScreenWidth() const -> uint32_t
{
  return m_goomInfo.GetScreenInfo().width;
}

inline auto GoomControl::GoomControlImpl::GetScreenHeight() const -> uint32_t
{
  return m_goomInfo.GetScreenInfo().height;
}

void GoomControl::GoomControlImpl::Start()
{
  m_filterSettingsService.Start();
  UpdateFilterSettings();

  m_visualFx.SetResetDrawBuffSettingsFunc(
      [this](const FXBuffSettings& settings) { ResetDrawBuffSettings(settings); });
  m_visualFx.ChangeColorMaps();
  m_visualFx.Start();

  m_musicSettingsReactor.Start();
}

void GoomControl::GoomControlImpl::Finish()
{
  m_visualFx.Finish();
}

void GoomControl::GoomControlImpl::Update(const AudioSamples& soundData,
                                          const float fps,
                                          const std::string& songTitle,
                                          const std::string& message)
{
  m_musicSettingsReactor.NewCycle();

  // Elargissement de l'intervalle d'évolution des points!
  // Calcul du deplacement des petits points ...
  // Widening of the interval of evolution of the points!
  // Calculation of the displacement of small points ...

  ProcessAudio(soundData);

  m_musicSettingsReactor.ChangeFilterModeIfMusicChanges();
  m_musicSettingsReactor.BigUpdateIfNotLocked();
  m_musicSettingsReactor.BigBreakIfMusicIsCalm();

  m_musicSettingsReactor.RegularlyLowerTheSpeed();

  m_musicSettingsReactor.ChangeZoomEffects();

  ApplyCurrentStateToSingleBuffer();
  ApplyZoom();
  ApplyCurrentStateToMultipleBuffers();

  // Gestion du Scope - Scope management
  m_musicSettingsReactor.UpdateLineModes();
  DisplayLinesIfInAGoom(soundData);

  UpdateBuffers();

#ifdef SHOW_STATE_TEXT_ON_SCREEN
  DisplayStateText();
#endif
  DisplayTitle(songTitle, message, fps);
}

inline void GoomControl::GoomControlImpl::ProcessAudio(const AudioSamples& soundData)
{
  /* ! etude du signal ... */
  m_goomInfo.ProcessSoundSample(soundData);
}

inline void GoomControl::GoomControlImpl::ApplyCurrentStateToSingleBuffer()
{
  m_visualFx.ApplyCurrentStateToSingleBuffer();
}

inline void GoomControl::GoomControlImpl::ApplyCurrentStateToMultipleBuffers()
{
  m_visualFx.ApplyCurrentStateToMultipleBuffers();
}

inline void GoomControl::GoomControlImpl::ResetDrawBuffSettings(const FXBuffSettings& settings)
{
  m_multiBufferDraw.SetBuffIntensity(settings.buffIntensity);
  m_multiBufferDraw.SetAllowOverexposed(settings.allowOverexposed);
}

inline void GoomControl::GoomControlImpl::UpdateBuffers()
{
  // affichage et swappage des buffers...
  m_imageBuffers.GetP1().CopyTo(m_imageBuffers.GetOutputBuff());

  RotateDrawBuffers();
}

inline void GoomControl::GoomControlImpl::RotateDrawBuffers()
{
  m_imageBuffers.RotateBuffers();
  m_multiBufferDraw.SetBuffers(GetCurrentBuffers());
}

inline auto GoomControl::GoomControlImpl::GetCurrentBuffers() const -> std::vector<PixelBuffer*>
{
  return {&m_imageBuffers.GetP1(), &m_imageBuffers.GetP2()};
}

inline void GoomControl::GoomControlImpl::ApplyZoom()
{
  UpdateFilterSettings();

  m_visualFx.ApplyZoom(m_imageBuffers.GetP1(), m_imageBuffers.GetP2());
}

inline void GoomControl::GoomControlImpl::UpdateFilterSettings()
{
  m_visualFx.UpdateFilterSettings(
      m_filterSettingsService.GetFilterSettings(),
      m_filterSettingsService.HaveEffectsSettingsChangedSinceLastUpdate());
  m_filterSettingsService.NotifyUpdatedFilterEffectsSettings();
}

inline void GoomControl::GoomControlImpl::DisplayLinesIfInAGoom(const AudioSamples& soundData)
{
  if (m_musicSettingsReactor.CanDisplayLines())
  {
    DisplayLines(soundData);
  }
}

void GoomControl::GoomControlImpl::DisplayLines(const AudioSamples& soundData)
{
  if (!m_visualFx.IsCurrentlyDrawable(GoomDrawable::LINES))
  {
    return;
  }

  m_visualFx.DisplayGoomLines(soundData);

  m_musicSettingsReactor.ChangeGoomLines();
}

void GoomControl::GoomControlImpl::DisplayTitle(const std::string& songTitle,
                                                const std::string& message,
                                                const float fps)
{
  std::string msg{message};

  if (fps > 0.0)
  {
    const std::string text = std20::format("{.0f} fps", fps);
    msg.insert(0, text + "\n");
  }

  UpdateMessages(msg);

  if (!songTitle.empty())
  {
    m_currentSongTitle = songTitle;

    const auto xPos = static_cast<int>(0.085F * static_cast<float>(GetScreenWidth()));
    const auto yPos = static_cast<int>(0.300F * static_cast<float>(GetScreenHeight()));
    m_goomTitleDisplayer.SetInitialPosition(xPos, yPos);
  }

  if ((!m_currentSongTitle.empty()) && (!m_goomTitleDisplayer.IsFinished()))
  {
    if (m_goomTitleDisplayer.IsFinalPhase())
    {
      m_goomTextOutput.SetBuffers({&m_imageBuffers.GetP1()});
    }
    else
    {
      m_goomTextOutput.SetBuffers({&m_imageBuffers.GetOutputBuff()});
    }
    m_goomTitleDisplayer.Draw(m_currentSongTitle);
  }
}

/*
 * Met a jour l'affichage du message defilant
 */
void GoomControl::GoomControlImpl::UpdateMessages(const std::string& messages)
{
  if (messages.empty())
  {
    return;
  }

  m_goomTextOutput.SetBuffers({&m_imageBuffers.GetOutputBuff()});
  m_messageDisplayer.UpdateMessages(messages);
}

#ifdef SHOW_STATE_TEXT_ON_SCREEN
void GoomControl::GoomControlImpl::DisplayStateText()
{
  std::string message = "";

  const FILTERS::ZoomFilterEffectsSettings& filterEffectsSettings =
      m_filterSettingsService.GetFilterSettings().filterEffectsSettings;
  const FILTERS::ZoomFilterBufferSettings& filterBufferSettings =
      m_filterSettingsService.GetFilterSettings().filterBufferSettings;

  message += std20::format("State: {}\n", m_visualFx.GetCurrentStateName());
  message += std20::format("Filter Mode: {}\n", m_filterSettingsService.GetCurrentFilterMode());
  message +=
      std20::format("Previous Filter Mode: {}\n", m_filterSettingsService.GetPreviousFilterMode());

  message += std20::format("tranLerpIncrement: {}\n", filterBufferSettings.tranLerpIncrement);
  message +=
      std20::format("tranLerpToMaxSwitchMult: {}\n", filterBufferSettings.tranLerpToMaxSwitchMult);

  message += GetNameValuesString(m_visualFx.GetZoomFilterFxNameValueParams()) + "\n";

  message += std20::format("middleX: {}\n", filterEffectsSettings.zoomMidPoint.x);
  message += std20::format("middleY: {}\n", filterEffectsSettings.zoomMidPoint.y);

  message += GetNameValuesString(m_musicSettingsReactor.GetNameValueParams());

  UpdateMessages(message);
}
#endif

} // namespace GOOM
