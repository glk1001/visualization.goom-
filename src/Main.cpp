/*
 *  Copyright (C) 2005-2022 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2005-2013 Team XBMC
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Main.h"

#include "goom/goom_graphic.h"
#include "goom/sound_info.h"
#include "goom/logging_control.h"
#undef NO_LOGGING
#include "goom/logging.h"

#include <cstddef>
#include <memory>
#include <stdexcept>

using GOOM::AudioSamples;
using GOOM::Pixel;
using GOOM::PixelBuffer;
using GOOM::UTILS::Logging;

constexpr int MAX_QUALITY = 4;
constexpr std::array<int, MAX_QUALITY + 1> WIDTHS_BY_QUALITY{
    512, 640, 1280, 1600, 1920,
};
constexpr std::array<int, MAX_QUALITY + 1> HEIGHTS_BY_QUALITY{
    256, 360, 720, 900, 1080,
};

const size_t CVisualizationGoom::CIRCULAR_BUFFER_SIZE = GOOM::AudioSamples::NUM_AUDIO_SAMPLES * GOOM::AudioSamples::AUDIO_SAMPLE_LEN;

CVisualizationGoom::CVisualizationGoom()
  : m_texWidth{WIDTHS_BY_QUALITY.at(
        static_cast<size_t>(std::min(MAX_QUALITY, kodi::GetSettingInt("quality"))))},
    m_texHeight{HEIGHTS_BY_QUALITY.at(
        static_cast<size_t>(std::min(MAX_QUALITY, kodi::GetSettingInt("quality"))))},
    m_goomBufferLen{static_cast<size_t>(m_texWidth * m_texHeight)},
    m_goomBufferSize{m_goomBufferLen * sizeof(uint32_t)},
    m_windowWidth{Width()},
    m_windowHeight{Height()},
    m_windowXPos{X()},
    m_windowYPos{Y()},
    m_buffer{static_cast<uint32_t>(CIRCULAR_BUFFER_SIZE)}
{
#ifdef HAS_GL
  m_usePixelBufferObjects = kodi::GetSettingBoolean("use_pixel_buffer_objects");
#endif

  InitQuadData();

  kodi::Log(ADDON_LOG_DEBUG, "CVisualizationGoom: Created CVisualizationGoom object.");
}

//-- Destroy -------------------------------------------------------------------
// Do everything before unload of this add-on
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
CVisualizationGoom::~CVisualizationGoom()
{
  delete[] m_quadData;

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
    static const auto s_fKodiLog = [](const Logging::LogLevel lvl, const std::string& s) {
      const auto kodiLvl = static_cast<AddonLog>(static_cast<size_t>(lvl));
      kodi::Log(kodiLvl, s.c_str());
    };
    AddLogHandler("kodi-logger", s_fKodiLog);
    SetLogFile("/tmp/kodi_goom.log");
    SetLogLevelForFiles(Logging::LogLevel::INFO);
    //setLogLevelForFiles(Logging::LogLevel::debug);
    LogStart();
    LogInfo("CVisualizationGoom: Visualization starting - song name = \"{}\".", songName);
    LogInfo("CVisualizationGoom: Texture width = {} and height = {}.", m_texWidth, m_texHeight);

    m_numChannels = static_cast<size_t>(numChannels);
    m_audioBufferLen = m_numChannels * AudioSamples::AUDIO_SAMPLE_LEN;
    m_audioBufferNum = 0;
    m_currentSongName = songName;
    m_titleChange = true;

    // Make one init frame in black
    const auto sp{std::make_shared<PixelBuffer>(static_cast<uint32_t>(m_texWidth),
                                                static_cast<uint32_t>(m_texHeight))};
    sp->Fill(Pixel::BLACK);
    m_activeQueue.push(sp);

    // Init GL parts
    if (!LoadShaderFiles(kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl"),
                         kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl")))
    {
      LogError("CVisualizationGoom: Failed to load GL shaders.");
      return false;
    }

    if (!CompileAndLink())
    {
      LogError("CVisualizationGoom: Failed to compile GL shaders.");
      return false;
    }

    if (!InitGlObjects())
    {
      LogError("CVisualizationGoom: Could not initialize GL objects.");
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

    glDeleteTextures(1, &m_textureId);
    m_textureId = 0;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_vertexVBO);
    m_vertexVBO = 0;

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
  m_goomControl = std::make_unique<GOOM::GoomControl>(static_cast<uint16_t>(m_texWidth),
                                                      static_cast<uint16_t>(m_texHeight),
                                                      kodi::GetAddonPath("resources"));
  if (!m_goomControl)
  {
    LogError("CVisualizationGoom: Goom controller could not be initialized!");
    return false;
  }

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
    const std::unique_lock<std::mutex> lock(m_mutex);
    m_workerThreadExit = true;
    m_wait.notify_one();
  }
  if (m_workerThread.joinable())
  {
    m_workerThread.join();
  }
  LogInfo("CVisualizationGoom: Goom process buffers thread stopped.");
}

void CVisualizationGoom::OnCompiledAndLinked()
{
  m_uProjModelMatLoc = glGetUniformLocation(ProgramHandle(), "u_projModelMat");
  m_aPositionLoc = glGetAttribLocation(ProgramHandle(), "in_position");
  m_aCoordLoc = glGetAttribLocation(ProgramHandle(), "in_tex_coord");
}

auto CVisualizationGoom::OnEnabled() -> bool
{
  glUniformMatrix4fv(m_uProjModelMatLoc, 1, GL_FALSE, glm::value_ptr(m_projModelMatrix));
  return true;
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

  const std::unique_lock<std::mutex> lock(m_mutex);
  if (m_buffer.DataAvailable() >= CIRCULAR_BUFFER_SIZE)
  {
    AudioDataQueueTooBig();
    return;
  }

  (void)m_buffer.Write(audioData, static_cast<unsigned int>(audioDataLength));
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

  m_showTitleAlways = kodi::GetSettingBoolean("show_title_always");

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
    glVertexAttribPointer(m_aPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, m_quadData);
    glEnableVertexAttribArray(m_aPositionLoc);
    glVertexAttribPointer(m_aCoordLoc, 2, GL_FLOAT, GL_FALSE, 0,
                          m_quadData + m_numVertices * m_componentsPerVertex);
    glEnableVertexAttribArray(m_aCoordLoc);
#endif

    // Setup texture.
    glDisable(GL_BLEND);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    const std::shared_ptr<PixelBuffer> pixels = GetNextActivePixels();
    if (pixels != nullptr)
    {
#ifdef HAS_GL
      if (m_usePixelBufferObjects)
      {
        m_currentPboIndex = (m_currentPboIndex + 1) % G_NUM_PBOS;
        const int nextPboIndex = (m_currentPboIndex + 1) % G_NUM_PBOS;

        // Bind to current PBO and send pixels to texture object.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds[m_currentPboIndex]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texWidth, m_texHeight, GL_BGRA, GL_UNSIGNED_BYTE,
                        nullptr);

        // Bind to next PBO and update data directly on the mapped buffer.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds[nextPboIndex]);
        (void)std::memcpy(m_pboGoomBuffer[nextPboIndex],
                          std::const_pointer_cast<const PixelBuffer>(pixels)->GetIntBuff(),
                          m_goomBufferSize);

        (void)glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // release pointer to mapping buffer
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
      }
      else
#endif
      {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texWidth, m_texHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                        std::const_pointer_cast<const PixelBuffer>(pixels)->GetIntBuff());
      }

      PushUsedPixels(pixels);
    }

    (void)EnableShader();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
    DisableShader();

    glEnable(GL_BLEND);

#ifdef HAS_GL
    glBindVertexArray(0);
#else
    glDisableVertexAttribArray(m_aPositionLoc);
    glDisableVertexAttribArray(m_aCoordLoc);
#endif
  }
  catch (const std::exception& e)
  {
    LogError("CVisualizationGoom: Goom render failed: {}", e.what());
  }
}

inline auto CVisualizationGoom::GetNextActivePixels() -> std::shared_ptr<PixelBuffer>
{
  std::shared_ptr<PixelBuffer> pixels{};
  const std::lock_guard<std::mutex> lk(m_mutex);
  if (m_activeQueue.empty())
  {
    NoActiveBufferAvailable();
  }
  else
  {
    pixels = m_activeQueue.front();
    m_activeQueue.pop();
  }
  return pixels;
}

inline void CVisualizationGoom::PushUsedPixels(const std::shared_ptr<PixelBuffer>& pixels)
{
  const std::lock_guard<std::mutex> lk(m_mutex);
  m_storedQueue.push(pixels);
}

void CVisualizationGoom::Process()
{
  try
  {
    std::vector<float> floatAudioData(m_audioBufferLen);
    uint64_t buffNum = 0;

    while (true)
    {
      std::unique_lock<std::mutex> lk(m_mutex);
      if (m_workerThreadExit)
      {
        break;
      }
      if (m_buffer.DataAvailable() < m_audioBufferLen)
      {
        m_wait.wait(lk);
      }
      const uint32_t read =
          m_buffer.Read(floatAudioData.data(), static_cast<uint32_t>(m_audioBufferLen));
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

      std::shared_ptr<PixelBuffer> pixels;
      lk.lock();
      if (m_storedQueue.empty())
      {
        pixels = std::make_shared<PixelBuffer>(static_cast<uint32_t>(m_texWidth),
                                               static_cast<uint32_t>(m_texHeight));
      }
      else
      {
        pixels = m_storedQueue.front();
        m_storedQueue.pop();
      }
      lk.unlock();

      UpdateGoomBuffer(GetTitle(), floatAudioData, pixels);
      ++buffNum;

      lk.lock();
      m_activeQueue.push(pixels);
      lk.unlock();
    }
  }
  catch (const std::exception& e)
  {
    LogError("CVisualizationGoom: Goom process failed: {}", e.what());
  }
}

auto CVisualizationGoom::GetTitle() -> std::string
{
  if ((!m_titleChange) && (!m_showTitleAlways))
  {
    return "";
  }
  m_titleChange = false;
  return m_currentSongName;
}

void CVisualizationGoom::UpdateGoomBuffer(const std::string& title,
                                          const std::vector<float>& floatAudioData,
                                          std::shared_ptr<PixelBuffer>& pixels)
{
  const GOOM::AudioSamples audioData{m_numChannels, floatAudioData};
  m_goomControl->SetScreenBuffer(pixels);
  m_goomControl->Update(audioData, 0.0F, title, "");
}

void CVisualizationGoom::InitQuadData()
{
  const auto x0 = static_cast<GLfloat>(this->m_windowXPos);
  const auto y0 = static_cast<GLfloat>(this->m_windowYPos);
  const auto x1 = static_cast<GLfloat>(this->m_windowXPos + this->m_windowWidth);
  const auto y1 = static_cast<GLfloat>(this->m_windowYPos + this->m_windowHeight);
  // clang-format off
  const GLfloat tempQuadData[] = {
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

  m_numElements = sizeof(tempQuadData) / sizeof(GLfloat);
  m_quadData = new GLfloat[static_cast<size_t>(m_numElements)];
  for (int i = 0; i < m_numElements; ++i)
  {
    m_quadData[i] = tempQuadData[i];
  }
  m_componentsPerVertex = 2;
  m_componentsPerTexel = 2;
  m_numVertices = 2 * 3; // 2 triangles
  m_vertexVBO = 0;
}

auto CVisualizationGoom::InitGlObjects() -> bool
{
  m_projModelMatrix =
      glm::ortho(0.0F, static_cast<float>(Width()), 0.0F, static_cast<float>(Height()));

  // Setup vertex attributes
#ifdef HAS_GL
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  glGenVertexArrays(1, &m_vaoObject);
  glBindVertexArray(m_vaoObject);
  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glEnableVertexAttribArray(static_cast<GLuint>(m_aPositionLoc));
  glEnableVertexAttribArray(static_cast<GLuint>(m_aCoordLoc));
  glVertexAttribPointer(static_cast<GLuint>(m_aPositionLoc), m_componentsPerVertex, GL_FLOAT,
                        GL_FALSE, 0, nullptr);
  glVertexAttribPointer(
      static_cast<GLuint>(m_aCoordLoc), m_componentsPerTexel, GL_FLOAT, GL_FALSE, 0,
      reinterpret_cast<GLvoid*>(
          (static_cast<size_t>(m_numVertices * m_componentsPerVertex) * sizeof(GLfloat))));
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(static_cast<size_t>(m_numElements) * sizeof(GLfloat)),
               m_quadData, GL_STATIC_DRAW);
  glBindVertexArray(0);
#endif

  // Create texture.
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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texWidth, m_texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               nullptr);
  glBindTexture(GL_TEXTURE_2D, 0);

#ifdef HAS_GL
  if (!m_usePixelBufferObjects)
  {
    LogInfo("CVisualizationGoom: Not using pixel buffer objects.");
  }
  else
  {
    LogInfo("CVisualizationGoom: Using pixel buffer objects.");
    m_currentPboIndex = 0;

    glGenBuffers(G_NUM_PBOS, m_pboIds);
    for (int i = 0; i < G_NUM_PBOS; ++i)
    {
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds[i]);
      glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(m_goomBufferSize), nullptr,
                   GL_STREAM_DRAW);
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds[i]);
      glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(m_goomBufferSize), nullptr,
                   GL_STREAM_DRAW);
      m_pboGoomBuffer[i] =
          static_cast<unsigned char*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
      if (!m_pboGoomBuffer[i])
      {
        LogError("CVisualizationGoom: Could not do glMapBuffer for pbo %d.", i);
        return false;
      }
    }
    (void)glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // release pointer to mapping buffer
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  }
#endif

  return true;
}

#ifndef DO_TESTING
ADDONCREATOR(CVisualizationGoom) // Don't touch this!
#else
#pragma message("Compiling " __FILE__ " with 'DO_TESTING' ON.")
#endif
