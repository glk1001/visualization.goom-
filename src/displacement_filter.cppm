module;

//#define SAVE_FILTER_BUFFERS

#include "gl_call.h"
#include "goom/frame_data.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/goom_logger.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"

#ifdef IS_KODI_BUILD
#include <kodi/gui/gl/GL.h> // NOLINT: False positive??
#else
#include "glad/glad.h"
#endif
#include <GL/glext.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <functional>
#include <glm/ext/vector_float4.hpp>
#include <span>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef SAVE_FILTER_BUFFERS
#include "goom_graphic.h"

#include <format_23>
#include <fstream>
#endif

namespace GOOM
{
class GoomLogger;
}

export module Goom.GoomVisualization:DisplacementFilter;

import :Gl2dTextures;
import :GlRenderTypes;
import :GlslProgram;
import :GlslShaderFile;
import :Scene;

namespace GOOM::OPENGL
{

class DisplacementFilter : public IScene
{
public:
  static constexpr auto NUM_PBOS = 3U;
  using FilterPosBuffersXY       = GOOM::Point2dFlt;

  DisplacementFilter(GOOM::GoomLogger& goomLogger,
                     const std::string& shaderDir,
                     const GOOM::TextureBufferDimensions& textureBufferDimensions) noexcept;

  auto InitScene() -> void override;
  auto Resize(const GOOM::WindowDimensions& windowDimensions) noexcept -> void override;
  auto DestroyScene() noexcept -> void override;

  [[nodiscard]] auto GetShaderDir() const noexcept -> const std::string&;
  [[nodiscard]] auto GetBrightnessAdjust() const noexcept -> float;
  auto SetBrightnessAdjust(float value) -> void;

  auto Render() -> void override;

  [[nodiscard]] auto GetFrameData(size_t pboIndex) noexcept -> GOOM::FrameData&;
  auto UpdateFrameData(size_t pboIndex) noexcept -> void;
  using RequestNextFrameDataFunc = std::function<bool()>;
  auto SetRequestNextFrameDataFunc(
      const RequestNextFrameDataFunc& requestNextFrameDataFunc) noexcept -> void;
  using ReleaseCurrentFrameDataFunc = std::function<void(size_t slot)>;
  auto SetReleaseCurrentFrameDataFunc(
      const ReleaseCurrentFrameDataFunc& releaseCurrentFrameDataFunc) noexcept -> void;

protected:
  static constexpr auto DEFAULT_GAMMA = 1.5F;

  static constexpr auto* UNIFORM_LERP_FACTOR           = "u_lerpFactor";
  static constexpr auto* UNIFORM_BRIGHTNESS            = "u_brightness";
  static constexpr auto* UNIFORM_BRIGHTNESS_ADJUST     = "u_brightnessAdjust";
  static constexpr auto* UNIFORM_HUE_SHIFT             = "u_hueShift";
  static constexpr auto* UNIFORM_CHROMA_FACTOR         = "u_chromaFactor";
  static constexpr auto* UNIFORM_BASE_COLOR_MULTIPLIER = "u_baseColorMultiplier";
  static constexpr auto* UNIFORM_LUMINANCE_PARAMS      = "u_params";
  static constexpr auto* UNIFORM_GAMMA                 = "u_gamma";
  static constexpr auto* UNIFORM_RESET_SRCE_FILTER_POS = "u_resetSrceFilterPosBuffers";
  static constexpr auto* UNIFORM_POS1_POS2_MIX_FREQ    = "u_pos1Pos2MixFreq";
  static constexpr auto* UNIFORM_TIME                  = "u_time";

  // IMPORTANT - To make proper use of HDR (which is why we're using RGBA16), we
  //             must use a floating point internal format.
  static constexpr auto FILTER_BUFF_TEX_INTERNAL_FORMAT = GL_RGBA16F;
  static constexpr auto FILTER_POS_TEX_INTERNAL_FORMAT  = GL_RG32F;
  static constexpr auto IMAGE_TEX_INTERNAL_FORMAT       = FILTER_BUFF_TEX_INTERNAL_FORMAT;

  static constexpr auto FILTER_BUFF_TEX_FORMAT = GL_RGBA;
  static constexpr auto FILTER_POS_TEX_FORMAT  = GL_RG;
  static constexpr auto IMAGE_TEX_FORMAT       = FILTER_BUFF_TEX_FORMAT;

  // Following must match 'filterSrcePos' types in FrameData.
  static constexpr auto FILTER_BUFF_TEX_PIXEL_TYPE = GL_UNSIGNED_SHORT;
  static constexpr auto FILTER_POS_TEX_PIXEL_TYPE  = GL_FLOAT;
  static constexpr auto IMAGE_TEX_PIXEL_TYPE       = FILTER_BUFF_TEX_PIXEL_TYPE;

  [[nodiscard]] auto GetBuffSize() const noexcept -> size_t;
  [[nodiscard]] auto GetCurrentFrameData() const noexcept -> const GOOM::FrameData&;
  auto BindFilterBuff3Texture() noexcept -> void;
  [[nodiscard]] auto GetLumAverage() const noexcept -> float;

  static constexpr auto PASS1_VERTEX_SHADER   = "filter.vs";
  static constexpr auto PASS1_FRAGMENT_SHADER = "pass1_update_filter_buff1_and_buff3.fs";
  virtual auto Pass1UpdateFilterBuff1AndBuff3() noexcept -> void;

private:
  GOOM::GoomLogger* m_goomLogger;
  std::string m_shaderDir;
  size_t m_buffSize;
  float m_aspectRatio;
  float m_brightnessAdjust = 1.0F;
  GLuint m_renderToTextureFbo{};
  GLuint m_renderTextureName{};
  bool m_receivedFrameData = false;
  GLsync m_renderSync{};
  auto DoTheDraw() const noexcept -> void;
  auto WaitForRenderSync() noexcept -> void;

  size_t m_currentPboIndex = 0U;
  std::vector<FrameData> m_frameDataArray;
  auto InitFrameDataArrayPointers(std::vector<FrameData>& frameDataArray) noexcept -> void;
  auto InitFrameDataArray() noexcept -> void;
  auto InitFrameDataArrayToGl() noexcept -> void;
  static auto InitMiscData(MiscData& miscData) noexcept -> void;
  static auto InitImageArrays(ImageArrays& imageArrays) noexcept -> void;
  auto InitFilterPosArrays(FilterPosArrays& filterPosArrays) noexcept -> void;

  auto CopyTextureData(GLuint srceTextureName, GLuint destTextureName) const noexcept -> void;

  GLuint m_fsQuad{};
  static constexpr GLuint COMPONENTS_PER_VERTEX     = 2;
  static constexpr int32_t NUM_VERTICES_IN_TRIANGLE = 3;
  static constexpr int32_t NUM_TRIANGLES            = 2;
  static constexpr int32_t NUM_VERTICES             = NUM_TRIANGLES * NUM_VERTICES_IN_TRIANGLE;

