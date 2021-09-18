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
#include "control/goom_lock.h"
#include "control/goom_message_displayer.h"
#include "control/goom_title_displayer.h"
#include "draw/goom_draw_to_buffer.h"
#include "filters/filter_buffers_service.h"
#include "filters/filter_colors_service.h"
#include "filters/filter_settings_service.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "goomutils/goomrand.h"
#include "goomutils/logging_control.h"
#undef NO_LOGGING
#include "goomutils/graphics/small_image_bitmaps.h"
#include "goomutils/logging.h"
#ifdef SHOW_STATE_TEXT_ON_SCREEN
#include "goomutils/name_value_pairs.h"
#endif
#include "goomutils/parallel_utils.h"
#include "goomutils/spimpl.h"
#include "goomutils/t_values.h"
#include "goomutils/timer.h"

#include <cmath>
#include <cstdint>
#if __cplusplus > 201402L
#include <filesystem>
#endif
//#undef NDEBUG
#include <cassert>
#include <memory>
#include <stdexcept>
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
using CONTROL::GoomLock;
using CONTROL::GoomMessageDisplayer;
using CONTROL::GoomTitleDisplayer;
using DRAW::GoomDrawToBuffer;
using FILTERS::FilterBuffersService;
using FILTERS::FilterColorsService;
using FILTERS::FilterSettingsService;
using FILTERS::Vitesse;
#ifdef SHOW_STATE_TEXT_ON_SCREEN
using UTILS::GetNameValuesString;
#endif
using UTILS::Logging;
using UTILS::Parallel;
using UTILS::ProbabilityOfMInN;
using UTILS::SmallImageBitmaps;
using UTILS::Timer;
using UTILS::TValue;

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
  const std::shared_ptr<WritablePluginInfo> m_goomInfo;
  GoomDrawToBuffer m_multiBufferDraw;
  GoomImageBuffers m_imageBuffers;
  const std::string m_resourcesDirectory;
  FilterSettingsService m_filterSettingsService;
  const SmallImageBitmaps m_smallBitmaps;
  GoomAllVisualFx m_visualFx;

  using GoomEvent = GoomEvents::GoomEvent;
  GoomEvents m_goomEvent{};

  uint32_t m_updateNum = 0;

  static constexpr uint32_t NORMAL_UPDATE_LOCK_TIME = 50;
  static constexpr uint32_t REVERSE_SPEED_AND_STOP_SPEED_LOCK_TIME = 75;
  static constexpr uint32_t REVERSE_SPEED_LOCK_TIME = 100;
  static constexpr uint32_t MEGA_LENT_LOCK_TIME_INCREASE = 50;
  static constexpr uint32_t CHANGE_VITESSE_LOCK_TIME_INCREASE = 50;
  static constexpr uint32_t CHANGE_SWITCH_VALUES_LOCK_TIME = 150;
  GoomLock m_lock{}; // pour empecher de nouveaux changements

  void ProcessAudio(const AudioSamples& soundData) const;

  // Changement d'effet de zoom !
  static constexpr int32_t MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE = 200;
  int32_t m_updatesSinceLastZoomEffectsChange = 0; // nombre de Cycle Depuis Dernier Changement
  int32_t m_previousZoomSpeed = Vitesse::DEFAULT_VITESSE + 1;
  void ChangeZoomEffects();
  void UpdateFilterSettings();
  void ApplyZoom();
  void UpdateBuffers();
  void RotateDrawBuffers();
  [[nodiscard]] auto GetCurrentBuffers() const -> std::vector<PixelBuffer*>;
  void ResetDrawBuffSettings(const FXBuffSettings& settings);

  static constexpr uint32_t MAX_NUM_STATE_SELECTIONS_BLOCKED = 3;
  uint32_t m_stateSelectionBlocker = 0;
  uint32_t m_timeInState = 0;
  void ChangeState();
  void DoChangeState();

  void ApplyCurrentStateToSingleBuffer();
  void ApplyCurrentStateToMultipleBuffers();

  void BigNormalUpdate();
  void MegaLentUpdate();

  void ChangeAllowOverexposed();
  void ChangeBlockyWavy();
  void ChangeNoise();
  void ChangeRotation();
  void ChangeSwitchValues();
  void ChangeSpeedReverse();
  void ChangeVitesse();
  void ChangeStopSpeeds();

  static constexpr uint32_t NUM_BLOCKY_WAVY_UPDATES = 100;
  Timer m_blockyWavyTimer{NUM_BLOCKY_WAVY_UPDATES};
  static constexpr uint32_t NUM_NOISE_UPDATES = 100;
  Timer m_noiseTimer{NUM_NOISE_UPDATES};
  static constexpr uint32_t NUM_ALLOW_OVEREXPOSED_UPDATES = 100;
  Timer m_allowOverexposedTimer{NUM_ALLOW_OVEREXPOSED_UPDATES};
  void UpdateTimers();

  // on verifie qu'il ne se pas un truc interressant avec le son.
  [[nodiscard]] auto ChangeFilterModeEventHappens() const -> bool;
  void ChangeFilterModeIfMusicChanges();
  void ChangeFilterMode();
  void ChangeMilieu();

  // baisser regulierement la vitesse
  void RegularlyLowerTheSpeed();

  // tout ceci ne sera fait qu'en cas de non-blocage
  void BigUpdateIfNotLocked();
  void BigUpdate();

  // gros frein si la musique est calme
  void BigBreakIfMusicIsCalm();
  void BigBreak();

  // duree de la transition entre afficher les lignes ou pas
  int32_t m_stopLines = 0;
  int32_t m_drawLinesDuration = LinesFx::MIN_LINE_DURATION;
  int32_t m_lineMode = LinesFx::MIN_LINE_DURATION; // l'effet lineaire a dessiner
  // arret demande
  void StopLinesIfRequested();
  // arret aleatore.. changement de mode de ligne..
  void StopRandomLineChangeMode();
  void DisplayLinesIfInAGoom(const AudioSamples& soundData);
  void DisplayLines(const AudioSamples& soundData);
  using GoomLineSettings = GoomAllVisualFx::GoomLineSettings;
  [[nodiscard]] auto GetGoomLineResetSettings(int farVal) const -> GoomLineSettings;
  [[nodiscard]] auto GetGoomLineStopSettings() const -> GoomLineSettings;
  void ResetGoomLines();
  void StopGoomLines();
  void ResetLineModes();

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
  : m_goomInfo{std::make_shared<WritablePluginInfo>(screenWidth, screenHeight)},
    m_multiBufferDraw{screenWidth, screenHeight},
    m_imageBuffers{screenWidth, screenHeight},
    m_resourcesDirectory{std::move(resourcesDirectory)},
    m_filterSettingsService{m_parallel, m_goomInfo, m_resourcesDirectory},
    m_smallBitmaps{m_resourcesDirectory},
    m_visualFx{m_parallel,
               m_multiBufferDraw,
               std::const_pointer_cast<const PluginInfo>(
                   std::dynamic_pointer_cast<PluginInfo>(m_goomInfo)),
               m_smallBitmaps,
               m_filterSettingsService.GetFilterBuffersService(),
               FilterSettingsService::GetFilterColorsService()},
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
  return m_goomInfo->GetScreenInfo().width;
}

