/**
* file: goom_control.c (onetime goom_core.c)
 * author: Jean-Christophe Hoelt (which is not so proud of it)
 *
 * Contains the core of goom's work.
 *
 * (c)2000-2003, by iOS-software.
 *
 *  - converted to C++17 2021-02-01 (glk)
 *
 */

#include "goom_control.h"

//#undef NO_LOGGING

#include "control/goom_all_visual_fx.h"
#include "control/goom_events.h"
#include "control/goom_image_buffers.h"
#include "control/goom_message_displayer.h"
#include "control/goom_music_settings_reactor.h"
#include "control/goom_random_state_handler.h"
#include "control/goom_state_strings.h"
#include "control/goom_title_displayer.h"
#include "draw/goom_draw_to_buffer.h"
#include "goom/compiler_versions.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "goom_version.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand.h"
#include "utils/parallel_utils.h"
#include "utils/strutils.h"
#include "visual_fx/filters/filter_settings_service.h"
#include "visual_fx/filters/speed_coefficients_effect_factory.h"
#include "visual_fx/fx_helper.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace GOOM
{

using CONTROL::GetGoomStateStrings;
using CONTROL::GoomAllVisualFx;
using CONTROL::GoomEvents;
using CONTROL::GoomImageBuffers;
using CONTROL::GoomMessageDisplayer;
using CONTROL::GoomMusicSettingsReactor;
using CONTROL::GoomRandomStateHandler;
using CONTROL::GoomTitleDisplayer;
using DRAW::GoomDrawToBuffer;
using UTILS::Logging;
using UTILS::Parallel;
using UTILS::StringSplit;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::GoomRand;
using VISUAL_FX::FxHelper;
using VISUAL_FX::FILTERS::CreateSpeedCoefficientsEffect;
using VISUAL_FX::FILTERS::FilterSettingsService;

class GoomControl::GoomControlImpl
{
public:
  GoomControlImpl(uint32_t screenWidth, uint32_t screenHeight, std::string resourcesDirectory);

  [[nodiscard]] auto GetScreenWidth() const -> uint32_t;
  [[nodiscard]] auto GetScreenHeight() const -> uint32_t;

  void SetShowTitle(ShowTitleType value);

  void Start();
  void Finish();

  void SetScreenBuffer(const std::shared_ptr<PixelBuffer>& buffer);
  void ShowGoomState(bool value);

  void Update(const AudioSamples& soundData,
              float fps,
              const std::string& songTitle,
              const std::string& message);

  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

private:
  Parallel m_parallel{-1}; // max cores - 1
  WritablePluginInfo m_goomInfo;
  const GoomRand m_goomRand{};
  GoomDrawToBuffer m_multiBufferDraw;
  GoomImageBuffers m_imageBuffers;
  const std::string m_resourcesDirectory;
  FilterSettingsService m_filterSettingsService;
  const SmallImageBitmaps m_smallBitmaps;
  GoomAllVisualFx m_visualFx;
  GoomEvents m_goomEvents;
  GoomRandomStateHandler m_stateHandler;
  ShowTitleType m_showTitle = ShowTitleType::AT_START;
  GoomMusicSettingsReactor m_musicSettingsReactor;

  void ProcessAudio(const AudioSamples& soundData);

  void UseMusicToChangeSettings();

  void DrawAndZoom(const AudioSamples& soundData);

  void ApplyStateToSingleBufferPreZoom();
  void ApplyStateToMultipleBuffersPostZoom();
  void DisplayLinesIfInAGoom(const AudioSamples& soundData);

  void NewCycle();
  void ApplyZoomEffects();
  void UpdateFilterSettings();
  void UpdateBuffers();
  void RotateBuffers();

  [[nodiscard]] auto GetCurrentBuffers() const -> std::vector<PixelBuffer*>;
  void ResetDrawBuffSettings(const FXBuffSettings& settings);

  std::string m_currentSongTitle{};
  GoomDrawToBuffer m_goomTextOutput;
  GoomTitleDisplayer m_goomTitleDisplayer;
  GoomMessageDisplayer m_messageDisplayer;
  bool m_showGoomState = false;
  void DisplayGoomState();
  [[nodiscard]] auto GetFontDirectory() const -> std::string;
  void InitTitleDisplay(const std::string_view& songTitle);
  void DisplayTitle(const std::string& songTitle, const std::string& message, float fps);
  void DisplayCurrentTitle();
  void UpdateMessages(const std::string& messages);
  [[nodiscard]] auto GetMessagesFontFile() const -> std::string;
};

auto GoomControl::GetCompilerVersion() -> std::string
{
  return GOOM::GetCompilerVersion();
}

auto GoomControl::GetGoomVersionInfo() -> std::string
{
  return GetFullVersionStr();
}

auto GoomControl::GetRandSeed() -> uint64_t
{
  return UTILS::MATH::RAND::GetRandSeed();
}

void GoomControl::SetRandSeed(const uint64_t seed)
{
  LogDebug("Set goom seed = {}.", seed);
  UTILS::MATH::RAND::SetRandSeed(seed);
}

GoomControl::GoomControl(const uint32_t width,
                         const uint32_t height,
                         const std::string& resourcesDirectory)
  : m_controller{spimpl::make_unique_impl<GoomControlImpl>(width, height, resourcesDirectory)}
{
}

void GoomControl::SetShowTitle(const ShowTitleType value)
{
  m_controller->SetShowTitle(value);
}

void GoomControl::ShowGoomState(const bool value)
{
  m_controller->ShowGoomState(value);
}

void GoomControl::Start()
{
  m_controller->Start();
}

void GoomControl::Finish()
{
  m_controller->Finish();
}

void GoomControl::SetScreenBuffer(const std::shared_ptr<PixelBuffer>& buffer)
{
  m_controller->SetScreenBuffer(buffer);
}

auto GoomControl::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_controller->GetLastShaderEffects();
}