  auto CompileAndLinkShaders() -> void;
  using ShaderMacros = std::unordered_map<std::string, std::string>;
  static auto CompileShaderFile(GlslProgram& program,
                                const std::string& filepath,
                                const ShaderMacros& shaderMacros) -> void;
  [[nodiscard]] auto GetShaderFilepath(const std::string& filename) const noexcept -> std::string;
  auto SetupRenderToTextureFBO() noexcept -> void;
  auto SetupScreenBuffers() noexcept -> void;
  static auto SetupGlSettings() -> void;
  auto SetupGlData() -> void;
  auto InitTextureBuffers() noexcept -> void;
  auto SetupGlLumComputeData() noexcept -> void;
  RequestNextFrameDataFunc m_requestNextFrameData{};
  ReleaseCurrentFrameDataFunc m_releaseCurrentFrameData{};
  auto UpdatePass1MiscDataToGl(size_t pboIndex) noexcept -> void;
  auto UpdatePass4MiscDataToGl(size_t pboIndex) noexcept -> void;
  auto UpdateCurrentDestFilterPosBufferToGl() noexcept -> void;
  auto UpdateImageBuffersToGl(size_t pboIndex) noexcept -> void;

  GlslProgram m_programPass1UpdateFilterBuff1AndBuff3;

  GlslProgram m_programPass2FilterBuff1LuminanceHistogram;
  static constexpr auto PASS2_SHADER = "pass2_lum_histogram.cs";
  auto Pass2FilterBuff3LuminanceHistogram() noexcept -> void;

  GlslProgram m_programPass3FilterBuff1LuminanceAverage;
  static constexpr auto PASS3_SHADER = "pass3_lum_avg.cs";
  auto Pass3FilterBuff3LuminanceAverage() noexcept -> void;

  GlslProgram m_programPass4ResetFilterBuff2AndOutputBuff3;
  static constexpr auto PASS4_VERTEX_SHADER   = "filter.vs";
  static constexpr auto PASS4_FRAGMENT_SHADER = "pass4_reset_filter_buff2_and_output_buff3.fs";
  auto Pass4UpdateFilterBuff2AndOutputBuff3() noexcept -> void;

  auto Pass5OutputToScreen() noexcept -> void;

  static constexpr auto NUM_FILTER_BUFF_TEXTURES  = 1;
  static constexpr auto FILTER_BUFF1_TEX_LOCATION = 0;
  static constexpr auto FILTER_BUFF2_TEX_LOCATION =
      FILTER_BUFF1_TEX_LOCATION + NUM_FILTER_BUFF_TEXTURES;
  static constexpr auto FILTER_BUFF3_TEX_LOCATION =
      FILTER_BUFF2_TEX_LOCATION + NUM_FILTER_BUFF_TEXTURES;

  // NOTE: FILTER_SRCE_POS and FILTER_DEST_POS have two textures each
  static constexpr auto NUM_FILTER_POS_TEXTURES = 2;
  static constexpr auto FILTER_SRCE_POS_TEX_LOCATION =
      FILTER_BUFF3_TEX_LOCATION + NUM_FILTER_BUFF_TEXTURES;
  static constexpr auto FILTER_DEST_POS_TEX_LOCATION =
      FILTER_SRCE_POS_TEX_LOCATION + NUM_FILTER_POS_TEXTURES;

  static constexpr auto NUM_IMAGE_TEXTURES = 1;
  static constexpr auto MAIN_IMAGE_TEX_LOCATION =
      FILTER_DEST_POS_TEX_LOCATION + NUM_FILTER_POS_TEXTURES;
  static constexpr auto LOW_IMAGE_TEX_LOCATION = MAIN_IMAGE_TEX_LOCATION + NUM_IMAGE_TEXTURES;

  static constexpr auto LUM_AVG_TEX_LOCATION = LOW_IMAGE_TEX_LOCATION + NUM_IMAGE_TEXTURES;

  static constexpr auto NULL_TEXTURE_NAME            = "";
  static constexpr auto FILTER_BUFF2_TEX_SHADER_NAME = "tex_filterBuff2";
  static constexpr auto MAIN_IMAGE_TEX_SHADER_NAME   = "tex_mainImage";
  static constexpr auto LOW_IMAGE_TEX_SHADER_NAME    = "tex_lowImage";

  static constexpr auto NULL_IMAGE_UNIT             = -1;
  static constexpr auto FILTER_BUFF1_IMAGE_UNIT     = 0;
  static constexpr auto FILTER_BUFF2_IMAGE_UNIT     = 1;
  static constexpr auto FILTER_BUFF3_IMAGE_UNIT     = 2;
  static constexpr auto LUM_AVG_IMAGE_UNIT          = 3;
  static constexpr auto FILTER_SRCE_POS_IMAGE_UNITS = std::array{4, 5};
  static constexpr auto FILTER_DEST_POS_IMAGE_UNITS = std::array{6, 7};

  GLuint m_histogramBufferName{};
  static constexpr auto HISTOGRAM_BUFFER_LENGTH    = 256;
  static constexpr auto LUM_AVG_GROUP_SIZE         = 256;
  static constexpr auto LUM_HISTOGRAM_BUFFER_INDEX = 3;
  static constexpr auto LUM_AVG_TEX_UNIT           = GL_TEXTURE0 + LUM_AVG_TEX_LOCATION;
  auto SetLumHistogramParams() noexcept -> void;
  auto SetupGlLumHistogramBuffer() noexcept -> void;
  GLuint m_lumAverageDataTextureName{};
  auto SetLumAverageParams(float frameTime) noexcept -> void;
  auto SetupGlLumAverageData() noexcept -> void;

  struct GlFilterPosBuffers
  {
    Gl2DTexture<FilterPosBuffersXY,
                NUM_FILTER_POS_TEXTURES,
                FILTER_SRCE_POS_TEX_LOCATION,
                FILTER_POS_TEX_FORMAT,
                FILTER_POS_TEX_INTERNAL_FORMAT,
                FILTER_POS_TEX_PIXEL_TYPE,
                0>
        filterSrcePosTexture{};
    Gl2DTexture<FilterPosBuffersXY,
                NUM_FILTER_POS_TEXTURES,
                FILTER_DEST_POS_TEX_LOCATION,
                FILTER_POS_TEX_FORMAT,
                FILTER_POS_TEX_INTERNAL_FORMAT,
                FILTER_POS_TEX_PIXEL_TYPE,
                NUM_PBOS>
        filterDestPosTexture{};
    size_t numActiveTextures         = NUM_FILTER_POS_TEXTURES;
    size_t currentActiveTextureIndex = 0;
  };
  auto RotateCurrentFilterPosTextureIndex() noexcept -> void;
  GlFilterPosBuffers m_glFilterPosBuffers{};
  auto SetupGlFilterPosBuffers() -> void;