inline auto GoomControl::GoomControlImpl::GetScreenHeight() const -> uint32_t
{
  return m_goomInfo->GetScreenInfo().height;
}

inline auto GoomControl::GoomControlImpl::ChangeFilterModeEventHappens() const -> bool
{
  return m_goomEvent.Happens(GoomEvent::CHANGE_FILTER_MODE);
}

void GoomControl::GoomControlImpl::Start()
{
  m_updateNum = 0;
  m_timeInState = 0;

  m_filterSettingsService.Start();
  UpdateFilterSettings();

  m_visualFx.SetResetDrawBuffSettingsFunc(
      [this](const FXBuffSettings& settings) { ResetDrawBuffSettings(settings); });
  m_visualFx.ChangeColorMaps();
  m_visualFx.Start();

  DoChangeState();
}

void GoomControl::GoomControlImpl::Finish()
{
  m_visualFx.Finish();

  m_updateNum = 0;
}

void GoomControl::GoomControlImpl::Update(const AudioSamples& soundData,
                                          const float fps,
                                          const std::string& songTitle,
                                          const std::string& message)
{
  ++m_updateNum;

  UpdateTimers();

  // Elargissement de l'intervalle d'évolution des points!
  // Calcul du deplacement des petits points ...
  // Widening of the interval of evolution of the points!
  // Calculation of the displacement of small points ...

  ProcessAudio(soundData);

  m_lock.Update();

  ChangeFilterModeIfMusicChanges();
  BigUpdateIfNotLocked();
  BigBreakIfMusicIsCalm();

  RegularlyLowerTheSpeed();

  ChangeZoomEffects();

  ApplyCurrentStateToSingleBuffer();
  ApplyZoom();
  ApplyCurrentStateToMultipleBuffers();

  // Gestion du Scope - Scope management
  StopLinesIfRequested();
  StopRandomLineChangeMode();
  DisplayLinesIfInAGoom(soundData);

  UpdateBuffers();

#ifdef SHOW_STATE_TEXT_ON_SCREEN
  DisplayStateText();
#endif
  DisplayTitle(songTitle, message, fps);
}