void GoomControl::Update(const AudioSamples& audioSamples,
                         const float fps,
                         const std::string& songTitle,
                         const std::string& message)
{
  m_controller->Update(audioSamples, fps, songTitle, message);
}

GoomControl::GoomControlImpl::GoomControlImpl(const uint32_t screenWidth,
                                              const uint32_t screenHeight,
                                              std::string resourcesDirectory)
  : m_goomInfo{screenWidth, screenHeight},
    m_multiBufferDraw{screenWidth, screenHeight},
    m_imageBuffers{screenWidth, screenHeight},
    m_resourcesDirectory{std::move(resourcesDirectory)},
    m_filterSettingsService{m_parallel,
                            m_goomInfo,
                            m_goomRand,
                            m_resourcesDirectory,
                            CreateSpeedCoefficientsEffect},
    m_smallBitmaps{m_resourcesDirectory},
    m_visualFx{m_parallel,
               FxHelper{m_multiBufferDraw, m_goomInfo, m_goomRand},
               m_smallBitmaps,
               m_resourcesDirectory,
               m_stateHandler,
               m_filterSettingsService.GetFilterBuffersService(),
               m_filterSettingsService.GetFilterColorsService()},
    m_goomEvents{m_goomRand},
    m_stateHandler{m_goomRand},
    m_musicSettingsReactor{m_goomInfo, m_goomRand, m_visualFx, m_goomEvents,
                           m_filterSettingsService},
    m_goomTextOutput{screenWidth, screenHeight},
    m_goomTitleDisplayer{m_goomTextOutput, m_goomRand, GetFontDirectory()},
    m_messageDisplayer{m_goomTextOutput, GetMessagesFontFile()}
{
  RotateBuffers();
}

inline auto GoomControl::GoomControlImpl::GetScreenWidth() const -> uint32_t
{
  return m_goomInfo.GetScreenInfo().width;
}

inline auto GoomControl::GoomControlImpl::GetScreenHeight() const -> uint32_t
{
  return m_goomInfo.GetScreenInfo().height;
}

inline void GoomControl::GoomControlImpl::SetShowTitle(const ShowTitleType value)
{
  m_showTitle = value;
}