  struct GlFilterBuffers
  {
    Gl2DTexture<GOOM::PixelIntType,
                NUM_FILTER_BUFF_TEXTURES,
                FILTER_BUFF1_TEX_LOCATION,
                FILTER_BUFF_TEX_FORMAT,
                FILTER_BUFF_TEX_INTERNAL_FORMAT,
                FILTER_BUFF_TEX_PIXEL_TYPE,
                0>
        filterBuff1Texture{};
    Gl2DTexture<GOOM::PixelIntType,
                NUM_FILTER_BUFF_TEXTURES,
                FILTER_BUFF2_TEX_LOCATION,
                FILTER_BUFF_TEX_FORMAT,
                FILTER_BUFF_TEX_INTERNAL_FORMAT,
                FILTER_BUFF_TEX_PIXEL_TYPE,
                0>
        filterBuff2Texture{};
    Gl2DTexture<GOOM::PixelIntType,
                NUM_FILTER_BUFF_TEXTURES,
                FILTER_BUFF3_TEX_LOCATION,
                FILTER_BUFF_TEX_FORMAT,
                FILTER_BUFF_TEX_INTERNAL_FORMAT,
                FILTER_BUFF_TEX_PIXEL_TYPE,
                0>
        filterBuff3Texture{};
  };
  GlFilterBuffers m_glFilterBuffers{};
  auto SetupGlFilterBuffers() -> void;
  auto BindGlFilterBuffer2() noexcept -> void;

  struct GlImageBuffers
  {
    Gl2DTexture<GOOM::Pixel,
                NUM_IMAGE_TEXTURES,
                MAIN_IMAGE_TEX_LOCATION,
                IMAGE_TEX_FORMAT,
                IMAGE_TEX_INTERNAL_FORMAT,
                IMAGE_TEX_PIXEL_TYPE,
                NUM_PBOS>
        mainImageTexture{};
    Gl2DTexture<GOOM::Pixel,
                NUM_IMAGE_TEXTURES,
                LOW_IMAGE_TEX_LOCATION,
                IMAGE_TEX_FORMAT,
                IMAGE_TEX_INTERNAL_FORMAT,
                IMAGE_TEX_PIXEL_TYPE,
                NUM_PBOS>
        lowImageTexture{};
  };
  GlImageBuffers m_glImageBuffers{};
  auto SetupGlImageBuffers() -> void;
  auto BindGlImageBuffers() noexcept -> void;

#ifdef SAVE_FILTER_BUFFERS
  uint32_t m_pass1SaveNum = 0U;
  uint32_t m_pass4SaveNum = 0U;
  auto SaveGlBuffersAfterPass1() -> void;
  auto SaveGlBuffersAfterPass4() -> void;

  auto SaveFilterBuffersAfterPass1() -> void;
  auto SaveFilterPosBuffersAfterPass1() -> void;
  auto SaveFilterBuffersAfterPass4() -> void;

  auto SavePixelBuffer(const std::string& filename,
                       std::span<GOOM::Pixel> buffer,
                       float lumAverage = 0.0F) const -> void;
  auto SaveFilterPosBuffer(const std::string& filename, uint32_t textureIndex) -> void;
  auto SaveFilterPosBuffer(const std::string& filename, std::span<FilterPosBuffersXY> buffer) const
      -> void;
#endif
};

inline auto DisplacementFilter::GetShaderDir() const noexcept -> const std::string&
{
  return m_shaderDir;
}

inline auto DisplacementFilter::GetFrameData(const size_t pboIndex) noexcept -> GOOM::FrameData&
{
  return m_frameDataArray.at(pboIndex);
}

inline auto DisplacementFilter::GetBrightnessAdjust() const noexcept -> float
{
  return m_brightnessAdjust;
}

inline auto DisplacementFilter::SetBrightnessAdjust(const float value) -> void
{
  m_brightnessAdjust = value;
}

inline auto DisplacementFilter::SetRequestNextFrameDataFunc(
    const RequestNextFrameDataFunc& requestNextFrameDataFunc) noexcept -> void
{
  m_requestNextFrameData = requestNextFrameDataFunc;
}

inline auto DisplacementFilter::SetReleaseCurrentFrameDataFunc(
    const ReleaseCurrentFrameDataFunc& releaseCurrentFrameDataFunc) noexcept -> void
{
  m_releaseCurrentFrameData = releaseCurrentFrameDataFunc;
}

inline auto DisplacementFilter::GetBuffSize() const noexcept -> size_t
{
  return m_buffSize;
}

inline auto DisplacementFilter::GetCurrentFrameData() const noexcept -> const GOOM::FrameData&
{
  return m_frameDataArray.at(m_currentPboIndex);
}

inline auto DisplacementFilter::BindFilterBuff3Texture() noexcept -> void
{
  m_glFilterBuffers.filterBuff3Texture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
}

} // namespace GOOM::OPENGL

namespace fs = std::filesystem;

// TODO(glk) - Need to pass goomLogger
//std_fmt::println("{}", __LINE__);

