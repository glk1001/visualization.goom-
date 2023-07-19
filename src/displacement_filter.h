#pragma once

#include "gl_2d_textures.h"
#include "gl_render_types.h"
#include "glsl_program.h"
#include "goom/frame_data.h"
#include "goom/goom_graphic.h"
#include "scene.h"

#include <atomic>
#include <functional>
#include <span>
#include <string>
#include <vector>

namespace GOOM
{
class GoomLogger;
}

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
  auto DestroyScene() noexcept -> void;

  auto Render() -> void override;

  auto ClearFilterBuffers() noexcept -> void;

  [[nodiscard]] auto GetFrameData(size_t pboIndex) noexcept -> GOOM::FrameData&;
  auto UpdateFrameData(size_t pboIndex) noexcept -> void;
  using RequestNextFrameDataFunc = std::function<bool()>;
  auto SetRequestNextFrameDataFunc(
      const RequestNextFrameDataFunc& requestNextFrameDataFunc) noexcept -> void;
  using ReleaseCurrentFrameDataFunc = std::function<void(size_t slot)>;
  auto SetReleaseCurrentFrameDataFunc(
      const ReleaseCurrentFrameDataFunc& releaseCurrentFrameDataFunc) noexcept -> void;

protected:
  static constexpr auto* UNIFORM_LERP_FACTOR           = "u_lerpFactor";
  static constexpr auto* UNIFORM_BRIGHTNESS            = "u_brightness";
  static constexpr auto* UNIFORM_HUE_SHIFT             = "u_hueShift";
  static constexpr auto* UNIFORM_CHROMA_FACTOR         = "u_chromaFactor";
  static constexpr auto* UNIFORM_BASE_COLOR_MULTIPLIER = "u_baseColorMultiplier";
  static constexpr auto* UNIFORM_LUMINANCE_PARAMS      = "u_params";

  static constexpr auto FILTER_BUFF_TEX_INTERNAL_FORMAT = GL_RGBA16;
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

