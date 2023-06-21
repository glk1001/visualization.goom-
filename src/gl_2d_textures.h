#pragma once

#include "glsl_program.h"
#include "goom/goom_utils.h"

#include <cstdint>
#include <format>

namespace GOOM::OPENGL
{

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
class Gl2DTexture
{
public:
  Gl2DTexture() = default;

  auto Setup(const char* textureShaderName, int32_t textureWidth, int32_t textureHeight) -> void;

  [[nodiscard]] auto GetTextureName() const noexcept -> GLuint;
  [[nodiscard]] auto GetMappedBuffer(size_t pboIndex) noexcept -> CppTextureType*;

  auto ZeroTextureData() noexcept -> void;
  auto CopyMappedBufferToTexture(size_t pboIndex) noexcept -> void;
  auto BindTexture(GlslProgram& program) noexcept -> void;

private:
  static constexpr GLenum TEXTURE_UNIT = GL_TEXTURE0 + TextureLocation;

  const char* m_textureShaderName{};
  int32_t m_textureWidth{};
  int32_t m_textureHeight{};
  size_t m_buffSize{};
  GLuint m_textureName{};
  auto AllocateBuffers() -> void;

  struct PboBuffers
  {
    std::array<GLuint, NumPbos> ids{};
    std::array<CppTextureType*, NumPbos> mappedBuffers{};
  };
  PboBuffers m_pboBuffers{};
  auto AllocatePboBuffers() -> void;
  auto CopyPboBufferToTexture(size_t pboIndex) noexcept -> void;
};

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 TextureImageUint,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::Setup(const char* const textureShaderName,
                                 const int32_t textureWidth,
                                 const int32_t textureHeight) -> void
{
  m_textureShaderName = textureShaderName;
  m_textureWidth      = textureWidth;
  m_textureHeight     = textureHeight;
  m_buffSize          = static_cast<size_t>(m_textureWidth) * static_cast<size_t>(m_textureHeight);

  glGenTextures(1, &m_textureName);
  glActiveTexture(TEXTURE_UNIT);
  glBindTexture(GL_TEXTURE_2D, m_textureName);
  glTexStorage2D(GL_TEXTURE_2D, 1, TextureInternalFormat, m_textureWidth, m_textureHeight);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  if constexpr (TextureImageUint != -1)
  {
    glBindImageTexture(
        TextureImageUint, m_textureName, 0, GL_FALSE, 0, GL_READ_WRITE, TextureInternalFormat);
  }

  AllocateBuffers();
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 TextureImageUint,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::AllocateBuffers() -> void
{
  if constexpr (0 == NumPbos)
  {
    return;
  }

  AllocatePboBuffers();
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 TextureImageUint,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::BindTexture(GlslProgram& program) noexcept -> void
{
  program.SetUniform(m_textureShaderName, TextureLocation);

  glActiveTexture(TEXTURE_UNIT);
  glBindTexture(GL_TEXTURE_2D, m_textureName);
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
inline auto Gl2DTexture<CppTextureType,
                        TextureImageUint,
                        TextureLocation,
                        TextureFormat,
                        TextureInternalFormat,
                        TexturePixelType,
                        NumPbos>::GetTextureName() const noexcept -> GLuint
{
  return m_textureName;
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
inline auto Gl2DTexture<CppTextureType,
                        TextureImageUint,
                        TextureLocation,
                        TextureFormat,
                        TextureInternalFormat,
                        TexturePixelType,
                        NumPbos>::GetMappedBuffer(const size_t pboIndex) noexcept -> CppTextureType*
{
  return m_pboBuffers.mappedBuffers.at(pboIndex);
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
inline auto Gl2DTexture<CppTextureType,
                        TextureImageUint,
                        TextureLocation,
                        TextureFormat,
                        TextureInternalFormat,
                        TexturePixelType,
                        NumPbos>::ZeroTextureData() noexcept -> void
{
  glClearTexImage(m_textureName, 0, TextureFormat, TexturePixelType, nullptr);
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
inline auto Gl2DTexture<CppTextureType,
                        TextureImageUint,
                        TextureLocation,
                        TextureFormat,
                        TextureInternalFormat,
                        TexturePixelType,
                        NumPbos>::CopyMappedBufferToTexture(size_t pboIndex) noexcept -> void
{
  glBindTexture(GL_TEXTURE_2D, m_textureName);

  /**
  if constexpr (0 == NumPbos)
  {
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    m_textureWidth,
                    m_textureHeight,
                    TextureFormat,
                    TexturePixelType,
                    buffer);
    return;
  }
   **/

  CopyPboBufferToTexture(pboIndex);
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 TextureImageUint,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::AllocatePboBuffers() -> void
{
  glGenBuffers(NumPbos, m_pboBuffers.ids.data());

  for (auto i = 0U; i < NumPbos; ++i)
  {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboBuffers.ids.at(i));
    glBufferStorage(GL_PIXEL_UNPACK_BUFFER,
                    static_cast<GLsizeiptr>(m_buffSize * sizeof(CppTextureType)),
                    nullptr,
                    GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |
                        GL_MAP_COHERENT_BIT | GL_CLIENT_STORAGE_BIT);

    m_pboBuffers.mappedBuffers.at(i) = ptr_cast<CppTextureType*>(
        glMapBufferRange(GL_PIXEL_UNPACK_BUFFER,
                         0,
                         static_cast<GLsizeiptr>(m_buffSize * sizeof(CppTextureType)),
                         GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
    if (nullptr == m_pboBuffers.mappedBuffers.at(i))
    {
      throw std::runtime_error(std_fmt::format("Could not allocate mapped buffer for pbo {}.", i));
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  }
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 TextureImageUint,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::CopyPboBufferToTexture(const size_t pboIndex) noexcept -> void
{
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboBuffers.ids.at(pboIndex));

  static constexpr GLint LEVEL    = 0;
  static constexpr GLint X_OFFSET = 0;
  static constexpr GLint Y_OFFSET = 0;
  glTexSubImage2D(GL_TEXTURE_2D,
                  LEVEL,
                  X_OFFSET,
                  Y_OFFSET,
                  m_textureWidth,
                  m_textureHeight,
                  TextureFormat,
                  TexturePixelType,
                  nullptr);

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

} // namespace GOOM::OPENGL