namespace GOOM::OPENGL
{

using GOOM::GoomLogger;
//using GOOM::FILTER_FX::NormalizedCoords;

namespace
{
auto CopyBuffer(const std::span<const Point2dFlt> srce, std::span<Point2dFlt> dest) noexcept -> void
{
  std::copy(srce.begin(), srce.end(), dest.begin());
}

// TODO(glk) - Move this into goom filters?
auto InitFilterPosBuffer(const Dimensions& dimensions, std::span<Point2dFlt> tranBufferFlt) noexcept
    -> void
{
  Expects(dimensions.GetSize() == tranBufferFlt.size());

  static constexpr auto MIN_COORD   = MIN_NORMALIZED_COORD;
  static constexpr auto COORD_WIDTH = NORMALIZED_COORD_WIDTH;
  const float xRatioScreenToNormalizedCoord =
      COORD_WIDTH / static_cast<float>(dimensions.GetWidth());
  const float yRatioScreenToNormalizedCoord =
      COORD_WIDTH / static_cast<float>(dimensions.GetWidth());

  const auto getNormalizedCoords = [&xRatioScreenToNormalizedCoord, &yRatioScreenToNormalizedCoord](
                                       const float x, const float y) noexcept -> Point2dFlt
  {
    return {MIN_COORD + (xRatioScreenToNormalizedCoord * x),
            MIN_COORD + (yRatioScreenToNormalizedCoord * y)};
  };

  for (auto y = 0U; y < dimensions.GetHeight(); ++y)
  {
    const auto yIndex = static_cast<size_t>(y) * static_cast<size_t>(dimensions.GetWidth());
    for (auto x = 0U; x < dimensions.GetWidth(); ++x)
    {
      const auto index = yIndex + static_cast<size_t>(x);

      const auto identityXY =
          getNormalizedCoords(0.5F + static_cast<float>(x), 0.5F + static_cast<float>(y));

      tranBufferFlt[index] = identityXY;
    }
  }
}

} // namespace

DisplacementFilter::DisplacementFilter(
    GoomLogger& goomLogger,
    const std::string& shaderDir,
    const TextureBufferDimensions& textureBufferDimensions) noexcept
  : IScene{textureBufferDimensions},
    m_goomLogger{&goomLogger},
    m_shaderDir{shaderDir},
    m_buffSize{static_cast<size_t>(GetWidth()) * static_cast<size_t>(GetHeight())},
    m_aspectRatio{static_cast<float>(GetWidth()) / static_cast<float>(GetHeight())},
    m_frameDataArray(NUM_PBOS)
{
}

auto DisplacementFilter::InitScene() -> void
{
  CompileAndLinkShaders();

  SetupRenderToTextureFBO();

  SetupScreenBuffers();

  SetupGlData();

  SetupGlLumComputeData();

  InitTextureBuffers();

  InitFrameDataArray();

  InitFrameDataArrayToGl();
}

auto DisplacementFilter::DestroyScene() noexcept -> void
{
  GlCall(glDeleteTextures(1, &m_renderTextureName));
  GlCall(glDeleteFramebuffers(1, &m_renderToTextureFbo));

  m_glFilterPosBuffers.filterSrcePosTexture.DeleteBuffers();
  m_glFilterPosBuffers.filterDestPosTexture.DeleteBuffers();
  m_glImageBuffers.mainImageTexture.DeleteBuffers();
  m_glImageBuffers.lowImageTexture.DeleteBuffers();
  m_glFilterBuffers.filterBuff1Texture.DeleteBuffers();
  m_glFilterBuffers.filterBuff2Texture.DeleteBuffers();
  m_glFilterBuffers.filterBuff3Texture.DeleteBuffers();

  m_programPass1UpdateFilterBuff1AndBuff3.DeleteProgram();
  m_programPass2FilterBuff1LuminanceHistogram.DeleteProgram();
  m_programPass3FilterBuff1LuminanceAverage.DeleteProgram();
  m_programPass4ResetFilterBuff2AndOutputBuff3.DeleteProgram();
}

auto DisplacementFilter::InitFrameDataArray() noexcept -> void
{
  for (auto& frameData : m_frameDataArray)
  {
    InitMiscData(frameData.miscData);
    InitImageArrays(frameData.imageArrays);
    InitFilterPosArrays(frameData.filterPosArrays);
  }
}

auto DisplacementFilter::InitMiscData(GOOM::MiscData& miscData) noexcept -> void
{
  miscData.brightness          = 1.0F;
  miscData.chromaFactor        = 1.0F;
  miscData.baseColorMultiplier = 1.0F;
  miscData.gamma               = DEFAULT_GAMMA;
}

auto DisplacementFilter::InitImageArrays(GOOM::ImageArrays& imageArrays) noexcept -> void
{
  imageArrays.mainImagePixelBufferNeedsUpdating = false;
  imageArrays.lowImagePixelBufferNeedsUpdating  = false;
}

auto DisplacementFilter::InitFilterPosArrays(GOOM::FilterPosArrays& filterPosArrays) noexcept
    -> void
{
  filterPosArrays.filterPosBuffersLerpFactor = 0.0F;

  filterPosArrays.filterPos1Pos2FreqMixFreq  = FilterPosArrays::DEFAULT_POS1_POS2_MIX_FREQ;
  filterPosArrays.filterDestPosNeedsUpdating = false;

  InitFilterPosBuffer({static_cast<uint32_t>(GetWidth()), static_cast<uint32_t>(GetHeight())},
                      m_glFilterPosBuffers.filterDestPosTexture.GetMappedBuffer(0));

  for (auto i = 1U; i < NUM_PBOS; ++i)
  {
    CopyBuffer(m_glFilterPosBuffers.filterDestPosTexture.GetMappedBuffer(0),
               m_glFilterPosBuffers.filterDestPosTexture.GetMappedBuffer(i));
  }
}

auto DisplacementFilter::InitFrameDataArrayToGl() noexcept -> void
{
  for (auto i = 0U; i < NUM_FILTER_POS_TEXTURES; ++i)
  {
    m_glFilterPosBuffers.filterDestPosTexture.CopyMappedBufferToTexture(0, i);

    // Make sure copy buffer to texture has completed before using the texture.
    glFinish();

    CopyTextureData(m_glFilterPosBuffers.filterDestPosTexture.GetTextureName(i),
                    m_glFilterPosBuffers.filterSrcePosTexture.GetTextureName(i));
  }
}

auto DisplacementFilter::Resize(const WindowDimensions& windowDimensions) noexcept -> void
{
  SetFramebufferDimensions(windowDimensions);
}

auto DisplacementFilter::SetupRenderToTextureFBO() noexcept -> void
{
  // Generate and bind the FBO.
  GlCall(glGenFramebuffers(1, &m_renderToTextureFbo));
  GlCall(glBindFramebuffer(GL_FRAMEBUFFER, m_renderToTextureFbo));

  // Create the texture object.
  GlCall(glGenTextures(1, &m_renderTextureName));
  GlCall(glBindTexture(GL_TEXTURE_2D, m_renderTextureName));
  GlCall(
      glTexStorage2D(GL_TEXTURE_2D, 1, FILTER_BUFF_TEX_INTERNAL_FORMAT, GetWidth(), GetHeight()));

  // Bind the texture to the FBO.
  GlCall(glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTextureName, 0));

  // Set the targets for the fragment output variables.
  const auto drawBuffers = std::array<GLenum, 1>{GL_COLOR_ATTACHMENT0};
  GlCall(glDrawBuffers(1, drawBuffers.data()));