inline void GoomControl::GoomControlImpl::UpdateTimers()
{
  ++m_timeInState;

  m_blockyWavyTimer.Increment();
  m_noiseTimer.Increment();
  m_allowOverexposedTimer.Increment();
}

inline void GoomControl::GoomControlImpl::ProcessAudio(const AudioSamples& soundData) const
{
  /* ! etude du signal ... */
  m_goomInfo->ProcessSoundSample(soundData);
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

inline void GoomControl::GoomControlImpl::ChangeFilterModeIfMusicChanges()
{
  if (((0 == m_goomInfo->GetSoundInfo().GetTimeSinceLastGoom()) ||
       (m_updatesSinceLastZoomEffectsChange > MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE)) &&
      ChangeFilterModeEventHappens())
  {
    ChangeFilterMode();
  }
}

inline void GoomControl::GoomControlImpl::ChangeFilterMode()
{
  m_filterSettingsService.SetRandomFilterSettings();

  m_visualFx.DoIfsRenew();
}

inline void GoomControl::GoomControlImpl::ChangeState()
{
  if (m_stateSelectionBlocker)
  {
    --m_stateSelectionBlocker;
  }
  else if (m_goomEvent.Happens(GoomEvent::CHANGE_STATE))
  {
    m_stateSelectionBlocker = MAX_NUM_STATE_SELECTIONS_BLOCKED;
    DoChangeState();
  }
}

void GoomControl::GoomControlImpl::DoChangeState()
{
  const auto oldGDrawables = m_visualFx.GetCurrentGoomDrawables();

  m_visualFx.SetNextState();
  m_visualFx.ChangeColorMaps();
  m_visualFx.PostStateUpdate(oldGDrawables);

  m_timeInState = 0;

  if (m_goomEvent.Happens(GoomEvent::IFS_RENEW))
  {
    m_visualFx.DoIfsRenew();
  }

  ResetLineModes();
}

inline void GoomControl::GoomControlImpl::BigBreakIfMusicIsCalm()
{
  constexpr float CALM_SPEED = 0.01F;
  constexpr uint32_t CALM_CYCLES = 16;

  if ((m_goomInfo->GetSoundInfo().GetSpeed() < CALM_SPEED) &&
      (m_filterSettingsService.GetROVitesse().GetVitesse() < (Vitesse::STOP_SPEED - 4)) &&
      (0 == (m_updateNum % CALM_CYCLES)))
  {
    BigBreak();
  }
}

inline void GoomControl::GoomControlImpl::BigBreak()
{
  m_filterSettingsService.GetRWVitesse().GoSlowerBy(3);

  m_visualFx.ChangeColorMaps();
}

inline void GoomControl::GoomControlImpl::BigUpdateIfNotLocked()
{
  if (!m_lock.IsLocked())
  {
    BigUpdate();
  }
}

inline void GoomControl::GoomControlImpl::BigUpdate()
{
  // Reperage de goom (acceleration forte de l'acceleration du volume).
  // Coup de boost de la vitesse si besoin.
  // Goom tracking (strong acceleration of volume acceleration).
  // Speed boost if needed.
  if (0 == m_goomInfo->GetSoundInfo().GetTimeSinceLastGoom())
  {
    BigNormalUpdate();
  }

  // mode mega-lent
  if (m_goomEvent.Happens(GoomEvent::CHANGE_TO_MEGA_LENT_MODE))
  {
    MegaLentUpdate();
  }
}

void GoomControl::GoomControlImpl::BigNormalUpdate()
{
  m_lock.SetLockTime(NORMAL_UPDATE_LOCK_TIME);

  ChangeState();
  ChangeSpeedReverse();
  ChangeStopSpeeds();
  ChangeRotation();
  ChangeMilieu();
  ChangeNoise();
  ChangeBlockyWavy();
  ChangeAllowOverexposed();
  ChangeVitesse();
  ChangeSwitchValues();

  m_visualFx.SetSingleBufferDots(ProbabilityOfMInN(1, 20));
}

inline void GoomControl::GoomControlImpl::MegaLentUpdate()
{
  m_lock.IncreaseLockTime(MEGA_LENT_LOCK_TIME_INCREASE);

  m_filterSettingsService.GetRWVitesse().SetVitesse(Vitesse::STOP_SPEED - 1);
  m_filterSettingsService.SetDefaultTranLerpIncrement();
  m_filterSettingsService.SetTranLerpToMaxSwitchMult(1.0F);
}

inline void GoomControl::GoomControlImpl::ChangeMilieu()
{
  m_filterSettingsService.ChangeMilieu();
}

void GoomControl::GoomControlImpl::ChangeVitesse()
{
  const auto goFasterVal = static_cast<int32_t>(
      std::lround(3.5F * std::log10(1.0F + (500.0F * m_goomInfo->GetSoundInfo().GetSpeed()))));
  const int32_t newVitesse = Vitesse::STOP_SPEED - goFasterVal;
  const int32_t oldVitesse = m_filterSettingsService.GetROVitesse().GetVitesse();

  if (newVitesse >= oldVitesse)
  {
    return;
  }

  constexpr uint32_t VITESSE_CYCLES = 3;
  constexpr int32_t FAST_SPEED = Vitesse::STOP_SPEED - 6;
  constexpr int32_t FASTER_SPEED = Vitesse::STOP_SPEED - 7;
  constexpr int32_t SLOW_SPEED = Vitesse::STOP_SPEED - 1;
  constexpr float OLD_TO_NEW_MIX = 0.4F;

  // on accelere
  if (((newVitesse < FASTER_SPEED) && (oldVitesse < FAST_SPEED) &&
       (0 == (m_updateNum % VITESSE_CYCLES))) ||
      m_goomEvent.Happens(GoomEvent::FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE))
  {
    m_filterSettingsService.GetRWVitesse().SetVitesse(SLOW_SPEED);
    m_filterSettingsService.GetRWVitesse().ToggleReverseVitesse();
  }
  else
  {
    m_filterSettingsService.GetRWVitesse().SetVitesse(static_cast<int32_t>(std::lround(stdnew::lerp(
        static_cast<float>(oldVitesse), static_cast<float>(newVitesse), OLD_TO_NEW_MIX))));
  }

  m_lock.IncreaseLockTime(CHANGE_VITESSE_LOCK_TIME_INCREASE);
}

void GoomControl::GoomControlImpl::ChangeSpeedReverse()
{
  // Retablir le zoom avant.
  // Restore zoom in.

  constexpr uint32_t REVERSE_VITESSE_CYCLES = 13;
  constexpr int32_t SLOW_SPEED = Vitesse::STOP_SPEED - 2;

  if ((m_filterSettingsService.GetROVitesse().GetReverseVitesse()) &&
      ((m_updateNum % REVERSE_VITESSE_CYCLES) != 0) &&
      m_goomEvent.Happens(GoomEvent::FILTER_REVERSE_OFF_AND_STOP_SPEED))
  {
    m_filterSettingsService.GetRWVitesse().SetReverseVitesse(false);
    m_filterSettingsService.GetRWVitesse().SetVitesse(SLOW_SPEED);
    m_lock.SetLockTime(REVERSE_SPEED_AND_STOP_SPEED_LOCK_TIME);
  }
  if (m_goomEvent.Happens(GoomEvent::FILTER_REVERSE_ON))
  {
    m_filterSettingsService.GetRWVitesse().SetReverseVitesse(true);
    m_lock.SetLockTime(REVERSE_SPEED_LOCK_TIME);
  }
}

inline void GoomControl::GoomControlImpl::ChangeStopSpeeds()
{
  if (m_goomEvent.Happens(GoomEvent::FILTER_VITESSE_STOP_SPEED_MINUS1))
  {
    constexpr int32_t SLOW_SPEED = Vitesse::STOP_SPEED - 1;
    m_filterSettingsService.GetRWVitesse().SetVitesse(SLOW_SPEED);
  }
  else if (m_goomEvent.Happens(GoomEvent::FILTER_VITESSE_STOP_SPEED))
  {
    m_filterSettingsService.GetRWVitesse().SetVitesse(Vitesse::STOP_SPEED);
  }
}

inline void GoomControl::GoomControlImpl::ChangeSwitchValues()
{
  if (m_lock.GetLockTime() > CHANGE_SWITCH_VALUES_LOCK_TIME)
  {
    m_filterSettingsService.SetDefaultTranLerpIncrement();
    m_filterSettingsService.SetTranLerpToMaxSwitchMult(1.0F);
  }
}

inline void GoomControl::GoomControlImpl::ChangeNoise()
{
  if (!m_noiseTimer.Finished())
  {
    return;
  }

  if (m_goomEvent.Happens(GoomEvent::TURN_OFF_NOISE))
  {
    m_filterSettingsService.SetNoise(false);
  }
  else
  {
    m_filterSettingsService.SetNoise(true);
    m_noiseTimer.ResetToZero();
  }
}

/* Changement d'effet de zoom !
 */
void GoomControl::GoomControlImpl::ChangeZoomEffects()
{
  ChangeBlockyWavy();
  ChangeAllowOverexposed();

  if (!m_filterSettingsService.HasFilterModeChangedSinceLastUpdate())
  {
    if (m_updatesSinceLastZoomEffectsChange > MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE)
    {
      m_updatesSinceLastZoomEffectsChange = 0;

      ChangeRotation();
      m_visualFx.DoIfsRenew();
    }
    else
    {
      ++m_updatesSinceLastZoomEffectsChange;
    }
  }
  else
  {
    m_updatesSinceLastZoomEffectsChange = 0;
    m_filterSettingsService.SetDefaultTranLerpIncrement();

    int32_t diff = m_filterSettingsService.GetROVitesse().GetVitesse() - m_previousZoomSpeed;
    if (diff < 0)
    {
      diff = -diff;
    }

    if (diff > 2)
    {
      m_filterSettingsService.MultiplyTranLerpIncrement((diff + 2) / 2);
    }
    m_previousZoomSpeed = m_filterSettingsService.GetROVitesse().GetVitesse();
    m_filterSettingsService.SetTranLerpToMaxSwitchMult(1.0F);

    if ((0 == m_goomInfo->GetSoundInfo().GetTimeSinceLastGoom()) &&
        (m_goomInfo->GetSoundInfo().GetTotalGoomsInCurrentCycle() < 2))
    {
      m_filterSettingsService.SetTranLerpIncrement(0);
      m_filterSettingsService.SetTranLerpToMaxDefaultSwitchMult();

      ChangeRotation();
      m_visualFx.DoIfsRenew();
    }
  }
}

inline void GoomControl::GoomControlImpl::ChangeBlockyWavy()
{
  if (!m_blockyWavyTimer.Finished())
  {
    return;
  }

  const bool blockyWavy = m_goomEvent.Happens(GoomEvent::CHANGE_BLOCKY_WAVY_TO_ON);

  m_filterSettingsService.SetBlockyWavy(blockyWavy);

  m_blockyWavyTimer.ResetToZero();
}

inline void GoomControl::GoomControlImpl::ChangeAllowOverexposed()
{
  if (!m_allowOverexposedTimer.Finished())
  {
    return;
  }

  const bool allowOverexposed =
      m_goomEvent.Happens(GoomEvent::CHANGE_ZOOM_FILTER_ALLOW_OVEREXPOSED_TO_ON);

  m_visualFx.SetZoomFilterAllowOverexposed(allowOverexposed);

  m_allowOverexposedTimer.ResetToZero();
}

inline void GoomControl::GoomControlImpl::ChangeRotation()
{
  if (m_goomEvent.Happens(GoomEvent::FILTER_STOP_ROTATION))
  {
    m_filterSettingsService.SetRotateToZero();
  }
  else if (m_goomEvent.Happens(GoomEvent::FILTER_DECREASE_ROTATION))
  {
    constexpr float ROTATE_SLOWER_FACTOR = 0.9F;
    m_filterSettingsService.MultiplyRotate(ROTATE_SLOWER_FACTOR);
  }
  else if (m_goomEvent.Happens(GoomEvent::FILTER_INCREASE_ROTATION))
  {
    constexpr float ROTATE_FASTER_FACTOR = 1.1F;
    m_filterSettingsService.MultiplyRotate(ROTATE_FASTER_FACTOR);
  }
  else if (m_goomEvent.Happens(GoomEvent::FILTER_TOGGLE_ROTATION))
  {
    m_filterSettingsService.ToggleRotate();
  }
}

inline void GoomControl::GoomControlImpl::RegularlyLowerTheSpeed()
{
  constexpr uint32_t LOWER_SPEED_CYCLES = 73;
  constexpr int32_t FAST_SPEED = Vitesse::STOP_SPEED - 5;

  if ((0 == (m_updateNum % LOWER_SPEED_CYCLES)) &&
      (m_filterSettingsService.GetROVitesse().GetVitesse() < FAST_SPEED))
  {
    m_filterSettingsService.GetRWVitesse().GoSlowerBy(1);
  }
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

void GoomControl::GoomControlImpl::ResetLineModes()
{
  if (!m_visualFx.IsCurrentlyDrawable(GoomDrawable::SCOPE))
  {
    m_stopLines = 0xF000 & 5;
  }
  if (!m_visualFx.IsCurrentlyDrawable(GoomDrawable::FAR_SCOPE))
  {
    m_stopLines = 0;
    m_lineMode = m_drawLinesDuration;
  }
}

inline void GoomControl::GoomControlImpl::StopLinesIfRequested()
{
  if (((m_stopLines & 0xf000) != 0) || (!m_visualFx.IsCurrentlyDrawable(GoomDrawable::SCOPE)))
  {
    StopGoomLines();
  }
}

inline void GoomControl::GoomControlImpl::StopGoomLines()
{
  if (!m_visualFx.CanResetDestGoomLines())
  {
    return;
  }

  m_visualFx.ResetDestGoomLines(GetGoomLineStopSettings());

  m_stopLines &= 0x0fff;
}

inline void GoomControl::GoomControlImpl::ResetGoomLines()
{
  m_visualFx.ResetDestGoomLines(GetGoomLineResetSettings(m_stopLines));

  if (m_stopLines)
  {
    --m_stopLines;
  }
}

inline auto GoomControl::GoomControlImpl::GetGoomLineStopSettings() const -> GoomLineSettings
{
  GoomLineSettings lineSettings = GetGoomLineResetSettings(1);
  lineSettings.line1Color = GetBlackLineColor();
  lineSettings.line2Color = lineSettings.line1Color;
  return lineSettings;
}

auto GoomControl::GoomControlImpl::GetGoomLineResetSettings(const int farVal) const
    -> GoomLineSettings
{
  GoomLineSettings lineSettings;

  lineSettings.amplitude = 1.0F;
  lineSettings.mode = m_goomEvent.GetRandomLineTypeEvent();

  switch (lineSettings.mode)
  {
    case LinesFx::LineType::CIRCLE:
      if (farVal)
      {
        lineSettings.param1 = 0.47F;
        lineSettings.param2 = lineSettings.param1;
        lineSettings.amplitude = 0.8F;
        break;
      }
      if (m_goomEvent.Happens(GoomEvent::CHANGE_LINE_CIRCLE_AMPLITUDE))
      {
        lineSettings.param1 = 0.0F;
        lineSettings.param2 = 0.0F;
        lineSettings.amplitude = 3.0F;
      }
      else if (m_goomEvent.Happens(GoomEvent::CHANGE_LINE_CIRCLE_PARAMS))
      {
        lineSettings.param1 = 0.40F * static_cast<float>(GetScreenHeight());
        lineSettings.param2 = 0.22F * static_cast<float>(GetScreenHeight());
      }
      else
      {
        lineSettings.param1 = static_cast<float>(GetScreenHeight()) * 0.35F;
        lineSettings.param2 = lineSettings.param1;
      }
      break;
    case LinesFx::LineType::H_LINE:
      if (m_goomEvent.Happens(GoomEvent::CHANGE_H_LINE_PARAMS) || (farVal != 0))
      {
        lineSettings.param1 = static_cast<float>(GetScreenHeight()) / 7.0F;
        lineSettings.param2 = (6.0F * static_cast<float>(GetScreenHeight())) / 7.0F;
      }
      else
      {
        lineSettings.param1 = static_cast<float>(GetScreenHeight()) / 2.0F;
        lineSettings.param2 = lineSettings.param1;
        lineSettings.amplitude = 2.0F;
      }
      break;
    case LinesFx::LineType::V_LINE:
      if (m_goomEvent.Happens(GoomEvent::CHANGE_V_LINE_PARAMS) || (farVal != 0))
      {
        lineSettings.param1 = static_cast<float>(GetScreenWidth()) / 7.0F;
        lineSettings.param2 = (6.0F * static_cast<float>(GetScreenWidth())) / 7.0F;
      }
      else
      {
        lineSettings.param1 = static_cast<float>(GetScreenWidth()) / 2.0F;
        lineSettings.param2 = lineSettings.param1;
        lineSettings.amplitude = 1.5F;
      }
      break;
    default:
      throw std::logic_error("Unknown LineTypes enum.");
  }

  if ((farVal != 0) && m_goomEvent.Happens(GoomEvent::CHANGE_LINE_TO_BLACK))
  {
    lineSettings.line1Color = GetBlackLineColor();
    lineSettings.line2Color = lineSettings.line1Color;
  }
  else
  {
    lineSettings.line1Color = m_visualFx.GetGoomLine1RandomColor();
    lineSettings.line2Color = m_visualFx.GetGoomLine2RandomColor();
  }

  return lineSettings;
}

/* arret aleatore.. changement de mode de ligne..
  */
void GoomControl::GoomControlImpl::StopRandomLineChangeMode()
{
  constexpr uint32_t DEC_LINE_MODE_CYCLES = 80;
  constexpr uint32_t UPDATE_LINE_MODE_CYCLES = 120;

  if (m_lineMode != m_drawLinesDuration)
  {
    --m_lineMode;
    if (-1 == m_lineMode)
    {
      m_lineMode = 0;
    }
  }
  else if ((0 == (m_updateNum % DEC_LINE_MODE_CYCLES)) &&
           m_goomEvent.Happens(GoomEvent::REDUCE_LINE_MODE) && (m_lineMode != 0))
  {
    --m_lineMode;
  }

  if ((0 == (m_updateNum % UPDATE_LINE_MODE_CYCLES)) &&
      m_goomEvent.Happens(GoomEvent::UPDATE_LINE_MODE) &&
      m_visualFx.IsCurrentlyDrawable(GoomDrawable::SCOPE))
  {
    if (0 == m_lineMode)
    {
      m_lineMode = m_drawLinesDuration;
    }
    else if ((m_lineMode == m_drawLinesDuration) && m_visualFx.CanResetDestGoomLines())
    {
      --m_lineMode;

      ResetGoomLines();
    }
  }
}

inline void GoomControl::GoomControlImpl::DisplayLinesIfInAGoom(const AudioSamples& soundData)
{
  constexpr uint32_t DISPLAY_LINES_GOOM_NUM = 5;

  if ((m_lineMode != 0) ||
      (m_goomInfo->GetSoundInfo().GetTimeSinceLastGoom() < DISPLAY_LINES_GOOM_NUM))
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

  constexpr uint32_t CHANGE_GOOM_LINE_CYCLES = 121;

  if ((9 == (m_updateNum % CHANGE_GOOM_LINE_CYCLES)) &&
      m_goomEvent.Happens(GoomEvent::CHANGE_GOOM_LINE) &&
      ((0 == m_lineMode) || (m_lineMode == m_drawLinesDuration)) &&
      m_visualFx.CanResetDestGoomLines())
  {
    ResetGoomLines();
  }
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

  message +=
      std20::format("tranLerpFactor: {}\n", m_visualFx.GetZoomFilterFx().GetTranLerpFactor());
  message += std20::format("tranLerpIncrement: {}\n", filterBufferSettings.tranLerpIncrement);
  message +=
      std20::format("tranLerpToMaxSwitchMult: {}\n", filterBufferSettings.tranLerpToMaxSwitchMult);

  message += GetNameValuesString(m_visualFx.GetZoomFilterFx().GetNameValueParams("colors")) + "\n";
  message +=
      GetNameValuesString(m_visualFx.GetZoomFilterFx().GetNameValueParams("ZoomEffects")) + "\n";

  message += std20::format("middleX: {}\n", filterEffectsSettings.zoomMidPoint.x);
  message += std20::format("middleY: {}\n", filterEffectsSettings.zoomMidPoint.y);

  message += std20::format("vitesse: {}\n", filterEffectsSettings.vitesse.GetVitesse());
  message += std20::format("previousZoomSpeed: {}\n", m_previousZoomSpeed);
  message += std20::format("reverse: {}\n", filterEffectsSettings.vitesse.GetReverseVitesse());
  message +=
      std20::format("relative speed: {}\n", filterEffectsSettings.vitesse.GetRelativeSpeed());

  message += std20::format("updatesSinceLastChange: {}\n", m_updatesSinceLastZoomEffectsChange);

  UpdateMessages(message);
}
#endif

} // namespace GOOM
