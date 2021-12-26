/*
 *  Copyright (C) 2005-2022 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2005-2013 Team XBMC
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Main.h"

#include "goom/compiler_versions.h"
#include "goom/goom_graphic.h"
#include "goom/logging_control.h"
#undef NO_LOGGING
#include "goom/logging.h"
#include "goom/sound_info.h"

#undef NDEBUG
#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>

using GOOM::AudioSamples;
using GOOM::GetCompilerVersion;
using GOOM::GoomControl;
using GOOM::Pixel;
using GOOM::PixelBuffer;
using GOOM::UTILS::Logging;

// TODO Figure out correct format here
//      - GL_BGRA looks good but why?
//constexpr GLenum TEXTURE_FORMAT = GL_RGBA;
constexpr GLenum TEXTURE_FORMAT = GL_BGRA;
//constexpr GLint TEXTURE_SIZED_INTERNAL_FORMAT = GL_RGBA;
constexpr GLint TEXTURE_SIZED_INTERNAL_FORMAT = GL_RGBA16;
//constexpr GLenum TEXTURE_DATA_TYPE = GL_UNSIGNED_BYTE;
constexpr GLenum TEXTURE_DATA_TYPE = GL_UNSIGNED_SHORT;

constexpr int MAX_QUALITY = 4;
constexpr std::array<uint32_t, MAX_QUALITY + 1> WIDTHS_BY_QUALITY{
    512, 640, 1280, 1600, 1920,
};
constexpr std::array<uint32_t, MAX_QUALITY + 1> HEIGHTS_BY_QUALITY{
    256, 360, 720, 900, 1080,
};

const size_t CVisualizationGoom::CIRCULAR_BUFFER_SIZE = NUM_AUDIO_BUFFERS_IN_CIRCULAR_BUFFER *
                                                        GOOM::AudioSamples::NUM_AUDIO_SAMPLES *
                                                        GOOM::AudioSamples::AUDIO_SAMPLE_LEN;

CVisualizationGoom::CVisualizationGoom()
  : m_windowWidth{Width()},
    m_windowHeight{Height()},
    m_windowXPos{X()},
    m_windowYPos{Y()},
    m_textureWidth{WIDTHS_BY_QUALITY.at(
        static_cast<size_t>(std::min(MAX_QUALITY, kodi::GetSettingInt("quality"))))},
    m_textureHeight{HEIGHTS_BY_QUALITY.at(
        static_cast<size_t>(std::min(MAX_QUALITY, kodi::GetSettingInt("quality"))))},
    m_goomBufferLen{static_cast<size_t>(m_textureWidth * m_textureHeight)},
    m_goomBufferSize{PixelBuffer::GetIntBufferSize(m_textureWidth, m_textureHeight)},
    m_showTitle{static_cast<GoomControl::ShowTitleType>(kodi::GetSettingInt("show_title"))},
    m_quadData{GetGlQuadData(m_windowWidth, m_windowHeight, m_windowXPos, m_windowYPos)},
    m_usePixelBufferObjects{kodi::GetSettingBoolean("use_pixel_buffer_objects")}
{
  kodi::Log(ADDON_LOG_DEBUG, "CVisualizationGoom: Created CVisualizationGoom object.");
}

//-- Destroy -------------------------------------------------------------------
// Do everything before unload of this add-on
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
CVisualizationGoom::~CVisualizationGoom()
{
  kodi::Log(ADDON_LOG_DEBUG, "CVisualizationGoom: Destroyed CVisualizationGoom object.");
}

//-- Start --------------------------------------------------------------------
// Called when a new soundtrack is played
//-----------------------------------------------------------------------------
auto CVisualizationGoom::Start(const int numChannels,
                               [[maybe_unused]] const int samplesPerSec,
                               [[maybe_unused]] const int bitsPerSample,
                               const std::string& songName) -> bool
{
  if (m_started)
  {
    kodi::Log(ADDON_LOG_WARNING,
              "CVisualizationGoom: Already started without a stop - skipping this.");
    return true;
  }

  try
  {
    StartLogging();

    LogInfo("CVisualizationGoom: Texture width, height = {}, {}.", m_textureWidth, m_textureHeight);

    SetNumChannels(numChannels);
    SetSongTitle(songName);
    StartActiveQueue();

    if (!InitGl())
    {
      LogError("CVisualizationGoom: Could not initialize OpenGL.");
      return false;
    }

    if (!InitGoomController())
    {
      LogError("CVisualizationGoom: Could not initialize Goom Controller.");
      return false;
    }

    m_started = true;

    return true;
  }
  catch (const std::exception& e)
  {
    LogError("CVisualizationGoom: Goom start failed: {}", e.what());
    return false;
  }
}

void CVisualizationGoom::SetNumChannels(const int numChannels)
{
  m_numChannels = static_cast<size_t>(numChannels);
  m_audioBufferLen = m_numChannels * AudioSamples::AUDIO_SAMPLE_LEN;
  m_audioBufferNum = 0;
}

void CVisualizationGoom::SetSongTitle(const std::string& songTitle)
{
  LogInfo("CVisualizationGoom: Visualization starting - song title = \"{}\".", songTitle);
  m_currentSongName = songTitle;
  m_titleChange = true;
}

void CVisualizationGoom::StartLogging()
{
  static const auto s_fKodiLog = [](const Logging::LogLevel lvl, const std::string& msg) {
    const auto kodiLvl = static_cast<AddonLog>(static_cast<size_t>(lvl));
    kodi::Log(kodiLvl, msg.c_str());
  };
  AddLogHandler("kodi-logger", s_fKodiLog);
  SetLogFile("/tmp/kodi_goom.log");
  SetLogLevelForFiles(Logging::LogLevel::INFO);
  //setLogLevelForFiles(Logging::LogLevel::debug);
  LogStart();
}

void CVisualizationGoom::StartActiveQueue()
{
  // Make one initial frame in black
  PixelBufferData initialBufferData{MakePixelBufferData()};
  initialBufferData.pixelBuffer->Fill(Pixel::BLACK);
  m_activeQueue.push(initialBufferData);
}

inline auto CVisualizationGoom::MakePixelBufferData() const -> PixelBufferData
{
  PixelBufferData pixelBufferData;
  pixelBufferData.pixelBuffer = std::make_shared<PixelBuffer>(m_textureWidth, m_textureHeight);
  return pixelBufferData;
}

//-- Stop ---------------------------------------------------------------------
// Called when the visualisation is closed by Kodi
//-----------------------------------------------------------------------------
void CVisualizationGoom::Stop()
{
  if (!m_started)
  {
    LogWarn("CVisualizationGoom: Not started - skipping this.");
    return;
  }

  try
  {
    LogInfo("CVisualizationGoom: Visualization stopping.");
    m_started = false;

    StopGoomProcessBuffersThread();

    DeinitGoomController();

    DeinitGl();

    m_audioBufferNum = 0;
  }
  catch (const std::exception& e)
  {
    LogError("CVisualizationGoom: Goom stop failed: {}", e.what());
  }
}

auto CVisualizationGoom::InitGoomController() -> bool
{
  if (m_goomControl)
  {
    LogError("CVisualizationGoom: Goom controller already initialized!");
    return true;
  }

  LogInfo("CVisualizationGoom: Initializing goom controller.");
  m_goomControl = std::make_unique<GOOM::GoomControl>(m_textureWidth, m_textureHeight,
                                                      kodi::GetAddonPath("resources"));
  if (!m_goomControl)
  {
    LogError("CVisualizationGoom: Goom controller could not be initialized!");
    return false;
  }

  LogInfo("CVisualizationGoom: Compiler: {}.", GetCompilerVersion());
  LogInfo("Goom Library: Compiler: {}.", GoomControl::GetCompilerVersion());

  m_goomControl->SetShowTitle(m_showTitle);

  // goom will use same random sequence if following is uncommented
  // goom::GoomControl::setRandSeed(1);
  m_goomControl->Start();

  return true;
}

void CVisualizationGoom::DeinitGoomController()
{
  if (!m_goomControl)
  {
    LogError("CVisualizationGoom: Goom controller already uninitialized!");
    return;
  }

  m_goomControl->Finish();
  m_goomControl.reset();
  LogInfo("CVisualizationGoom: Uninitialized goom controller.");
}

void CVisualizationGoom::StartGoomProcessBuffersThread()
{

  LogInfo("CVisualizationGoom: Starting goom process buffers thread.");
  m_workerThread = std::thread(&CVisualizationGoom::Process, this);
}

void CVisualizationGoom::StopGoomProcessBuffersThread()
{
  LogInfo("CVisualizationGoom: Stopping goom process buffers thread.");
  {
    const std::unique_lock lock(m_mutex);
    m_workerThreadExit = true;
    m_wait.notify_one();
  }
  if (m_workerThread.joinable())
  {
    m_workerThread.join();
  }
  LogInfo("CVisualizationGoom: Goom process buffers thread stopped.");
}

//-- AudioData ----------------------------------------------------------------
// Called by Kodi to pass new audio data to the vis
//-----------------------------------------------------------------------------
void CVisualizationGoom::AudioData(const float* const audioData, size_t audioDataLength)
{
  if (!m_started)
  {
    LogError("CVisualizationGoom: Goom not started - cannot process audio.");
    return;
  }

  ++m_audioBufferNum;

  if (1 == m_audioBufferNum)
  {
    LogInfo("CVisualizationGoom: Starting audio data processing.");
    StartGoomProcessBuffersThread();
  }

  const std::unique_lock lock(m_mutex);
  if (m_audioBuffer.DataAvailable() >= CIRCULAR_BUFFER_SIZE)
  {
    AudioDataQueueTooBig();
    return;
  }

  m_audioBuffer.Write(audioData, static_cast<size_t>(audioDataLength));
  m_wait.notify_one();
}

auto CVisualizationGoom::UpdateTrack(const kodi::addon::VisualizationTrack& track) -> bool
{
  if (!m_goomControl)
  {
    return true;
  }

  m_lastSongName = m_currentSongName;

  std::string artist = track.GetArtist();
  if (artist.empty())
  {
    artist = track.GetAlbumArtist();
  }

  if (!artist.empty())
  {
    m_currentSongName = artist + " - " + track.GetTitle();
  }
  else
  {
    m_currentSongName = track.GetTitle();
  }

  if (m_lastSongName != m_currentSongName)
  {
    m_titleChange = true;
  }

  return true;
}

inline auto CVisualizationGoom::GetNextActivePixelBufferData() -> PixelBufferData
{
  PixelBufferData pixelBufferData{};

  const std::scoped_lock lk(m_mutex);
  if (m_activeQueue.empty())
  {
    NoActiveBufferAvailable();
  }
  else
  {
    pixelBufferData = m_activeQueue.front();
    m_activeQueue.pop();
  }

  return pixelBufferData;
}

inline void CVisualizationGoom::PushUsedPixels(const PixelBufferData& pixelBufferData)
{
  const std::scoped_lock lk(m_mutex);
  m_storedQueue.push(pixelBufferData);
}

void CVisualizationGoom::Process()
{
  try
  {
    std::vector<float> floatAudioData(m_audioBufferLen);
    uint64_t buffNum = 0;

    while (true)
    {
      std::unique_lock lk(m_mutex);
      if (m_workerThreadExit)
      {
        break;
      }

      if (m_audioBuffer.DataAvailable() < m_audioBufferLen)
      {
        m_wait.wait(lk);
      }
      const size_t read = m_audioBuffer.Read(floatAudioData.data(), m_audioBufferLen);
      if (read != m_audioBufferLen)
      {
        LogWarn("CVisualizationGoom: Num read audio length {} != {} = expected audio data length - "
                "skipping this.",
                read, m_audioBufferLen);
        AudioDataIncorrectReadLength();
        continue;
      }
      lk.unlock();

      if (m_workerThreadExit)
      {
        break;
      }

      lk.lock();
      if (m_activeQueue.size() > MAX_ACTIVE_QUEUE_LENGTH)
      {
        // Too far behind, skip this audio data.
        SkippedAudioData();
        continue;
      }
      lk.unlock();

      PixelBufferData pixelBufferData;
      lk.lock();
      if (m_storedQueue.empty())
      {
        pixelBufferData = MakePixelBufferData();
      }
      else
      {
        pixelBufferData = m_storedQueue.front();
        m_storedQueue.pop();
      }
      lk.unlock();

      UpdateGoomBuffer(GetTitle(), floatAudioData, pixelBufferData);
      ++buffNum;

      lk.lock();
      m_activeQueue.push(pixelBufferData);
      lk.unlock();
    }
  }
  catch (const std::exception& e)
  {
    LogError("CVisualizationGoom: Goom process failed: {}", e.what());
  }
}

inline auto CVisualizationGoom::GetTitle() -> std::string
{
  if (!m_titleChange)
  {
    return "";
  }
  m_titleChange = false;
  return m_currentSongName;
}

inline void CVisualizationGoom::UpdateGoomBuffer(const std::string& title,
                                                 const std::vector<float>& floatAudioData,
                                                 PixelBufferData& pixelBufferData)
{
  const GOOM::AudioSamples audioData{m_numChannels, floatAudioData};
  m_goomControl->SetScreenBuffer(pixelBufferData.pixelBuffer);
  m_goomControl->Update(audioData, 0.0F, title, "");
  pixelBufferData.goomShaderEffects = m_goomControl->GetLastShaderEffects();
}

auto CVisualizationGoom::InitGl() -> bool
{
  if (!InitGlShaders())
  {
    LogError("CVisualizationGoom: Failed to initialize GL shaders.");
    return false;
  }

  if (!InitGlObjects())
  {
    LogError("CVisualizationGoom: Could not initialize GL objects.");
    return false;
  }

  return true;
}

void CVisualizationGoom::DeinitGl()
{
  glDeleteTextures(1, &m_textureId);
  m_textureId = 0;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
}

auto CVisualizationGoom::GetGlQuadData(const int width,
                                       const int height,
                                       const int xPos,
                                       const int yPos) -> std::vector<GLfloat>
{
  const auto x0 = static_cast<GLfloat>(xPos);
  const auto y0 = static_cast<GLfloat>(yPos);
  const auto x1 = static_cast<GLfloat>(xPos + width);
  const auto y1 = static_cast<GLfloat>(yPos + height);

  // clang-format off
  return {
      // Vertex positions
      x0, y0,  // bottom left
      x0, y1,  // top left
      x1, y0,  // bottom right
      x1, y0,  // bottom right
      x1, y1,  // top right
      x0, y1,  // top left
      // Texture coordinates
      0.0, 1.0,
      0.0, 0.0,
      1.0, 1.0,
      1.0, 1.0,
      1.0, 0.0,
      0.0, 0.0,
  };
  // clang-format on
}

auto CVisualizationGoom::InitGlShaders() -> bool
{
  const std::string shaderSubdir = "resources/shaders/" GL_TYPE_STRING;
  const std::string vertexShaderFile = kodi::GetAddonPath(shaderSubdir + "/vertex.glsl");
  const std::string fragmentShaderFile = kodi::GetAddonPath(shaderSubdir + "/fragment.glsl");

  if (!LoadShaderFiles(vertexShaderFile, fragmentShaderFile))
  {
    LogError("CVisualizationGoom: Failed to load GL shaders.");
    return false;
  }

  if (!CompileAndLink())
  {
    LogError("CVisualizationGoom: Failed to compile GL shaders.");
    return false;
  }

  return true;
}

void CVisualizationGoom::OnCompiledAndLinked()
{
  m_uProjModelMatLoc = glGetUniformLocation(ProgramHandle(), "u_projModelMat");
  m_aPositionLoc = glGetAttribLocation(ProgramHandle(), "in_position");
  m_aTexCoordsLoc = glGetAttribLocation(ProgramHandle(), "in_texCoords");
  m_uTexExposureLoc = glGetUniformLocation(ProgramHandle(), "u_texExposure");
  m_uTexBrightnessLoc = glGetUniformLocation(ProgramHandle(), "u_texBrightness");
  m_uTexContrastLoc = glGetUniformLocation(ProgramHandle(), "u_texContrast");
  m_uTexContrastMinChannelValueLoc = glGetUniformLocation(ProgramHandle(), "u_texContrastMinChan");
  m_uTimeLoc = glGetUniformLocation(ProgramHandle(), "u_time");
}

auto CVisualizationGoom::OnEnabled() -> bool
{
  glUniformMatrix4fv(m_uProjModelMatLoc, 1, GL_FALSE, glm::value_ptr(m_projModelMatrix));
  return true;
}

auto CVisualizationGoom::InitGlObjects() -> bool
{
  m_projModelMatrix =
      glm::ortho(0.0F, static_cast<float>(Width()), 0.0F, static_cast<float>(Height()));

  SetupGlVertexAttributes();

  return CreateGlTexture();
}

void CVisualizationGoom::SetupGlVertexAttributes()
{
#ifdef HAS_GL
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  glGenVertexArrays(1, &m_vaoObject);
  glBindVertexArray(m_vaoObject);
  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glEnableVertexAttribArray(static_cast<GLuint>(m_aPositionLoc));
  glEnableVertexAttribArray(static_cast<GLuint>(m_aTexCoordsLoc));
  glVertexAttribPointer(static_cast<GLuint>(m_aPositionLoc), m_componentsPerVertex, GL_FLOAT,
                        GL_FALSE, 0, nullptr);
  glVertexAttribPointer(
      static_cast<GLuint>(m_aTexCoordsLoc), m_componentsPerTexel, GL_FLOAT, GL_FALSE, 0,
      reinterpret_cast<GLvoid*>(
          (static_cast<size_t>(m_numVertices * m_componentsPerVertex) * sizeof(GLfloat))));
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_quadData.size() * sizeof(GLfloat)),
               m_quadData.data(), GL_STATIC_DRAW);
  glBindVertexArray(0);
#endif
}

auto CVisualizationGoom::CreateGlTexture() -> bool
{
  glGenTextures(1, &m_textureId);
  if (0 == m_textureId)
  {
    LogError("CVisualizationGoom: Could not do glGenTextures.");
    return false;
  }

  glClear(GL_COLOR_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, m_textureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#ifdef HAS_GL
  glGenerateMipmap(GL_TEXTURE_2D);
#endif
  constexpr GLint LEVEL = 0;
  constexpr GLint BORDER = 0;
  constexpr void* const NULL_DATA = nullptr;
  glTexImage2D(GL_TEXTURE_2D, LEVEL, TEXTURE_SIZED_INTERNAL_FORMAT,
               static_cast<GLsizei>(m_textureWidth), static_cast<GLsizei>(m_textureHeight), BORDER,
               TEXTURE_FORMAT, TEXTURE_DATA_TYPE, NULL_DATA);
  glBindTexture(GL_TEXTURE_2D, 0);

#ifdef HAS_GL
  LogInfo("CVisualizationGoom: Using pixel buffer objects.");
  return SetupGlPixelBufferObjects();
#else
  return true;
#endif
}

auto CVisualizationGoom::SetupGlPixelBufferObjects() -> bool
{
  if (!m_usePixelBufferObjects)
  {
    LogInfo("CVisualizationGoom: Not using pixel buffer objects.");
    return true;
  }

  m_currentPboIndex = 0;

  glGenBuffers(G_NUM_PBOS, m_pboIds.data());

  for (size_t i = 0; i < G_NUM_PBOS; ++i)
  {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds.at(i));
    glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(m_goomBufferSize), nullptr,
                 GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds.at(i));
    glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(m_goomBufferSize), nullptr,
                 GL_STREAM_DRAW);
    m_pboGoomBuffer.at(i) =
        static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
    if (!m_pboGoomBuffer.at(i))
    {
      LogError("CVisualizationGoom: Could not do glMapBuffer for pbo %d.", i);
      return false;
    }
  }

  glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // release pointer to mapping buffer
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

  return true;
}

//-- Render -------------------------------------------------------------------
// Called once per frame. Do all rendering here.
//-----------------------------------------------------------------------------

auto CVisualizationGoom::IsDirty() -> bool
{
  return true; //!m_worker.RequestsQueueEmpty();
}

void CVisualizationGoom::Render()
{
  if (!m_started)
  {
    LogWarn("CVisualizationGoom: Goom not started - skipping this.");
    return;
  }

  if (m_audioBufferNum < MIN_AUDIO_BUFFERS_BEFORE_STARTING)
  {
    // Skip the first few - for some reason Kodi does a
    // 'reload' before really starting the music.
    return;
  }

  try
  {
    // Setup vertex attributes.
#ifdef HAS_GL
    glBindVertexArray(m_vaoObject);
#else
    glVertexAttribPointer(m_aPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, m_quadData.data());
    glEnableVertexAttribArray(m_aPositionLoc);
    glVertexAttribPointer(m_aTexCoordsLoc, 2, GL_FLOAT, GL_FALSE, 0,
                          m_quadData + m_numVertices * m_componentsPerVertex);
    glEnableVertexAttribArray(m_aTexCoordsLoc);
#endif

    // Setup texture.
    glDisable(GL_BLEND);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    const PixelBufferData pixelBufferData = GetNextActivePixelBufferData();
    if (pixelBufferData.pixelBuffer != nullptr)
    {
      RenderGlPixelBuffer(*pixelBufferData.pixelBuffer);
      PushUsedPixels(pixelBufferData);
    }

    EnableShader();
    if (pixelBufferData.pixelBuffer != nullptr)
    {
      SetGlShaderValues(pixelBufferData.goomShaderEffects);
    }
    glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
    DisableShader();

    glEnable(GL_BLEND);

#ifdef HAS_GL
    glBindVertexArray(0);
#else
    glDisableVertexAttribArray(m_aPositionLoc);
    glDisableVertexAttribArray(m_aTexCoordsLoc);
#endif
  }
  catch (const std::exception& e)
  {
    LogError("CVisualizationGoom: Goom render failed: {}", e.what());
  }
}

inline void CVisualizationGoom::RenderGlPixelBuffer(const GOOM::PixelBuffer& pixelBuffer)
{
#ifdef HAS_GL
  if (m_usePixelBufferObjects)
  {
    m_currentPboIndex = (m_currentPboIndex + 1) % G_NUM_PBOS;
    const size_t nextPboIndex = (m_currentPboIndex + 1) % G_NUM_PBOS;

    // Bind to current PBO and send pixels to texture object.
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds.at(m_currentPboIndex));

    constexpr GLint LEVEL = 0;
    constexpr GLint X_OFFSET = 0;
    constexpr GLint Y_OFFSET = 0;
    constexpr void* const NULL_PIXELS = nullptr;
    glTexSubImage2D(GL_TEXTURE_2D, LEVEL, X_OFFSET, Y_OFFSET, static_cast<GLsizei>(m_textureWidth),
                    static_cast<GLsizei>(m_textureHeight), TEXTURE_FORMAT, TEXTURE_DATA_TYPE,
                    NULL_PIXELS);

    // Bind to next PBO and update data directly on the mapped buffer.
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds.at(nextPboIndex));
    std::memcpy(m_pboGoomBuffer.at(nextPboIndex), pixelBuffer.GetIntBuff(), m_goomBufferSize);

    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // release pointer to mapping buffer
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  }
  else
#endif
  {
    constexpr GLint LEVEL = 0;
    constexpr GLint X_OFFSET = 0;
    constexpr GLint Y_OFFSET = 0;
    glTexSubImage2D(GL_TEXTURE_2D, LEVEL, X_OFFSET, Y_OFFSET, static_cast<GLsizei>(m_textureWidth),
                    static_cast<GLsizei>(m_textureHeight), TEXTURE_FORMAT, TEXTURE_DATA_TYPE,
                    pixelBuffer.GetIntBuff());
  }
}

inline void CVisualizationGoom::SetGlShaderValues(
    const GOOM::GoomShaderEffects& goomShaderEffects) const
{
  if (goomShaderEffects.exposure > 0.0F)
  {
    glUniform1f(m_uTexExposureLoc, goomShaderEffects.exposure);
  }
  if (goomShaderEffects.brightness > 0.0F)
  {
    glUniform1f(m_uTexBrightnessLoc, goomShaderEffects.brightness);
  }
  if (goomShaderEffects.contrast > 0.0F)
  {
    glUniform1f(m_uTexContrastLoc, goomShaderEffects.contrast);
    glUniform1f(m_uTexContrastMinChannelValueLoc, goomShaderEffects.contrastMinChannelValue);
  }
  static GLint m_time = 0;
  ++m_time;
  glUniform1i(m_uTimeLoc, m_time);
}

#ifndef DO_TESTING
ADDONCREATOR(CVisualizationGoom) // Don't touch this!
#else
#pragma message("Compiling " __FILE__ " with 'DO_TESTING' ON.")
#endif