  // Unbind the FBO, and revert to default framebuffer.
  GlCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

auto DisplacementFilter::SetupScreenBuffers() noexcept -> void
{
  // Setup the vertex and texture coordinate arrays for a full-screen quad (2 triangles).
  static constexpr auto X0 = -1.0F;
  static constexpr auto X1 = +1.0F;
  static constexpr auto Y0 = -1.0F;
  static constexpr auto Y1 = +1.0F;
  // Note: Larger Y at bottom of quad.
  static constexpr auto VERTICES =
      std::array<GLfloat, static_cast<size_t>(COMPONENTS_PER_VERTEX * NUM_VERTICES)>{
          X0,
          Y1, // bottom left
          X0,
          Y0, // top left
          X1,
          Y1, // bottom right
          X1,
          Y1, // bottom right
          X1,
          Y0, // top right
          X0,
          Y0, // top left
      };
  static constexpr auto TEX_COORDS =
      std::array<GLfloat, static_cast<size_t>(COMPONENTS_PER_VERTEX * NUM_VERTICES)>{
          0.0F,
          0.0F, // bottom left
          0.0F,
          1.0F, // top left
          1.0F,
          0.0F, // bottom right
          1.0F,
          0.0F, // bottom right
          1.0F,
          1.0F, // top right
          0.0F,
          1.0F, // top left
      };

  // Setup the vertex and texture array buffers.
  static constexpr auto NUM_ARRAY_BUFFERS = 2U;
  std::array<uint32_t, NUM_ARRAY_BUFFERS> handle{};
  GlCall(glGenBuffers(NUM_ARRAY_BUFFERS, handle.data()));

  GlCall(glBindBuffer(GL_ARRAY_BUFFER, handle[0]));
  GlCall(glBufferData(GL_ARRAY_BUFFER,
                      static_cast<size_t>(COMPONENTS_PER_VERTEX * NUM_VERTICES) * sizeof(float),
                      VERTICES.data(),
                      GL_STATIC_DRAW));

  GlCall(glBindBuffer(GL_ARRAY_BUFFER, handle[1]));
  GlCall(glBufferData(GL_ARRAY_BUFFER,
                      static_cast<size_t>(COMPONENTS_PER_VERTEX * NUM_VERTICES) * sizeof(float),
                      TEX_COORDS.data(),
                      GL_STATIC_DRAW));

  // TODO(glk) - Use 4.4 OpenGL - see cookbook
  // Setup the vertex and texture array objects.
  GlCall(glGenVertexArrays(1, &m_fsQuad));
  GlCall(glBindVertexArray(m_fsQuad));

  GlCall(glBindBuffer(GL_ARRAY_BUFFER, handle[0]));
  GlCall(glVertexAttribPointer(0, COMPONENTS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, nullptr));
  GlCall(glEnableVertexAttribArray(0)); // Vertex position

  GlCall(glBindBuffer(GL_ARRAY_BUFFER, handle[1]));
  GlCall(glVertexAttribPointer(1, COMPONENTS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, nullptr));
  GlCall(glEnableVertexAttribArray(1)); // Texture coordinates

  GlCall(glBindVertexArray(0));
}

auto DisplacementFilter::CompileAndLinkShaders() -> void
{
  const auto shaderMacros = std::unordered_map<std::string, std::string>{
      {    "FILTER_BUFF1_IMAGE_UNIT",           std::to_string(FILTER_BUFF1_IMAGE_UNIT)},
      {    "FILTER_BUFF2_IMAGE_UNIT",           std::to_string(FILTER_BUFF2_IMAGE_UNIT)},
      {    "FILTER_BUFF3_IMAGE_UNIT",           std::to_string(FILTER_BUFF3_IMAGE_UNIT)},
      {         "LUM_AVG_IMAGE_UNIT",                std::to_string(LUM_AVG_IMAGE_UNIT)},
      {"FILTER_SRCE_POS_IMAGE_UNIT1", std::to_string(FILTER_SRCE_POS_IMAGE_UNITS.at(0))},
      {"FILTER_SRCE_POS_IMAGE_UNIT2", std::to_string(FILTER_SRCE_POS_IMAGE_UNITS.at(1))},
      {"FILTER_DEST_POS_IMAGE_UNIT1", std::to_string(FILTER_DEST_POS_IMAGE_UNITS.at(0))},
      {"FILTER_DEST_POS_IMAGE_UNIT2", std::to_string(FILTER_DEST_POS_IMAGE_UNITS.at(1))},
      { "LUM_HISTOGRAM_BUFFER_INDEX",        std::to_string(LUM_HISTOGRAM_BUFFER_INDEX)},
      {                     "HEIGHT",                       std::to_string(GetHeight())},
      {               "ASPECT_RATIO",                     std::to_string(m_aspectRatio)},
      {       "FILTER_POS_MIN_COORD",              std::to_string(MIN_NORMALIZED_COORD)},
      {     "FILTER_POS_COORD_WIDTH",            std::to_string(NORMALIZED_COORD_WIDTH)},
  };

  try
  {
    CompileShaderFile(m_programPass1UpdateFilterBuff1AndBuff3,
                      GetShaderFilepath(PASS1_VERTEX_SHADER),
                      shaderMacros);
    CompileShaderFile(m_programPass1UpdateFilterBuff1AndBuff3,
                      GetShaderFilepath(PASS1_FRAGMENT_SHADER),
                      shaderMacros);
    m_programPass1UpdateFilterBuff1AndBuff3.LinkShader();

    CompileShaderFile(
        m_programPass2FilterBuff1LuminanceHistogram, GetShaderFilepath(PASS2_SHADER), shaderMacros);
    m_programPass2FilterBuff1LuminanceHistogram.LinkShader();

    CompileShaderFile(
        m_programPass3FilterBuff1LuminanceAverage, GetShaderFilepath(PASS3_SHADER), shaderMacros);
    m_programPass3FilterBuff1LuminanceAverage.LinkShader();

    CompileShaderFile(m_programPass4ResetFilterBuff2AndOutputBuff3,
                      GetShaderFilepath(PASS4_VERTEX_SHADER),
                      shaderMacros);
    CompileShaderFile(m_programPass4ResetFilterBuff2AndOutputBuff3,
                      GetShaderFilepath(PASS4_FRAGMENT_SHADER),
                      shaderMacros);
    m_programPass4ResetFilterBuff2AndOutputBuff3.LinkShader();
  }
  catch (GlslProgramException& e)
  {
    throw std::runtime_error{std::string{"Compile fail: "} + e.what()};
  }
}

auto DisplacementFilter::GetShaderFilepath(const std::string& filename) const noexcept
    -> std::string
{
  return m_shaderDir + "/" + filename;
}

auto DisplacementFilter::CompileShaderFile(GlslProgram& program,
                                           const std::string& filepath,
                                           const ShaderMacros& shaderMacros) -> void
{
  static constexpr auto* INCLUDE_DIR = "";

  const auto tempDir        = fs::temp_directory_path().string();
  const auto filename       = fs::path(filepath).filename().string();
  const auto tempShaderFile = tempDir + "/" + filename;

  const auto shaderFile = GlslShaderFile{filepath, shaderMacros, INCLUDE_DIR};
  shaderFile.WriteToFile(tempShaderFile);
  if (not fs::exists(tempShaderFile))
  {
    throw std::runtime_error(std::format("Could not find output file '{}'", tempShaderFile));
  }

  program.CompileShader(tempShaderFile);
}

auto DisplacementFilter::SetupGlData() -> void
{
  SetupGlSettings();
  SetupGlFilterBuffers();
  SetupGlFilterPosBuffers();
  SetupGlImageBuffers();

  InitFrameDataArrayPointers(m_frameDataArray);
}

auto DisplacementFilter::SetupGlSettings() -> void
{
  glDisable(GL_BLEND);
}

auto DisplacementFilter::Render() -> void
{
  GlCall(glViewport(0, 0, GetWidth(), GetHeight()));

  Pass1UpdateFilterBuff1AndBuff3();

  Pass2FilterBuff3LuminanceHistogram();

  Pass3FilterBuff3LuminanceAverage();

  Pass4UpdateFilterBuff2AndOutputBuff3();

  Pass5OutputToScreen();

  WaitForRenderSync();

  GlCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  UpdateCurrentDestFilterPosBufferToGl();
}

auto DisplacementFilter::UpdateFrameData(const size_t pboIndex) noexcept -> void
{
  m_currentPboIndex = pboIndex;
}

auto DisplacementFilter::Pass1UpdateFilterBuff1AndBuff3() noexcept -> void
{
  m_receivedFrameData = m_requestNextFrameData();
  if (m_receivedFrameData)
  {
    m_renderSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  }

  UpdateImageBuffersToGl(m_currentPboIndex);

  m_programPass1UpdateFilterBuff1AndBuff3.Use();

  UpdatePass1MiscDataToGl(m_currentPboIndex);

  BindGlFilterBuffer2();
  BindGlImageBuffers();

  DoTheDraw();

#ifdef SAVE_FILTER_BUFFERS
  SaveGlBuffersAfterPass1();
#endif
}

auto DisplacementFilter::DoTheDraw() const noexcept -> void
{
  GlCall(glBindFramebuffer(GL_FRAMEBUFFER, m_renderToTextureFbo));
  GlCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  // Render the full-screen quad
  GlCall(glBindVertexArray(m_fsQuad));
  GlCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, NUM_VERTICES));

  GlCall(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
}

