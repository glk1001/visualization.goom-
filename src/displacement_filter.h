#pragma once

#include "gl_2d_textures.h"
#include "gl_render_types.h"
#include "glsl_program.h"
#include "goom/frame_data.h"
#include "goom/goom_graphic.h"
#include "scene.h"

//[[nodiscard]] inline auto to_string(const GOOM::Point2dInt& point)
//{
//  return std_fmt::format("{:+6d}, {:+6d}", point.x, point.y);
//}
//[[nodiscard]] inline auto to_string(const GOOM::Point2dFlt& point)
//{
//  return std_fmt::format("{:+.5f}, {:+.5f}", point.x, point.y);
//}
//[[nodiscard]] inline auto to_string(const GOOM::Pixel& pixel)
//{
//  return std_fmt::format(
//      "{:5d}, {:5d}, {:5d}, {:5d}", pixel.R(), pixel.G(), pixel.B(), pixel.A());
//}
//#include "src/goom/src/utils/buffer_saver.h"

#include <atomic>
#include <functional>
#include <span>
#include <string>
#include <vector>

namespace GOOM::OPENGL
{

class DisplacementFilter : public IScene
{
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

public:
  static constexpr auto NUM_PBOS = 3U;
  using FilterPosDataXY          = GOOM::Point2dFlt;

  DisplacementFilter(const std::string& shaderDir,
                     const GOOM::TextureBufferDimensions& textureBufferDimensions) noexcept;

  auto InitScene() -> void override;
  auto Resize(const GOOM::WindowDimensions& windowDimensions) noexcept -> void override;
  auto DestroyScene() noexcept -> void;

  [[nodiscard]] auto GetFrameDataArray() noexcept -> std::vector<FrameData>&;
  auto InitAllFrameDataToGl() noexcept -> void;

  auto Render() noexcept -> void override;

  auto ClearFilterBuffers() noexcept -> void;

  [[nodiscard]] auto GetFrameData(size_t pboIndex) noexcept -> GOOM::FrameData&;
  auto UpdateFrameData(size_t pboIndex) noexcept -> void;
  using RequestNextFrameDataFunc = std::function<bool()>;
  auto SetRequestNextFrameDataFunc(
      const RequestNextFrameDataFunc& requestNextFrameDataFunc) noexcept -> void;
  using ReleaseCurrentFrameDataFunc = std::function<void(size_t slot)>;
  auto SetReleaseCurrentFrameDataFunc(
      const ReleaseCurrentFrameDataFunc& releaseCurrentFrameDataFunc) noexcept -> void;

private:
  std::string m_shaderDir;
  size_t m_buffSize;
  float m_aspectRatio;
  GLuint m_renderToTextureFbo{};
  GLuint m_renderTextureName{};
  std::vector<FrameData> m_frameDataArray;
  size_t m_currentPboIndex = 0U;
  auto InitFrameDataArrayPointers(std::vector<FrameData>& frameDataArray) noexcept -> void;
  auto CopyTextureData(GLuint srceTextureName, GLuint destTextureName) const noexcept -> void;

  //  auto SaveBuffersBeforePass1() -> void;
  //  auto SaveBuffersAfterPass1() -> void;
  //  auto SaveBuffersAfterPass2() -> void;
  //  static inline const auto saveDir = std::string{"/home/greg/.kodi/junk/"};
  //  UTILS::BufferSaver<Point2dInt> m_filterPosSrceBufferSave{saveDir + "filter_pos_srce"};
  //  UTILS::BufferSaver<Point2dInt> m_filterPosDestBufferSave{saveDir + "filter_pos_dest"};
  //  UTILS::BufferSaver<Point2dInt> m_filterPosDestInBufferSave{saveDir + "filter_pos_dest_in"};
  //  UTILS::BufferSaver<GOOM::Pixel> m_filterBuffer1SaveBeforePass1{saveDir +
  //                                                                 "filter_buff1_before_pass1"};
  //  UTILS::BufferSaver<GOOM::Pixel> m_filterBuffer1SaveAfterPass1{saveDir +
  //                                                                "filter_buff1_after_pass1"};
  //  UTILS::BufferSaver<GOOM::Pixel> m_filterBuffer1SaveAfterPass2{saveDir +
  //                                                                "filter_buff1_after_pass2"};
  //  UTILS::BufferSaver<GOOM::Pixel> m_lowImageSaveBeforePass1{saveDir + "low_image_before_pass1"};
  //  UTILS::BufferSaver<GOOM::Pixel> m_lowImageSaveAfterPass1{saveDir + "low_image_after_pass1"};
  //  UTILS::BufferSaver<GOOM::Pixel> m_lowImageSaveAfterPass2{saveDir + "low_image_after_pass2"};

  std::vector<Point2dFlt> m_previousFilterDestPos{};

  GLuint m_fsQuad{};
  static constexpr GLuint COMPONENTS_PER_VERTEX     = 2;
  static constexpr int32_t NUM_VERTICES_IN_TRIANGLE = 3;
  static constexpr int32_t NUM_TRIANGLES            = 2;
  static constexpr int32_t NUM_VERTICES             = NUM_TRIANGLES * NUM_VERTICES_IN_TRIANGLE;

  auto CompileAndLinkShaders() -> void;
  using ShaderMacros = std::map<std::string, std::string>;
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
  [[nodiscard]] auto GetLumAverage() const noexcept -> float;
  RequestNextFrameDataFunc m_requestNextFrameData{};
  ReleaseCurrentFrameDataFunc m_releaseCurrentFrameData{};
  auto UpdateFrameDataToGl(size_t pboIndex) noexcept -> void;
  auto UpdatePass1MiscDataToGl(size_t pboIndex) noexcept -> void;
  auto UpdatePass4MiscDataToGl(size_t pboIndex) noexcept -> void;
  auto UpdatePosDataToGl(size_t pboIndex) noexcept -> void;
  auto UpdateImageDataToGl(size_t pboIndex) noexcept -> void;

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

  struct GlFilterPosData
  {
    Gl2DTexture<FilterPosDataXY,
                -1,
                FILTER_SRCE_POS_TEX_LOCATION,
                FILTER_POS_TEX_FORMAT,
                FILTER_POS_TEX_INTERNAL_FORMAT,
                FILTER_POS_TEX_PIXEL_TYPE,
                NUM_PBOS>
        filterSrcePosTexture{};
    Gl2DTexture<FilterPosDataXY,
                -1,
                FILTER_DEST_POS_TEX_LOCATION,
                FILTER_POS_TEX_FORMAT,
                FILTER_POS_TEX_INTERNAL_FORMAT,
                FILTER_POS_TEX_PIXEL_TYPE,
                NUM_PBOS>
        filterDestPosTexture{};
  };
  GlFilterPosData m_glFilterPosData{};
  auto SetupGlFilterPosData() -> void;
  auto BindGlFilterPosData() noexcept -> void;

  struct GlFilterBuffData
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
  GlFilterBuffData m_glFilterBuffData{};
  std::atomic_bool m_filterBuffersNeedClearing = false;
  auto SetupGlFilterBuffData() -> void;
  auto BindGlFilterBuff2Data() noexcept -> void;
  auto CheckZeroFilterBuffers() noexcept -> void;

  struct GlImageData
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
  GlImageData m_glImageData{};
  auto SetupGlImageData() -> void;
  auto BindGlImageData() noexcept -> void;
};

inline auto DisplacementFilter::GetFrameDataArray() noexcept -> std::vector<FrameData>&
{
  return m_frameDataArray;
}

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

} // namespace GOOM::OPENGL
