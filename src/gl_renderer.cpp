#if defined(__apple_build_version__) || __clang_major__ == 14
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#undef NO_LOGGING

#include "gl_renderer.h"

#include "goom/goom_logger.h"
#include "goom/goom_utils.h"

#include <array>
#include <cstring>
#include <format>
#include <stdexcept>
#include <vector>

#ifdef IS_KODI_BUILD
#include <kodi/gui/gl/GL.h>
#else
#include "glad/glad.h"
#endif

#ifndef HAS_GL
//static_assert(false, "Must have 'HAS_GL'.");
#endif
// #undef HAS_GL

namespace GOOM
{

class GlRenderer::GLRendererImpl
{
public:
  GLRendererImpl(const TextureBufferDimensions& textureBufferDimensions,
                 const WindowDimensions& windowDimensions,
                 IShaderStrategy* shaderStrategy,
                 GoomLogger& goomLogger);
  GLRendererImpl(const GLRendererImpl&)                    = delete;
  GLRendererImpl(GLRendererImpl&&)                         = delete;
  ~GLRendererImpl()                                        = default;
  auto operator=(const GLRendererImpl&) -> GLRendererImpl& = delete;
  auto operator=(GLRendererImpl&&) -> GLRendererImpl&      = delete;

  auto Start() -> void;
  auto Stop() -> void;

  auto SetPixelBuffer(const PixelChannelType* pixelBuffer) noexcept -> void;
  auto Render() -> void;

private:
  TextureBufferDimensions m_textureBufferDimensions;
  GoomLogger* m_goomLogger;

  int32_t m_windowWidth;
  int32_t m_windowHeight;

  size_t m_textureBufferSize = static_cast<size_t>((4U * m_textureBufferDimensions.width) *
                                                   m_textureBufferDimensions.height) *
                               sizeof(PixelChannelType);
#ifdef HAS_GL
  bool m_usePixelBufferObjects   = true;
  static constexpr auto NUM_PBOS = 2U;
  std::array<GLuint, NUM_PBOS> m_pboIds{};
  std::array<PixelChannelType*, NUM_PBOS> m_pboMappedBuffer{};
  size_t m_currentPboIndex = 0U;
  GLuint m_vaoObject       = 0;
  auto AllocateGlTextureBuffers() -> void;
  auto UpdatePboGlTextureBuffer() -> void;
#endif

  GLint m_componentsPerVertex = 2;
#ifdef HAS_GL
  GLint m_componentsPerTexel = 2;
#endif
  static constexpr int32_t NUM_VERTICES_IN_TRIANGLE = 3;
  static constexpr int32_t NUM_TRIANGLES            = 2;
  int32_t m_numVertices                             = NUM_TRIANGLES * NUM_VERTICES_IN_TRIANGLE;
  std::vector<GLfloat> m_quadData;
  struct QuadDataProperties
  {
    int32_t width;
    int32_t height;
    int32_t xPos;
    int32_t yPos;
  };
  [[nodiscard]] static auto GetGlQuadData(const QuadDataProperties& quadDataProperties)
      -> std::vector<GLfloat>;

  GLuint m_textureId    = 0;
  GLuint m_vertexVBO    = 0;
  GLint m_aPositionLoc  = -1;
  GLint m_aTexCoordsLoc = -1;

  IShaderStrategy* m_glShader;
  const PixelChannelType* m_pixelBuffer{};

  auto InitGl() -> void;
  auto DeinitGl() -> void;
  auto InitGlShaders() -> void;
  auto InitGlObjects() -> void;
  auto InitGlVertexAttributes() -> void;
  auto InitVertexAttributes() const -> void;
#ifdef HAS_GL
  static auto DeinitVertexAttributes() -> void;
#else
  auto DeinitVertexAttributes() const -> void;
#endif
  auto CreateGlTexture() -> void;
  auto DrawGlTexture() -> void;
  auto UpdateGlTextureBuffer() -> void;
  auto UpdateNormalGlTextureBuffer() -> void;