auto DisplacementFilter::WaitForRenderSync() noexcept -> void
{
  if (not m_receivedFrameData)
  {
    return;
  }

  static constexpr auto TIMEOUT_NANOSECONDS = 50 * 1000 * 1000;

  if (const auto result =
          glClientWaitSync(m_renderSync, GL_SYNC_FLUSH_COMMANDS_BIT, TIMEOUT_NANOSECONDS);
      GL_TIMEOUT_EXPIRED == result)
  {
    LogError(*m_goomLogger, "GL fence did not finish before timeout.");
  }
  else if (GL_WAIT_FAILED == result)
  {
    LogError(*m_goomLogger, "A GL fence error occurred.");
  }

  m_releaseCurrentFrameData(m_currentPboIndex);
}

auto DisplacementFilter::Pass4UpdateFilterBuff2AndOutputBuff3() noexcept -> void
{
  m_programPass4ResetFilterBuff2AndOutputBuff3.Use();

  UpdatePass4MiscDataToGl(m_currentPboIndex);

  GlCall(glBindFramebuffer(GL_FRAMEBUFFER, m_renderToTextureFbo));
  GlCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  // Render the full-screen quad
  GlCall(glBindVertexArray(m_fsQuad));
  GlCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, NUM_VERTICES));

  GlCall(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));

#ifdef SAVE_FILTER_BUFFERS
  SaveGlBuffersAfterPass4();
#endif
}

auto DisplacementFilter::Pass5OutputToScreen() noexcept -> void
{
  GlCall(glViewport(0, 0, GetFramebufferWidth(), GetFramebufferHeight()));

  GlCall(glBlitNamedFramebuffer(m_renderToTextureFbo,
                                0, // default framebuffer
                                0, // source rectangle
                                0,
                                GetWidth(),
                                GetHeight(),
                                0, // destination rectangle
                                0,
                                GetFramebufferWidth(),
                                GetFramebufferHeight(),
                                GL_COLOR_BUFFER_BIT,
                                GL_LINEAR));
}

auto DisplacementFilter::Pass2FilterBuff3LuminanceHistogram() noexcept -> void
{
  m_programPass2FilterBuff1LuminanceHistogram.Use();

  GlCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_histogramBufferName));
  GlCall(glClearBufferData(
      GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr));

  GlCall(glDispatchCompute(
      static_cast<GLuint>(GetWidth() / 16), static_cast<GLuint>(GetHeight() / 16), 1));
  GlCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
}

auto DisplacementFilter::Pass3FilterBuff3LuminanceAverage() noexcept -> void
{
  m_programPass3FilterBuff1LuminanceAverage.Use();

  GlCall(glDispatchCompute(LUM_AVG_GROUP_SIZE, 1, 1));
  GlCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
}

auto DisplacementFilter::SetupGlLumComputeData() noexcept -> void
{
  SetupGlLumHistogramBuffer();
  m_programPass2FilterBuff1LuminanceHistogram.Use();
  SetLumHistogramParams();

  SetupGlLumAverageData();
  m_programPass3FilterBuff1LuminanceAverage.Use();
  static constexpr auto LUM_AVG_TIME_COEFF = 2.0F;
  SetLumAverageParams(LUM_AVG_TIME_COEFF);
}

auto DisplacementFilter::SetLumHistogramParams() noexcept -> void
{
  static constexpr auto MIN_LOG_LUM = -9.0F;
  static constexpr auto MAX_LOG_LUM = +3.5F;
  static_assert((MAX_LOG_LUM - MIN_LOG_LUM) > 0.0F);

  const auto histogramParams = glm::vec4{MIN_LOG_LUM,
                                         1.0F / (MAX_LOG_LUM - MIN_LOG_LUM),
                                         static_cast<float>(GetWidth()),
                                         static_cast<float>(GetHeight())};

  m_programPass2FilterBuff1LuminanceHistogram.SetUniform(UNIFORM_LUMINANCE_PARAMS, histogramParams);
}

auto DisplacementFilter::SetLumAverageParams(const float frameTime) noexcept -> void
{
  static constexpr auto MIN_LOG_LUM = -8.0F;
  static constexpr auto MAX_LOG_LUM = +3.5F;

  static constexpr auto TAU = 1.1F;
  const auto timeCoeff      = std::clamp(1.0F - std::exp(-frameTime * TAU), 0.0F, 1.0F);

  const auto lumAverageParams =
      glm::vec4{MIN_LOG_LUM, MAX_LOG_LUM - MIN_LOG_LUM, timeCoeff, static_cast<float>(m_buffSize)};

  m_programPass3FilterBuff1LuminanceAverage.SetUniform(UNIFORM_LUMINANCE_PARAMS, lumAverageParams);
}

auto DisplacementFilter::GetLumAverage() const noexcept -> float
{
  GlCall(glBindTexture(GL_TEXTURE_2D, m_lumAverageDataTextureName));

  auto lumAverage = 0.0F;
  GlCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &lumAverage));

  return lumAverage;
}