inline void GoomControl::GoomControlImpl::SetScreenBuffer(
    const std::shared_ptr<PixelBuffer>& buffer)
{
  m_imageBuffers.SetOutputBuff(buffer);
}

inline void GoomControl::GoomControlImpl::ShowGoomState(const bool value)
{
  m_showGoomState = value;
}

inline auto GoomControl::GoomControlImpl::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_visualFx.GetLastShaderEffects();
}

void GoomControl::GoomControlImpl::Start()
{
  m_filterSettingsService.Start();
  UpdateFilterSettings();

  m_visualFx.SetResetDrawBuffSettingsFunc([this](const FXBuffSettings& settings)
                                          { ResetDrawBuffSettings(settings); });
  m_visualFx.ChangeAllFxColorMaps();
  m_visualFx.Start();

  m_musicSettingsReactor.Start();

  m_visualFx.StartExposureControl();
}

void GoomControl::GoomControlImpl::Finish()
{
  m_visualFx.Finish();
}

inline auto GoomControl::GoomControlImpl::GetFontDirectory() const -> std::string
{
  return m_resourcesDirectory + PATH_SEP + FONTS_DIR;
}

inline auto GoomControl::GoomControlImpl::GetMessagesFontFile() const -> std::string
{
  return GetFontDirectory() + PATH_SEP + "verdana.ttf";
}

inline auto GoomControl::GoomControlImpl::GetCurrentBuffers() const -> std::vector<PixelBuffer*>
{
  return {&m_imageBuffers.GetP1(), &m_imageBuffers.GetP2()};
}

void GoomControl::GoomControlImpl::Update(const AudioSamples& soundData,
                                          const float fps,
                                          const std::string& songTitle,
                                          const std::string& message)
{
  NewCycle();

  // Elargissement de l'intervalle d'évolution des points!
  // Calcul du deplacement des petits points ...
  // Widening of the interval of evolution of the points!
  // Calculation of the displacement of small points ...

  ProcessAudio(soundData);

  UseMusicToChangeSettings();

  DrawAndZoom(soundData);

  UpdateBuffers();

  DisplayGoomState();
  DisplayTitle(songTitle, message, fps);
}

inline void GoomControl::GoomControlImpl::NewCycle()
{
  m_musicSettingsReactor.NewCycle();
  m_filterSettingsService.NewCycle();
}

inline void GoomControl::GoomControlImpl::UseMusicToChangeSettings()
{
  m_musicSettingsReactor.ChangeFilterModeIfMusicChanges();
  m_musicSettingsReactor.BigUpdateIfNotLocked();
  m_musicSettingsReactor.BigBreakIfMusicIsCalm();

  m_musicSettingsReactor.RegularlyLowerTheSpeed();

  m_musicSettingsReactor.ChangeZoomEffects();
}

inline void GoomControl::GoomControlImpl::DrawAndZoom(const AudioSamples& soundData)
{
  ApplyStateToSingleBufferPreZoom();

  ApplyZoomEffects();

  ApplyStateToMultipleBuffersPostZoom();

  DisplayLinesIfInAGoom(soundData);
}

inline void GoomControl::GoomControlImpl::ProcessAudio(const AudioSamples& soundData)
{
  /* ! etude du signal ... */
  m_goomInfo.ProcessSoundSample(soundData);
}

inline void GoomControl::GoomControlImpl::ApplyStateToSingleBufferPreZoom()
{
  m_visualFx.ApplyCurrentStateToSingleBuffer();
}

inline void GoomControl::GoomControlImpl::ApplyStateToMultipleBuffersPostZoom()
{
  m_visualFx.ApplyCurrentStateToMultipleBuffers();
}

inline void GoomControl::GoomControlImpl::ApplyZoomEffects()
{
  UpdateFilterSettings();

  m_visualFx.ApplyZoom(m_imageBuffers.GetP1(), m_imageBuffers.GetP2());
}

inline void GoomControl::GoomControlImpl::UpdateFilterSettings()
{
  m_visualFx.UpdateFilterSettings(
      const_cast<const FilterSettingsService&>(m_filterSettingsService).GetFilterSettings(),
      m_filterSettingsService.HaveEffectsSettingsChangedSinceLastUpdate());
  m_filterSettingsService.NotifyUpdatedFilterEffectsSettings();
}