private:
  GOOM::GoomLogger* m_goomLogger;
  std::string m_shaderDir;
  size_t m_buffSize;
  float m_aspectRatio;
  GLuint m_renderToTextureFbo{};
  GLuint m_renderTextureName{};
  bool m_receivedFrameData = false;
  GLsync m_renderSync{};
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
  auto InitFilterBuffers() noexcept -> void;
  auto SetupGlLumComputeData() noexcept -> void;
  RequestNextFrameDataFunc m_requestNextFrameData{};
  ReleaseCurrentFrameDataFunc m_releaseCurrentFrameData{};
  auto UpdatePass1MiscDataToGl(size_t pboIndex) noexcept -> void;
  auto UpdatePass4MiscDataToGl(size_t pboIndex) noexcept -> void;
  auto UpdateSrceFilterPosBufferToGl(size_t pboIndex) noexcept -> void;
  auto UpdateDestFilterPosBufferToGl(size_t pboIndex) noexcept -> void;
  auto UpdateImageBuffersToGl(size_t pboIndex) noexcept -> void;

  GlslProgram m_programPass1UpdateFilterBuff1AndBuff3;
  static constexpr auto PASS1_VERTEX_SHADER   = "filter.vs";
  static constexpr auto PASS1_FRAGMENT_SHADER = "pass1_update_filter_buff1_and_buff3.fs";
  auto Pass1UpdateFilterBuff1AndBuff3() noexcept -> void;

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

  static constexpr auto FILTER_BUFF1_TEX_LOCATION    = 0;
  static constexpr auto FILTER_BUFF2_TEX_LOCATION    = 1;
  static constexpr auto FILTER_BUFF3_TEX_LOCATION    = 2;
  static constexpr auto FILTER_SRCE_POS_TEX_LOCATION = 3;
  static constexpr auto FILTER_DEST_POS_TEX_LOCATION = 4;
  static constexpr auto MAIN_IMAGE_TEX_LOCATION      = 5;
  static constexpr auto LOW_IMAGE_TEX_LOCATION       = 6;
  static constexpr auto LUM_AVG_TEX_LOCATION         = 7;

  static constexpr auto FILTER_BUFF1_TEX_SHADER_NAME    = "";
  static constexpr auto FILTER_BUFF2_TEX_SHADER_NAME    = "tex_filterBuff2";
  static constexpr auto FILTER_BUFF3_TEX_SHADER_NAME    = "";
  static constexpr auto FILTER_SRCE_POS_TEX_SHADER_NAME = "tex_filterSrcePositions";
  static constexpr auto FILTER_DEST_POS_TEX_SHADER_NAME = "tex_filterDestPositions";
  static constexpr auto MAIN_IMAGE_TEX_SHADER_NAME      = "tex_mainImage";
  static constexpr auto LOW_IMAGE_TEX_SHADER_NAME       = "tex_lowImage";

  static constexpr auto FILTER_BUFF1_IMAGE_UNIT = 0;
  static constexpr auto FILTER_BUFF2_IMAGE_UNIT = 1;
  static constexpr auto FILTER_BUFF3_IMAGE_UNIT = 2;
  static constexpr auto LUM_AVG_IMAGE_UNIT      = 3;

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
                -1,
                FILTER_SRCE_POS_TEX_LOCATION,
                FILTER_POS_TEX_FORMAT,
                FILTER_POS_TEX_INTERNAL_FORMAT,
                FILTER_POS_TEX_PIXEL_TYPE,
                NUM_PBOS>
        filterSrcePosTexture{};
    Gl2DTexture<FilterPosBuffersXY,
                -1,
                FILTER_DEST_POS_TEX_LOCATION,
                FILTER_POS_TEX_FORMAT,
                FILTER_POS_TEX_INTERNAL_FORMAT,
                FILTER_POS_TEX_PIXEL_TYPE,
                NUM_PBOS>
        filterDestPosTexture{};
  };
  GlFilterPosBuffers m_glFilterPosBuffers{};
  auto SetupGlFilterPosBuffers() -> void;
  auto BindGlFilterPosBuffers() noexcept -> void;

  struct GlFilterBuffers
  {
    Gl2DTexture<GOOM::PixelIntType,
                FILTER_BUFF1_IMAGE_UNIT,
                FILTER_BUFF1_TEX_LOCATION,
                FILTER_BUFF_TEX_FORMAT,
                FILTER_BUFF_TEX_INTERNAL_FORMAT,
                FILTER_BUFF_TEX_PIXEL_TYPE,
                0>
        filterBuff1Texture{};
    Gl2DTexture<GOOM::PixelIntType,
                FILTER_BUFF2_IMAGE_UNIT,
                FILTER_BUFF2_TEX_LOCATION,
                FILTER_BUFF_TEX_FORMAT,
                FILTER_BUFF_TEX_INTERNAL_FORMAT,
                FILTER_BUFF_TEX_PIXEL_TYPE,
                0>
        filterBuff2Texture{};
    Gl2DTexture<GOOM::PixelIntType,
                FILTER_BUFF3_IMAGE_UNIT,
                FILTER_BUFF3_TEX_LOCATION,
                FILTER_BUFF_TEX_FORMAT,
                FILTER_BUFF_TEX_INTERNAL_FORMAT,
                FILTER_BUFF_TEX_PIXEL_TYPE,
                0>
        filterBuff3Texture{};
  };
  GlFilterBuffers m_glFilterBuffers{};
  std::atomic_bool m_filterBuffersNeedClearing = false;
  auto SetupGlFilterBuffers() -> void;
  auto BindGlFilterBuffer2() noexcept -> void;
  auto CheckZeroFilterBuffers() noexcept -> void;

  struct GlImageBuffers
  {
    Gl2DTexture<GOOM::Pixel,
                -1,
                MAIN_IMAGE_TEX_LOCATION,
                IMAGE_TEX_FORMAT,
                IMAGE_TEX_INTERNAL_FORMAT,
                IMAGE_TEX_PIXEL_TYPE,
                NUM_PBOS>
        mainImageTexture{};
    Gl2DTexture<GOOM::Pixel,
                -1,
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
};

inline auto DisplacementFilter::GetFrameData(const size_t pboIndex) noexcept -> GOOM::FrameData&
{
  return m_frameDataArray.at(pboIndex);
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
  m_glFilterBuffers.filterBuff3Texture.BindTexture(m_programPass1UpdateFilterBuff1AndBuff3);
}

} // namespace GOOM::OPENGL