auto DisplacementFilter::InitFrameDataArrayPointers(std::vector<FrameData>& frameDataArray) noexcept
    -> void
{
  for (auto i = 0U; i < NUM_PBOS; ++i)
  {
    frameDataArray.at(i).filterPosArrays.filterDestPos =
        m_glFilterPosBuffers.filterDestPosTexture.GetMappedBuffer(i);

    frameDataArray.at(i).imageArrays.mainImagePixelBuffer.SetPixelBuffer(
        m_glImageBuffers.mainImageTexture.GetMappedBuffer(i),
        Dimensions{static_cast<uint32_t>(GetWidth()), static_cast<uint32_t>(GetHeight())});
    frameDataArray.at(i).imageArrays.lowImagePixelBuffer.SetPixelBuffer(
        m_glImageBuffers.lowImageTexture.GetMappedBuffer(i),
        Dimensions{static_cast<uint32_t>(GetWidth()), static_cast<uint32_t>(GetHeight())});
  }
}

auto DisplacementFilter::UpdatePass1MiscDataToGl(const size_t pboIndex) noexcept -> void
{
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_LERP_FACTOR,
      m_frameDataArray.at(pboIndex).filterPosArrays.filterPosBuffersLerpFactor);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_RESET_SRCE_FILTER_POS,
      m_frameDataArray.at(pboIndex).filterPosArrays.filterDestPosNeedsUpdating);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_POS1_POS2_MIX_FREQ,
      m_frameDataArray.at(pboIndex).filterPosArrays.filterPos1Pos2FreqMixFreq);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_BASE_COLOR_MULTIPLIER, m_frameDataArray.at(pboIndex).miscData.baseColorMultiplier);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_TIME, static_cast<uint32_t>(m_frameDataArray.at(pboIndex).miscData.goomTime));
}

auto DisplacementFilter::UpdatePass4MiscDataToGl(const size_t pboIndex) noexcept -> void
{
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(
      UNIFORM_BRIGHTNESS, m_frameDataArray.at(pboIndex).miscData.brightness);
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(UNIFORM_BRIGHTNESS_ADJUST,
                                                          m_brightnessAdjust);
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(
      UNIFORM_HUE_SHIFT, m_frameDataArray.at(pboIndex).miscData.hueShift);
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(
      UNIFORM_CHROMA_FACTOR, m_frameDataArray.at(pboIndex).miscData.chromaFactor);
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(
      UNIFORM_GAMMA, m_frameDataArray.at(pboIndex).miscData.gamma);
}

auto DisplacementFilter::UpdateCurrentDestFilterPosBufferToGl() noexcept -> void
{
  if (not m_frameDataArray.at(m_currentPboIndex).filterPosArrays.filterDestPosNeedsUpdating)
  {
    return;
  }

  m_programPass1UpdateFilterBuff1AndBuff3.Use();
  m_glFilterPosBuffers.filterDestPosTexture.CopyMappedBufferToTexture(
      m_currentPboIndex, m_glFilterPosBuffers.currentActiveTextureIndex);

  RotateCurrentFilterPosTextureIndex();
}

auto DisplacementFilter::RotateCurrentFilterPosTextureIndex() noexcept -> void
{
  ++m_glFilterPosBuffers.currentActiveTextureIndex;
  if (m_glFilterPosBuffers.currentActiveTextureIndex >= m_glFilterPosBuffers.numActiveTextures)
  {
    m_glFilterPosBuffers.currentActiveTextureIndex = 0;
  }
}

auto DisplacementFilter::CopyTextureData(const GLuint srceTextureName,
                                         const GLuint destTextureName) const noexcept -> void
{
  GlCall(glCopyImageSubData(srceTextureName,
                            GL_TEXTURE_2D,
                            0,
                            0,
                            0,
                            0,
                            destTextureName,
                            GL_TEXTURE_2D,
                            0,
                            0,
                            0,
                            0,
                            GetWidth(),
                            GetHeight(),
                            1));
}

auto DisplacementFilter::UpdateImageBuffersToGl(const size_t pboIndex) noexcept -> void
{
  if (m_frameDataArray.at(pboIndex).imageArrays.mainImagePixelBufferNeedsUpdating)
  {
    m_glImageBuffers.mainImageTexture.CopyMappedBufferToTexture(pboIndex, 0);
  }
  if (m_frameDataArray.at(pboIndex).imageArrays.lowImagePixelBufferNeedsUpdating)
  {
    m_glImageBuffers.lowImageTexture.CopyMappedBufferToTexture(pboIndex, 0);
  }
}

auto DisplacementFilter::BindGlFilterBuffer2() noexcept -> void
{
  m_glFilterBuffers.filterBuff2Texture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
}

auto DisplacementFilter::BindGlImageBuffers() noexcept -> void
{
  m_glImageBuffers.mainImageTexture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
  m_glImageBuffers.lowImageTexture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
}

auto DisplacementFilter::SetupGlFilterBuffers() -> void
{
  m_glFilterBuffers.filterBuff1Texture.Setup(
      0, NULL_TEXTURE_NAME, FILTER_BUFF1_IMAGE_UNIT, GetWidth(), GetHeight());
  m_glFilterBuffers.filterBuff2Texture.Setup(
      0, FILTER_BUFF2_TEX_SHADER_NAME, FILTER_BUFF2_IMAGE_UNIT, GetWidth(), GetHeight());
  m_glFilterBuffers.filterBuff3Texture.Setup(
      0, NULL_TEXTURE_NAME, FILTER_BUFF3_IMAGE_UNIT, GetWidth(), GetHeight());
}

auto DisplacementFilter::SetupGlFilterPosBuffers() -> void
{
  for (auto i = 0U; i < NUM_FILTER_POS_TEXTURES; ++i)
  {
    m_glFilterPosBuffers.filterSrcePosTexture.Setup(
        i, NULL_TEXTURE_NAME, FILTER_SRCE_POS_IMAGE_UNITS.at(i), GetWidth(), GetHeight());
    m_glFilterPosBuffers.filterDestPosTexture.Setup(
        i, NULL_TEXTURE_NAME, FILTER_DEST_POS_IMAGE_UNITS.at(i), GetWidth(), GetHeight());
  }
}

auto DisplacementFilter::SetupGlImageBuffers() -> void
{
  m_glImageBuffers.mainImageTexture.Setup(
      0, MAIN_IMAGE_TEX_SHADER_NAME, NULL_IMAGE_UNIT, GetWidth(), GetHeight());
  m_glImageBuffers.lowImageTexture.Setup(
      0, LOW_IMAGE_TEX_SHADER_NAME, NULL_IMAGE_UNIT, GetWidth(), GetHeight());
}