inline void GoomControl::GoomControlImpl::ResetDrawBuffSettings(const FXBuffSettings& settings)
{
  m_multiBufferDraw.SetBuffIntensity(settings.buffIntensity);
}

inline void GoomControl::GoomControlImpl::UpdateBuffers()
{
  // affichage et swappage des buffers...
  m_imageBuffers.GetP1().CopyTo(m_imageBuffers.GetOutputBuff());

  RotateBuffers();
}

inline void GoomControl::GoomControlImpl::RotateBuffers()
{
  m_imageBuffers.RotateBuffers();
  m_multiBufferDraw.SetBuffers(GetCurrentBuffers());
}

inline void GoomControl::GoomControlImpl::DisplayLinesIfInAGoom(const AudioSamples& soundData)
{
  // Gestion du Scope - Scope management
  m_musicSettingsReactor.UpdateLineModes();

  if (!m_musicSettingsReactor.CanDisplayLines())
  {
    return;
  }
  if (!m_visualFx.CanDisplayLines())
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

  if (fps > 0.0F)
  {
    const std::string text = std20::format("{.0f} fps", fps);
    msg.insert(0, text + "\n");
  }

  UpdateMessages(msg);

  if (m_showTitle == ShowTitleType::NEVER)
  {
    return;
  }

  if (!songTitle.empty())
  {
    InitTitleDisplay(songTitle);
  }
  if (!m_currentSongTitle.empty())
  {
    DisplayCurrentTitle();
  }
}

void GoomControl::GoomControlImpl::InitTitleDisplay(const std::string_view& songTitle)
{
  m_currentSongTitle = songTitle;

  const float xPosFraction = m_showTitle == ShowTitleType::ALWAYS ? 0.050F : 0.085F;
  const float yPosFraction = m_showTitle == ShowTitleType::ALWAYS ? 0.130F : 0.300F;
  const auto xPos = static_cast<int>(xPosFraction * static_cast<float>(GetScreenWidth()));
  const auto yPos = static_cast<int>(yPosFraction * static_cast<float>(GetScreenHeight()));

  m_goomTitleDisplayer.SetInitialPosition(xPos, yPos);
}

inline void GoomControl::GoomControlImpl::DisplayCurrentTitle()
{
  if (m_showTitle == ShowTitleType::ALWAYS)
  {
    m_goomTextOutput.SetBuffers({&m_imageBuffers.GetOutputBuff()});
    m_goomTitleDisplayer.DrawStaticText(m_currentSongTitle);
    return;
  }

  if (m_goomTitleDisplayer.IsFinished())
  {
    return;
  }

  if (m_goomTitleDisplayer.IsFinalPhase())
  {
    constexpr float FINAL_TITLE_BUFF_INTENSITY = 0.2F;
    m_goomTextOutput.SetBuffIntensity(FINAL_TITLE_BUFF_INTENSITY);
    m_goomTextOutput.SetBuffers({&m_imageBuffers.GetP1()});
  }
  else
  {
    m_goomTextOutput.SetBuffers({&m_imageBuffers.GetOutputBuff()});
  }
  m_goomTitleDisplayer.DrawMovingText(m_currentSongTitle);
}

/*
 * Met a jour l'affichage du message defilant
 */
inline void GoomControl::GoomControlImpl::UpdateMessages(const std::string& messages)
{
  if (messages.empty())
  {
    return;
  }

  m_goomTextOutput.SetBuffers({&m_imageBuffers.GetOutputBuff()});

  m_messageDisplayer.UpdateMessages(StringSplit(messages, "\n"));
}

inline void GoomControl::GoomControlImpl::DisplayGoomState()
{
  if (!m_showGoomState)
  {
    return;
  }

  const std::string message = GetGoomStateStrings(m_visualFx, m_musicSettingsReactor,
                                                  m_filterSettingsService, GetLastShaderEffects());

  UpdateMessages(message);
}

} // namespace GOOM