  auto EnableShader() -> void;
  auto UpdateShader() -> void;
  auto DisableShader() -> void;
  auto CompileAndLinkShaders() -> void;
  [[nodiscard]] auto GetShaderHandle() const noexcept -> GLuint;
  auto InitGlShaderVariables() -> void;
};

GlRenderer::GlRenderer(const TextureBufferDimensions& textureBufferDimensions,
                       const WindowDimensions& windowDimensions,
                       IShaderStrategy* const shaderStrategy,
                       GoomLogger& goomLogger)
  : m_pimpl{spimpl::make_unique_impl<GLRendererImpl>(
        textureBufferDimensions, windowDimensions, shaderStrategy, goomLogger)}
{
}

auto GlRenderer::Start() -> void
{
  m_pimpl->Start();
}

auto GlRenderer::Stop() -> void
{
  m_pimpl->Stop();
}

auto GlRenderer::SetPixelBuffer(const PixelChannelType* const pixelBuffer) noexcept -> void
{
  m_pimpl->SetPixelBuffer(pixelBuffer);
}

auto GlRenderer::Render() -> void
{
  return m_pimpl->Render();
}

// TODO(glk) - The following need to match PixelChannelType
#ifdef HAS_GL
static constexpr GLenum TEXTURE_FORMAT               = GL_RGBA;
static constexpr GLint TEXTURE_SIZED_INTERNAL_FORMAT = GL_RGBA16;
#else
static constexpr GLenum TEXTURE_FORMAT = GL_RGBA;
// TODO Not correct but compiles - that's a start.
#ifndef GL_RGBA16
static constexpr GLint GL_RGBA16                     = 0x805B;
#endif
static constexpr GLint TEXTURE_SIZED_INTERNAL_FORMAT = GL_RGBA16;
#endif
static constexpr GLenum TEXTURE_DATA_TYPE = GL_UNSIGNED_SHORT;

GlRenderer::GLRendererImpl::GLRendererImpl(const TextureBufferDimensions& textureBufferDimensions,
                                           const WindowDimensions& windowDimensions,
                                           IShaderStrategy* const shaderStrategy,
                                           GoomLogger& goomLogger)
  : m_textureBufferDimensions{textureBufferDimensions},
    m_goomLogger{&goomLogger},
    m_windowWidth{windowDimensions.width},
    m_windowHeight{windowDimensions.height},
    //TODO(glk) - Is pos (0,0) OK? Used to pass in pos from Kodi.
    m_quadData{GetGlQuadData({m_windowWidth, m_windowHeight, 0, 0})},
    m_glShader{shaderStrategy}
{
  LogDebug(*m_goomLogger, "Start constructor.");
}

inline auto GlRenderer::GLRendererImpl::SetPixelBuffer(
    const PixelChannelType* const pixelBuffer) noexcept -> void
{
  m_pixelBuffer = pixelBuffer;
}

inline auto GlRenderer::GLRendererImpl::Start() -> void
{
  LogInfo(*m_goomLogger, "Calling Start...");

  InitGl();
}

inline auto GlRenderer::GLRendererImpl::Stop() -> void
{
  LogInfo(*m_goomLogger, "Calling Stop...");

  DeinitGl();
}

auto GlRenderer::GLRendererImpl::InitGl() -> void
{
  LogDebug(*m_goomLogger, "Start InitGl.");
  LogDebug(*m_goomLogger, "glGetError() = {}", glGetError());

  LogDebug(*m_goomLogger, "GL_VERSION  : {}", ptr_cast<const char*>(glGetString(GL_VERSION)));
  LogDebug(*m_goomLogger, "GL_VENDOR   : {}", ptr_cast<const char*>(glGetString(GL_VENDOR)));
  LogDebug(*m_goomLogger, "GL_RENDERER : {}", ptr_cast<const char*>(glGetString(GL_RENDERER)));
  LogDebug(*m_goomLogger,
           "GLSL VERSION: {}",
           ptr_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

  InitGlShaders();
  InitGlObjects();

  LogDebug(*m_goomLogger, "glGetError() = {}", glGetError());
  LogDebug(*m_goomLogger, "Finish InitGl.");
}

auto GlRenderer::GLRendererImpl::DeinitGl() -> void
{
  if (0 != m_textureId)
  {
    glDeleteTextures(1, &m_textureId);
    m_textureId = 0;
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
}

auto GlRenderer::GLRendererImpl::GetGlQuadData(const QuadDataProperties& quadDataProperties)
    -> std::vector<GLfloat>
{
  const auto x0 = static_cast<GLfloat>(quadDataProperties.xPos);
  const auto y0 = static_cast<GLfloat>(quadDataProperties.yPos);
  const auto x1 = static_cast<GLfloat>(quadDataProperties.xPos + quadDataProperties.width);
  const auto y1 = static_cast<GLfloat>(quadDataProperties.yPos + quadDataProperties.height);

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

auto GlRenderer::GLRendererImpl::InitGlShaders() -> void
{
  LogDebug(*m_goomLogger, "Start InitGlShaders.");
  LogDebug(*m_goomLogger, "glGetError() = {}", glGetError());

  CompileAndLinkShaders();

  LogDebug(*m_goomLogger, "glGetError() = {}", glGetError());
  LogDebug(*m_goomLogger, "Finish InitGlShaders.");
}

auto GlRenderer::GLRendererImpl::InitGlObjects() -> void
{
  LogDebug(*m_goomLogger, "Start InitGlObjects.");
  LogDebug(*m_goomLogger, "glGetError() = {}", glGetError());

  InitGlVertexAttributes();
  CreateGlTexture();

  LogDebug(*m_goomLogger, "glGetError() = {}", glGetError());
  LogDebug(*m_goomLogger, "Finish InitGlObjects.");
}

auto GlRenderer::GLRendererImpl::InitGlVertexAttributes() -> void
{
  LogDebug(*m_goomLogger, "Start InitGlVertexAttributes.");
  LogDebug(*m_goomLogger, "glGetError() = {}", glGetError());

#ifdef HAS_GL
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);

  glGenVertexArrays(1, &m_vaoObject);
  glBindVertexArray(m_vaoObject);
  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  glEnableVertexAttribArray(static_cast<GLuint>(m_aPositionLoc));
  glVertexAttribPointer(
      static_cast<GLuint>(m_aPositionLoc), m_componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(static_cast<GLuint>(m_aTexCoordsLoc));
  glVertexAttribPointer(
      static_cast<GLuint>(m_aTexCoordsLoc),
      m_componentsPerTexel,
      GL_FLOAT,
      GL_FALSE,
      0,
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
      reinterpret_cast<const GLvoid*>(static_cast<size_t>(m_numVertices * m_componentsPerVertex) *
                                      sizeof(GLfloat)));

  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(m_quadData.size() * sizeof(GLfloat)),
               m_quadData.data(),
               GL_STATIC_DRAW);

  glBindVertexArray(0);
#endif

  LogDebug(*m_goomLogger, "glGetError() = {}", glGetError());
  LogDebug(*m_goomLogger, "Finish InitGlVertexAttributes.");
}

auto GlRenderer::GLRendererImpl::CreateGlTexture() -> void
{
  LogDebug(*m_goomLogger, "Start CreateGlTexture.");
  LogDebug(*m_goomLogger, "glGetError() = {}", glGetError());

  glGenTextures(1, &m_textureId);
  if (0 == m_textureId)
  {
    throw std::runtime_error("Could not create GL texture.");
  }

  glClear(GL_COLOR_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, m_textureId);

  // Optional texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Required texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef HAS_GL
  //TODO(glk) Is this worth using?
  glGenerateMipmap(GL_TEXTURE_2D);
#endif

  static constexpr GLint LEVEL           = 0;
  static constexpr GLint BORDER          = 0;
  static constexpr const void* NULL_DATA = nullptr;
  glTexImage2D(GL_TEXTURE_2D,
               LEVEL,
               TEXTURE_SIZED_INTERNAL_FORMAT,
               static_cast<GLsizei>(m_textureBufferDimensions.width),
               static_cast<GLsizei>(m_textureBufferDimensions.height),
               BORDER,
               TEXTURE_FORMAT,
               TEXTURE_DATA_TYPE,
               NULL_DATA);

  glBindTexture(GL_TEXTURE_2D, 0);

#ifdef HAS_GL
  AllocateGlTextureBuffers();
#endif

  LogDebug(*m_goomLogger, "glGetError() = {}", glGetError());
  LogDebug(*m_goomLogger, "Finish CreateGlTexture.");
}

#ifdef HAS_GL
auto GlRenderer::GLRendererImpl::AllocateGlTextureBuffers() -> void
{
  if (not m_usePixelBufferObjects)
  {
    LogDebug(*m_goomLogger, "Not using pixel buffer objects.");
    return;
  }

  m_currentPboIndex = 0;

  glGenBuffers(NUM_PBOS, m_pboIds.data());

  for (auto i = 0U; i < NUM_PBOS; ++i)
  {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds.at(i));
    glBufferData(GL_PIXEL_UNPACK_BUFFER,
                 static_cast<GLsizeiptr>(m_textureBufferSize),
                 nullptr,
                 GL_STREAM_DRAW);

    m_pboMappedBuffer.at(i) =
        static_cast<PixelChannelType*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
    if (nullptr == m_pboMappedBuffer.at(i))
    {
      throw std::runtime_error(std_fmt::format("Could not allocate mapped buffer for pbo {}.", i));
    }

    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // release pointer to mapping buffer
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  }
}
#endif

auto GlRenderer::GLRendererImpl::Render() -> void
{
  //LogDebug(*m_goomLogger, "Start Render.");
  //LogDebug(*m_goomLogger, "glGetError() = {}", glGetError());

  InitVertexAttributes();
  DrawGlTexture();
  DeinitVertexAttributes();

  //LogDebug(*m_goomLogger, "glGetError() = {}", glGetError());
  //LogDebug(*m_goomLogger, "Finish Render.");
}

inline auto GlRenderer::GLRendererImpl::InitVertexAttributes() const -> void
{
#ifdef HAS_GL
  glBindVertexArray(m_vaoObject);
#else
  glVertexAttribPointer(
      static_cast<GLuint>(m_aPositionLoc), 2, GL_FLOAT, GL_FALSE, 0, m_quadData.data());
  glEnableVertexAttribArray(static_cast<GLuint>(m_aPositionLoc));
  glVertexAttribPointer(static_cast<GLuint>(m_aTexCoordsLoc),
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        0,
                        m_quadData.data() + (m_numVertices * m_componentsPerVertex));
  glEnableVertexAttribArray(static_cast<GLuint>(m_aTexCoordsLoc));
#endif
}

#ifdef HAS_GL
inline auto GlRenderer::GLRendererImpl::DeinitVertexAttributes() -> void
{
  glBindVertexArray(0);
}
#else
inline auto GlRenderer::GLRendererImpl::DeinitVertexAttributes() const -> void
{
  glDisableVertexAttribArray(static_cast<GLuint>(m_aPositionLoc));
  glDisableVertexAttribArray(static_cast<GLuint>(m_aTexCoordsLoc));
}
#endif

inline auto GlRenderer::GLRendererImpl::DrawGlTexture() -> void
{
  // Setup texture.
  glDisable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_textureId);

  EnableShader();

  UpdateGlTextureBuffer();

  glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_TRIANGLES * NUM_VERTICES_IN_TRIANGLE);