auto DisplacementFilter::SetupGlLumHistogramBuffer() noexcept -> void
{
  GlCall(glGenBuffers(1, &m_histogramBufferName));
  GlCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_histogramBufferName));
  GlCall(glBufferData(GL_SHADER_STORAGE_BUFFER,
                      HISTOGRAM_BUFFER_LENGTH * sizeof(uint32_t),
                      nullptr,
                      GL_DYNAMIC_COPY));
  GlCall(glBindBufferBase(
      GL_SHADER_STORAGE_BUFFER, LUM_HISTOGRAM_BUFFER_INDEX, m_histogramBufferName));
  GlCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

auto DisplacementFilter::SetupGlLumAverageData() noexcept -> void
{
  GlCall(glGenTextures(1, &m_lumAverageDataTextureName));
  GlCall(glActiveTexture(LUM_AVG_TEX_UNIT));
  GlCall(glBindTexture(GL_TEXTURE_2D, m_lumAverageDataTextureName));
  GlCall(glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16F, 1, 1));
  GlCall(glBindImageTexture(
      LUM_AVG_IMAGE_UNIT, m_lumAverageDataTextureName, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R16F));

  const auto initialData = 0.5F;
  GlCall(glBindTexture(GL_TEXTURE_2D, m_lumAverageDataTextureName));
  GlCall(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED, GL_FLOAT, &initialData));
}

auto DisplacementFilter::InitTextureBuffers() noexcept -> void
{
  m_glFilterBuffers.filterBuff1Texture.ZeroTextures();
  m_glFilterBuffers.filterBuff2Texture.ZeroTextures();
  m_glFilterBuffers.filterBuff3Texture.ZeroTextures();

  m_glImageBuffers.mainImageTexture.ZeroTextures();
  m_glImageBuffers.lowImageTexture.ZeroTextures();

  GlCall(glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT));
}

#ifdef SAVE_FILTER_BUFFERS
// TODO(glk) - Use formatted GoomPoint2dBufferSaver
static constexpr auto SAVE_ROOT_DIR = "/home/greg/.kodi/filter_buffers";
//static constexpr auto SAVE_ROOT_DIR = "/home/greg/Prj/workdir/filter_buffers";

auto DisplacementFilter::SaveGlBuffersAfterPass1() -> void
{
  ++m_pass1SaveNum;

  SaveFilterBuffersAfterPass1();
  SaveFilterPosBuffersAfterPass1();
}

auto DisplacementFilter::SaveGlBuffersAfterPass4() -> void
{
  ++m_pass4SaveNum;

  SaveFilterBuffersAfterPass4();
}

auto DisplacementFilter::SaveFilterBuffersAfterPass1() -> void
{
  auto filterBuffer = std::vector<GOOM::Pixel>(m_buffSize);
  m_glFilterBuffers.filterBuff1Texture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
  GlCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, FILTER_BUFF_TEX_PIXEL_TYPE, filterBuffer.data()));

  const auto filename = std::format("{}/filter_buffer1_{:04d}.txt", SAVE_ROOT_DIR, m_pass1SaveNum);

  SavePixelBuffer(filename, filterBuffer);
}

auto DisplacementFilter::SaveFilterPosBuffersAfterPass1() -> void
{
  for (auto i = 0U; i < NUM_FILTER_POS_TEXTURES; ++i)
  {
    const auto filename =
        std::format("{}/filter_pos_buffer{}_{:04d}.txt", SAVE_ROOT_DIR, i, m_pass1SaveNum);

    SaveFilterPosBuffer(filename, i);
  }
}

auto DisplacementFilter::SaveFilterBuffersAfterPass4() -> void
{
  const auto lumAverage = GetLumAverage();

  auto filterBuffer = std::vector<GOOM::Pixel>(m_buffSize);
  m_glFilterBuffers.filterBuff3Texture.BindTextures(m_programPass4ResetFilterBuff2AndOutputBuff3);
  GlCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, FILTER_BUFF_TEX_PIXEL_TYPE, filterBuffer.data()));

  const auto filename = std::format("{}/filter_buffer3_{:04d}.txt", SAVE_ROOT_DIR, m_pass4SaveNum);

  SavePixelBuffer(filename, filterBuffer, lumAverage);
}

auto DisplacementFilter::SaveFilterPosBuffer(const std::string& filename,
                                             const uint32_t textureIndex) -> void
{
  auto filterBuffer = std::vector<FilterPosBuffersXY>(m_buffSize);
  m_glFilterPosBuffers.filterSrcePosTexture.BindTexture(m_programPass1UpdateFilterBuff1AndBuff3,
                                                        textureIndex);
  GlCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, FILTER_POS_TEX_PIXEL_TYPE, filterBuffer.data()));
  SaveFilterPosBuffer(filename, filterBuffer);
}

auto DisplacementFilter::SaveFilterPosBuffer(const std::string& filename,
                                             std::span<FilterPosBuffersXY> buffer) const -> void
{
  auto file = std::ofstream{filename};
  if (not file.good())
  {
    std_fmt::println(stderr, "ERROR: Could not open file '{}'.", filename);
    return;
  }

  auto index = 0U;
  for (auto y = 0; y < GetHeight(); ++y)
  {
    for (auto x = 0; x < GetWidth(); ++x)
    {
      const auto pos = buffer[index];
      file << std::format("[{:4d} {:4d}]  {:6.2f}, {:6.2f}\n", x, y, pos.x, pos.y);
    }
    ++index;
  }
}

auto DisplacementFilter::SavePixelBuffer(const std::string& filename,
                                         std::span<GOOM::Pixel> buffer,
                                         const float lumAverage) const -> void
{
  auto file = std::ofstream{filename};
  if (not file.good())
  {
    std_fmt::println(stderr, "ERROR: Could not open file '{}'.", filename);
    return;
  }

  const auto linearScale   = 0.18F; // MAYBE brightness ??
  const auto finalExposure = linearScale / (lumAverage + 0.0001F);
  file << std::format("LumAverage    = {:.3f}.\n", lumAverage);
  file << std::format("FinalExposure = {:.3f}.\n\n", finalExposure);

  auto index = 0U;
  for (auto y = 0; y < GetHeight(); ++y)
  {
    for (auto x = 0; x < GetWidth(); ++x)
    {
      const auto pixel = buffer[index];
      if (static constexpr auto CUT_OFF = 256U;
          (pixel.R() > CUT_OFF) or (pixel.G() > CUT_OFF) or (pixel.B() > CUT_OFF))
      {
        file << std::format("[{:4d} {:4d}]  {:6d}, {:6d}, {:6d}, {:6d}\n",
                            x,
                            y,
                            pixel.R(),
                            pixel.G(),
                            pixel.B(),
                            pixel.A());
      }
      ++index;
    }
  }
}
#endif

} // namespace GOOM::OPENGL