  DisableShader();

  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
}

inline auto GlRenderer::GLRendererImpl::UpdateGlTextureBuffer() -> void
{
  if (nullptr == m_pixelBuffer)
  {
    return;
  }

#ifdef HAS_GL
  if (m_usePixelBufferObjects)
  {
    UpdatePboGlTextureBuffer();
  }
  else
#endif
  {
    UpdateNormalGlTextureBuffer();
  }

  UpdateShader();
}

#ifdef HAS_GL
inline auto GlRenderer::GLRendererImpl::UpdatePboGlTextureBuffer() -> void
{
  //LogDebug(*m_goomLogger, "Send to texture.");
  m_currentPboIndex = (m_currentPboIndex + 1) % NUM_PBOS;
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds.at(m_currentPboIndex));

  static constexpr GLint LEVEL             = 0;
  static constexpr GLint X_OFFSET          = 0;
  static constexpr GLint Y_OFFSET          = 0;
  static constexpr const void* NULL_PIXELS = nullptr;
  glTexSubImage2D(GL_TEXTURE_2D,
                  LEVEL,
                  X_OFFSET,
                  Y_OFFSET,
                  static_cast<GLsizei>(m_textureBufferDimensions.width),
                  static_cast<GLsizei>(m_textureBufferDimensions.height),
                  TEXTURE_FORMAT,
                  TEXTURE_DATA_TYPE,
                  NULL_PIXELS);

  // Update data directly on the mapped buffer.
  const size_t nextPboIndex = (m_currentPboIndex + 1) % NUM_PBOS;
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds.at(nextPboIndex));
  std::memcpy(m_pboMappedBuffer.at(nextPboIndex), m_pixelBuffer, m_textureBufferSize);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}
#endif

inline auto GlRenderer::GLRendererImpl::UpdateNormalGlTextureBuffer() -> void
{
  static constexpr GLint LEVEL    = 0;
  static constexpr GLint X_OFFSET = 0;
  static constexpr GLint Y_OFFSET = 0;
  glTexSubImage2D(GL_TEXTURE_2D,
                  LEVEL,
                  X_OFFSET,
                  Y_OFFSET,
                  static_cast<GLsizei>(m_textureBufferDimensions.width),
                  static_cast<GLsizei>(m_textureBufferDimensions.height),
                  TEXTURE_FORMAT,
                  TEXTURE_DATA_TYPE,
                  m_pixelBuffer);
}

inline auto GlRenderer::GLRendererImpl::EnableShader() -> void
{
  m_glShader->EnableShader();
}

inline auto GlRenderer::GLRendererImpl::UpdateShader() -> void
{
  m_glShader->UpdateShader();
}

inline auto GlRenderer::GLRendererImpl::DisableShader() -> void
{
  m_glShader->DisableShader();
}

inline auto GlRenderer::GLRendererImpl::GetShaderHandle() const noexcept -> GLuint
{
  return m_glShader->GetShaderHandle();
}

inline auto GlRenderer::GLRendererImpl::CompileAndLinkShaders() -> void
{
  m_glShader->CreateGlShaders();

  InitGlShaderVariables();
}

inline auto GlRenderer::GLRendererImpl::InitGlShaderVariables() -> void
{
  m_aPositionLoc  = glGetAttribLocation(GetShaderHandle(), "in_position");
  m_aTexCoordsLoc = glGetAttribLocation(GetShaderHandle(), "in_texCoords");
}

} // namespace GOOM

#if defined(__apple_build_version__) || __clang_major__ == 14
#pragma clang diagnostic pop
#endif
