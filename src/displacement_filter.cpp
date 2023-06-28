#undef NO_LOGGING

#include "displacement_filter.h"

#include "gl_utils.h"
#include "goom/goom_logger.h"

#include <algorithm>
#include <filesystem>
#include <format>
#include <span>
#include <stdexcept>

using std::filesystem::temp_directory_path;

// TODO - Need to pass goomLogger
//std_fmt::println("{}", __LINE__);

namespace GOOM::OPENGL
{

static auto InitTranBufferDest(const Dimensions& dimensions,
                               std_spn::span<Point2dFlt>& tranBufferFlt) noexcept -> void
{
  Expects(dimensions.GetSize() == tranBufferFlt.size());

  static constexpr auto MIN_COORD   = -2.0F;
  static constexpr auto MAX_COORD   = +2.0F;
  static constexpr auto COORD_WIDTH = MAX_COORD - MIN_COORD;
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

DisplacementFilter::DisplacementFilter(
    const std::string& shaderDir, const TextureBufferDimensions& textureBufferDimensions) noexcept
  : IScene{textureBufferDimensions},
    m_shaderDir{shaderDir},
    m_buffSize{static_cast<size_t>(GetWidth()) * static_cast<size_t>(GetHeight())},
    m_frameDataArray(NUM_PBOS),
    m_previousFilterDestPos(m_buffSize)
{
}

auto DisplacementFilter::InitScene() -> void
{
  CompileAndLinkShader();

  SetupRenderToTextureFBO();

  SetupScreenBuffers();

  SetupProgramSubroutines();

  SetupGlData();

  SetupGlLumComputeData();

  InitFilterBuffers();
}

auto DisplacementFilter::DestroyScene() noexcept -> void
{
  m_program.DeleteProgram();
  m_lumHistogramComputeProgram.DeleteProgram();
  m_lumAverageComputeProgram.DeleteProgram();
}

auto DisplacementFilter::Resize(const WindowDimensions& windowDimensions) noexcept -> void
{
  SetFramebufferDimensions(windowDimensions);
}

auto DisplacementFilter::SetupRenderToTextureFBO() noexcept -> void
{
  // Generate and bind the FBO.
  glGenFramebuffers(1, &m_renderToTextureFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_renderToTextureFbo);

  // Create the texture object.
  GLuint renderTextureName{};
  glGenTextures(1, &renderTextureName);
  glBindTexture(GL_TEXTURE_2D, renderTextureName);
  glTexStorage2D(GL_TEXTURE_2D, 1, FILTER_BUFF_TEX_INTERNAL_FORMAT, GetWidth(), GetHeight());

  // Bind the texture to the FBO.
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTextureName, 0);

  // Set the targets for the fragment output variables.
  const auto drawBuffers = std::array<GLenum, 1>{GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, drawBuffers.data());

  // Unbind the FBO, and revert to default framebuffer.
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
  glGenBuffers(NUM_ARRAY_BUFFERS, handle.data());

  glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<size_t>(COMPONENTS_PER_VERTEX * NUM_VERTICES) * sizeof(float),
               VERTICES.data(),
               GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<size_t>(COMPONENTS_PER_VERTEX * NUM_VERTICES) * sizeof(float),
               TEX_COORDS.data(),
               GL_STATIC_DRAW);

  // TODO - Use 4.4 OpenGL - see cookbook
  // Setup the vertex and texture array objects.
  glGenVertexArrays(1, &m_fsQuad);
  glBindVertexArray(m_fsQuad);

  glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
  glVertexAttribPointer(0, COMPONENTS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0); // Vertex position

  glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
  glVertexAttribPointer(1, COMPONENTS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(1); // Texture coordinates

  glBindVertexArray(0);
}

auto DisplacementFilter::CompileAndLinkShader() -> void
{
  try
  {
    static constexpr auto* INCLUDE_DIR = "";

    const auto tempVertexShaderFile = temp_directory_path().string() + "/filter.vs";
    PutFileWithExpandedIncludes(INCLUDE_DIR, m_shaderDir + "/filter.vs", tempVertexShaderFile);
    m_program.CompileShader(tempVertexShaderFile);

    const auto tempFragmentShaderFile = temp_directory_path().string() + "/filter.fs";
    PutFileWithExpandedIncludes(INCLUDE_DIR, m_shaderDir + "/filter.fs", tempFragmentShaderFile);
    m_program.CompileShader(tempFragmentShaderFile);

    m_program.LinkShader();

    const auto tempLumHistogramShaderFile = temp_directory_path().string() + "/lum_histogram.cs";
    PutFileWithExpandedIncludes(
        INCLUDE_DIR, m_shaderDir + "/lum_histogram.cs", tempLumHistogramShaderFile);
    m_lumHistogramComputeProgram.CompileShader(tempLumHistogramShaderFile);
    m_lumHistogramComputeProgram.LinkShader();

    const auto tempLumAverageShaderFile = temp_directory_path().string() + "/lum_avg.cs";
    PutFileWithExpandedIncludes(INCLUDE_DIR, m_shaderDir + "/lum_avg.cs", tempLumAverageShaderFile);
    m_lumAverageComputeProgram.CompileShader(tempLumAverageShaderFile);
    m_lumAverageComputeProgram.LinkShader();
  }
  catch (GlslProgramException& e)
  {
    throw std::runtime_error{std::string{"Compile fail: "} + e.what()};
  }
}

auto DisplacementFilter::SetupGlData() -> void
{
  SetupGlParams();
  SetupGlFilterBuffData();
  SetupGlFilterPosData();
  SetupGlImageData();

  InitFrameDataArrayPointers(m_frameDataArray);
}

auto DisplacementFilter::SetupGlParams() -> void
{
  glDisable(GL_BLEND);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
}

auto DisplacementFilter::SetupProgramSubroutines() noexcept -> void
{
  m_program.Use();
  m_pass1Index = m_program.GetSubroutineIndex(GL_FRAGMENT_SHADER, PASS1_NAME);
  m_pass2Index = m_program.GetSubroutineIndex(GL_FRAGMENT_SHADER, PASS2_NAME);
}

auto DisplacementFilter::Update(const float t) noexcept -> void
{
  m_lumAverageComputeProgram.Use();
  SetLumAverageParams(t);
}

auto DisplacementFilter::Render() noexcept -> void
{
  glViewport(0, 0, GetWidth(), GetHeight());

  m_program.Use();
  //  SaveBuffersBeforePass1();

  m_requestNextFrameData();
  Pass1UpdateFilterBuffers();
  //  SaveBuffersAfterPass1();

  m_lumHistogramComputeProgram.Use();
  LumHistogramComputePass();

  m_lumAverageComputeProgram.Use();
  LumAverageComputePass();

  m_program.Use();
  Pass2OutputToneMappedImage();
  //  SaveBuffersAfterPass2();

  glViewport(0, 0, GetFramebufferWidth(), GetFramebufferHeight());
  Pass3OutputToScreen();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

auto DisplacementFilter::InitAllFrameDataToGl() noexcept -> void
{
  m_program.Use();

  for (auto i = 0U; i < NUM_PBOS; ++i)
  {
    UpdateFrameDataToGl(i);

    auto filterSrcePos = m_glFilterPosData.filterSrcePosTexture.GetMappedBuffer(i);
    InitTranBufferDest({static_cast<uint32_t>(GetWidth()), static_cast<uint32_t>(GetHeight())},
                       filterSrcePos);
    m_glFilterPosData.filterSrcePosTexture.CopyMappedBufferToTexture(i);
  }
}

auto DisplacementFilter::UpdateFrameData(const size_t pboIndex) noexcept -> void
{
  UpdateFrameDataToGl(pboIndex);
  CheckZeroFilterBuffers();

  static constexpr auto TIMEOUT_NANO = 100U * 1000U * 1000U;
  m_glFenceSync                      = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  /*const auto result = */ glClientWaitSync(
      m_glFenceSync, GL_SYNC_FLUSH_COMMANDS_BIT, TIMEOUT_NANO);
  // if ((result == GL_ALREADY_SIGNALED) or (result == GL_CONDITION_SATISFIED))
  // {
  //   std_fmt::println("Fence finished OK.");
  // }
  // else
  // {
  //   std_fmt::println("Fence did not finish.");
  // }
}

// auto DisplacementFilter::SaveBuffersBeforePass1() -> void
// {
//  auto filterBuffer = std::vector<GOOM::Pixel>(m_buffSize);
//  m_glFilterBuffData.filterBuff1Texture.BindTexture(m_program);
//  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, FILTER_BUFF_TEX_PIXEL_TYPE, filterBuffer.data());
//  GOOM::OPENGL::CheckForOpenGLError(__FILE__, __LINE__);
//
//  auto lowImage = std::vector<GOOM::Pixel>(m_buffSize);
//  m_glImageData.lowImageTexture.BindTexture(m_program);
//  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, FILTER_BUFF_TEX_PIXEL_TYPE, lowImage.data());
//  GOOM::OPENGL::CheckForOpenGLError(__FILE__, __LINE__);
//
//  glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
//
//  auto lowImageView = UTILS::BufferView<GOOM::Pixel>{lowImage.size(), lowImage.data()};
//  m_lowImageSaveBeforePass1.Write(lowImageView, false);
//  auto filterBufferView =
//      UTILS::BufferView<GOOM::Pixel>{filterBuffer.size(), filterBuffer.data()};
//  m_filterBuffer1SaveBeforePass1.Write(filterBufferView, false);
//}
//
//auto DisplacementFilter::SaveBuffersAfterPass1() -> void
//{
//  auto filterBuffer = std::vector<GOOM::Pixel>(m_buffSize);
//  m_glFilterBuffData.filterBuff1Texture.BindTexture(m_program);
//  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, FILTER_BUFF_TEX_PIXEL_TYPE, filterBuffer.data());
//  GOOM::OPENGL::CheckForOpenGLError(__FILE__, __LINE__);
//
//  glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
//
//  auto filterBufferView =
//      UTILS::BufferView<GOOM::Pixel>{filterBuffer.size(), filterBuffer.data()};
//  m_filterBuffer1SaveAfterPass1.Write(filterBufferView, false);
//}
//
//auto DisplacementFilter::SaveBuffersAfterPass2() -> void
//{
//  auto filterBuffer = std::vector<GOOM::Pixel>(m_buffSize);
//  m_glFilterBuffData.filterBuff1Texture.BindTexture(m_program);
//  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, FILTER_BUFF_TEX_PIXEL_TYPE, filterBuffer.data());
//  GOOM::OPENGL::CheckForOpenGLError(__FILE__, __LINE__);
//
//  glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
//
//  auto filterBufferView =
//      UTILS::BufferView<GOOM::Pixel>{filterBuffer.size(), filterBuffer.data()};
//  m_filterBuffer1SaveAfterPass2.Write(filterBufferView, false);
//}

auto DisplacementFilter::Pass1UpdateFilterBuffers() noexcept -> void
{
  BindGlFilterPosData();
  BindGlFilterBuffData();
  BindGlImageData();

  glBindFramebuffer(GL_FRAMEBUFFER, m_renderToTextureFbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_pass1Index);

  // Render the full-screen quad
  glBindVertexArray(m_fsQuad);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, NUM_VERTICES);

  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

auto DisplacementFilter::Pass2OutputToneMappedImage() noexcept -> void
{
  //std_fmt::println("Before av lum {}", __LINE__);
  //m_program.SetUniform("u_averageLuminance", GetLumAverage());
  //std_fmt::println("After av lum {}", __LINE__);
  //std_fmt::println("LumAverage = {}.", GetLumAverage());

  glBindFramebuffer(GL_FRAMEBUFFER, m_renderToTextureFbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_pass2Index);

  // Render the full-screen quad
  glBindVertexArray(m_fsQuad);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, NUM_VERTICES);

  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

auto DisplacementFilter::Pass3OutputToScreen() noexcept -> void
{
  glBlitNamedFramebuffer(m_renderToTextureFbo,
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
                         GL_LINEAR);
}

auto DisplacementFilter::LumHistogramComputePass() noexcept -> void
{
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_histogramBufferName);
  glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);

  glDispatchCompute(static_cast<GLuint>(GetWidth() / 16), static_cast<GLuint>(GetHeight() / 16), 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  /**
  glBindBuffer(GL_ARRAY_BUFFER, m_bufferName);
  void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
  uint32_t data[HISTOGRAM_BUFFER_LENGTH];
  memcpy(data, ptr, sizeof(data));
  glUnmapBuffer(GL_ARRAY_BUFFER);
  std_fmt::print("Lum Hist: ");
  for (auto i = 0U; i < HISTOGRAM_BUFFER_LENGTH; ++i)
  {
    std_fmt::print("{}, ", data[i]);
  }
  std_fmt::println("");
   **/
}

auto DisplacementFilter::LumAverageComputePass() noexcept -> void
{
  glDispatchCompute(LUM_AVG_GROUP_SIZE, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);

  /**
  glBindBuffer(GL_ARRAY_BUFFER, m_bufferName);
  void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
  uint32_t data[HISTOGRAM_BUFFER_LENGTH];
  memcpy(data, ptr, sizeof(data));
  glUnmapBuffer(GL_ARRAY_BUFFER);
  std_fmt::print("Lum Hist: ");
  for (auto i = 0U; i < HISTOGRAM_BUFFER_LENGTH; ++i)
  {
    std_fmt::print("{}, ", data[i]);
  }
  std_fmt::println("");
   **/
}

auto DisplacementFilter::SetLumHistogramParams() noexcept -> void
{
  static constexpr auto MIN_LOG_LUM = -8.0F;
  static constexpr auto MAX_LOG_LUM = +3.5F;
  static_assert((MAX_LOG_LUM - MIN_LOG_LUM) > 0.0F);

  const auto histogramParams = glm::vec4{MIN_LOG_LUM,
                                         1.0F / (MAX_LOG_LUM - MIN_LOG_LUM),
                                         static_cast<float>(GetWidth()),
                                         static_cast<float>(GetHeight())};

  m_lumHistogramComputeProgram.SetUniform("u_params", histogramParams);
}

auto DisplacementFilter::SetLumAverageParams(const float frameTime) noexcept -> void
{
  static constexpr auto MIN_LOG_LUM = -8.0F;
  static constexpr auto MAX_LOG_LUM = +3.5F;

  static constexpr auto TAU = 1.1F;
  const auto timeCoeff      = std::clamp(1.0F - std::exp(-frameTime * TAU), 0.0F, 1.0F);

  const auto lumAverageParams =
      glm::vec4{MIN_LOG_LUM, MAX_LOG_LUM - MIN_LOG_LUM, timeCoeff, static_cast<float>(m_buffSize)};

  m_lumAverageComputeProgram.SetUniform("u_params", lumAverageParams);
}

auto DisplacementFilter::SetupGlLumComputeData() noexcept -> void
{
  SetupGlLumHistogramBuffer();

  m_lumHistogramComputeProgram.Use();
  SetLumHistogramParams();

  m_lumAverageComputeProgram.Use();
  static constexpr auto LUM_AVG_TIME_COEFF = 2.0F;
  SetLumAverageParams(LUM_AVG_TIME_COEFF);
  SetupGlLumAverageData();
}

auto DisplacementFilter::GetLumAverage() const noexcept -> float
{
  //return 0.5F;
  auto lumAverage = 0.0F;

  glBindTexture(GL_TEXTURE_2D, m_lumDataTextureName);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &lumAverage);

  return lumAverage;
}

auto DisplacementFilter::InitFrameDataArrayPointers(std::vector<FrameData>& frameDataArray) noexcept
    -> void
{
  auto prevFilterDest =
      std_spn::span<Point2dFlt>{m_previousFilterDestPos.data(), m_previousFilterDestPos.size()};
  InitTranBufferDest({static_cast<uint32_t>(GetWidth()), static_cast<uint32_t>(GetHeight())},
                     prevFilterDest);

  for (auto i = 0U; i < NUM_PBOS; ++i)
  {
    frameDataArray.at(i).filterPosArrays.filterDestPos =
        m_glFilterPosData.filterDestPosTexture.GetMappedBuffer(i);

    frameDataArray.at(i).imageArrays.mainImageData.SetPixelBuffer(
        m_glImageData.mainImageTexture.GetMappedBuffer(i),
        Dimensions{static_cast<uint32_t>(GetWidth()), static_cast<uint32_t>(GetHeight())});
    frameDataArray.at(i).imageArrays.lowImageData.SetPixelBuffer(
        m_glImageData.lowImageTexture.GetMappedBuffer(i),
        Dimensions{static_cast<uint32_t>(GetWidth()), static_cast<uint32_t>(GetHeight())});
  }
}

auto DisplacementFilter::UpdateFrameDataToGl(const size_t pboIndex) noexcept -> void
{
  UpdateMiscDataToGl(pboIndex);
  UpdatePosDataToGl(pboIndex);
  UpdateImageDataToGl(pboIndex);
}

auto DisplacementFilter::UpdateMiscDataToGl(const size_t pboIndex) noexcept -> void
{
  //LogInfo(GOOM::UTILS::GetGoomLogger(), "New lerpFactor = {}.", m_previousLerpFactor);
  m_program.SetUniform("u_lerpFactor", m_frameDataArray.at(pboIndex).miscData.lerpFactor);
  m_program.SetUniform("u_brightness", m_frameDataArray.at(pboIndex).miscData.brightness);
}

//[[nodiscard]] static auto GetTranArray(const Point2dFlt* const buffIn,
//                                       const int width,
//                                       [[maybe_unused]] const int height,
//                                       const size_t buffSize) -> std::vector<Point2dInt>
//{
//  auto buffOut = std::vector<Point2dInt>(buffSize);
//  for (auto i = 0U; i < buffSize; ++i)
//  {
//    buffOut[i] = {
//        16 * static_cast<int32_t>(static_cast<float>(width - 1) * (2.0F + buffIn[i].x) / 4.0F),
//        16 * static_cast<int32_t>(static_cast<float>(width - 1) * (2.0F + buffIn[i].y) / 4.0F)};
//  }
//
//  return buffOut;
//}

auto DisplacementFilter::UpdatePosDataToGl(const size_t pboIndex) noexcept -> void
{
  if (not m_frameDataArray.at(pboIndex).filterPosArrays.filterDestPosNeedsUpdating)
  {
    return;
  }

  // LogInfo(GOOM::UTILS::GetGoomLogger(), "Filter dest pos needs updating.");

  // const auto* filterPosDestIn = m_glFilterPosData.filterDestPosTexture.GetMappedBuffer(pboIndex);
  // const auto filterPosDestInInt =
  //    GetTranArray(filterPosDestIn, GetWidth(), GetHeight(), m_buffSize);
  // auto filterPosDestInIntBufferView =
  //     UTILS::BufferView<Point2dInt>{m_buffSize, filterPosDestInInt.data()};
  // m_filterPosDestInBufferSave.Write(filterPosDestInIntBufferView, false);

  // TODO - Can a pbo Id swap be made to work?
  //    LogInfo(GOOM::UTILS::GetGoomLogger(),
  //            "Copying lerped srce/dest to srce. Lerpfactor = {}.",
  //            m_previousLerpFactor);
  auto filterSrcePos = m_glFilterPosData.filterSrcePosTexture.GetMappedBuffer(pboIndex);
  const auto destToSrceLerpFactor =
      m_frameDataArray.at(pboIndex).filterPosArrays.lerpFactorForDestToSrceUpdate;
  std::transform(m_previousFilterDestPos.cbegin(),
                 m_previousFilterDestPos.cend(),
                 filterSrcePos.begin(),
                 filterSrcePos.begin(),
                 [&destToSrceLerpFactor](const Point2dFlt& prevDestPos, const Point2dFlt& srcePos)
                 { return lerp(srcePos, prevDestPos, destToSrceLerpFactor); });


  m_glFilterPosData.filterSrcePosTexture.CopyMappedBufferToTexture(pboIndex);
  m_glFilterPosData.filterDestPosTexture.CopyMappedBufferToTexture(pboIndex);

  const auto filterDestPos = m_glFilterPosData.filterDestPosTexture.GetMappedBuffer(pboIndex);
  std::copy(filterDestPos.begin(), filterDestPos.end(), m_previousFilterDestPos.begin());

  //  auto filterPosSrce = std::vector<Point2dFlt>(m_buffSize);
  //  m_glFilterPosData.filterSrcePosTexture.BindTexture(m_program);
  //  glGetTexImage(
  //      GL_TEXTURE_2D, 0, FILTER_POS_TEX_FORMAT, FILTER_POS_TEX_PIXEL_TYPE, filterPosSrce.data());
  //  GOOM::OPENGL::CheckForOpenGLError(__FILE__, __LINE__);
  //
  //  auto filterPosDest = std::vector<Point2dFlt>(m_buffSize);
  //  m_glFilterPosData.filterDestPosTexture.BindTexture(m_program);
  //  glGetTexImage(
  //      GL_TEXTURE_2D, 0, FILTER_POS_TEX_FORMAT, FILTER_POS_TEX_PIXEL_TYPE, filterPosDest.data());
  //  GOOM::OPENGL::CheckForOpenGLError(__FILE__, __LINE__);
  //
  //  glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
  //
  //  const auto filterPosSrceInt =
  //      GetTranArray(filterPosSrce.data(), GetWidth(), GetHeight(), m_buffSize);
  //  auto filterPosSrceBufferView =
  //      UTILS::BufferView<Point2dInt>{filterPosSrceInt.size(), filterPosSrceInt.data()};
  //  m_filterPosSrceBufferSave.Write(filterPosSrceBufferView, false);
  //
  //  const auto filterPosDestInt =
  //      GetTranArray(filterPosDest.data(), GetWidth(), GetHeight(), m_buffSize);
  //  auto filterPosDestBufferView =
  //      UTILS::BufferView<Point2dInt>{filterPosDestInt.size(), filterPosDestInt.data()};
  //  m_filterPosDestBufferSave.Write(filterPosDestBufferView, false);
}

auto DisplacementFilter::CopyTextureData(const GLuint srceTextureName,
                                         const GLuint destTextureName) const noexcept -> void
{
  glCopyImageSubData(srceTextureName,
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
                     1);
}

auto DisplacementFilter::UpdateImageDataToGl(const size_t pboIndex) noexcept -> void
{
  if (m_frameDataArray.at(pboIndex).imageArrays.mainImageDataNeedsUpdating)
  {
    m_glImageData.mainImageTexture.CopyMappedBufferToTexture(pboIndex);
  }
  if (m_frameDataArray.at(pboIndex).imageArrays.lowImageDataNeedsUpdating)
  {
    m_glImageData.lowImageTexture.CopyMappedBufferToTexture(pboIndex);
  }
}

auto DisplacementFilter::BindGlFilterPosData() noexcept -> void
{
  m_glFilterPosData.filterSrcePosTexture.BindTexture(m_program);
  m_glFilterPosData.filterDestPosTexture.BindTexture(m_program);
}

auto DisplacementFilter::BindGlFilterBuffData() noexcept -> void
{
  m_glFilterBuffData.filterBuff2Texture.BindTexture(m_program);
}

auto DisplacementFilter::BindGlImageData() noexcept -> void
{
  m_glImageData.mainImageTexture.BindTexture(m_program);
  m_glImageData.lowImageTexture.BindTexture(m_program);
}

auto DisplacementFilter::SetupGlFilterBuffData() -> void
{
  m_glFilterBuffData.filterBuff1Texture.Setup(
      FILTER_BUFF1_TEX_SHADER_NAME, GetWidth(), GetHeight());
  m_glFilterBuffData.filterBuff2Texture.Setup(
      FILTER_BUFF2_TEX_SHADER_NAME, GetWidth(), GetHeight());
  m_glFilterBuffData.filterBuff3Texture.Setup(
      FILTER_BUFF3_TEX_SHADER_NAME, GetWidth(), GetHeight());
}

auto DisplacementFilter::SetupGlFilterPosData() -> void
{
  m_glFilterPosData.filterSrcePosTexture.Setup(
      FILTER_SRCE_POS_TEX_SHADER_NAME, GetWidth(), GetHeight());
  m_glFilterPosData.filterDestPosTexture.Setup(
      FILTER_DEST_POS_TEX_SHADER_NAME, GetWidth(), GetHeight());
}

auto DisplacementFilter::SetupGlImageData() -> void
{
  m_glImageData.mainImageTexture.Setup(MAIN_IMAGE_TEX_SHADER_NAME, GetWidth(), GetHeight());
  m_glImageData.lowImageTexture.Setup(LOW_IMAGE_TEX_SHADER_NAME, GetWidth(), GetHeight());
}

auto DisplacementFilter::SetupGlLumHistogramBuffer() noexcept -> void
{
  glGenBuffers(1, &m_histogramBufferName);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_histogramBufferName);
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               HISTOGRAM_BUFFER_LENGTH * sizeof(uint32_t),
               nullptr,
               GL_DYNAMIC_COPY);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LUM_HISTOGRAM_BUFFER_INDEX, m_histogramBufferName);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

auto DisplacementFilter::SetupGlLumAverageData() noexcept -> void
{
  glGenTextures(1, &m_lumDataTextureName);
  glActiveTexture(LUM_AVG_TEX_UNIT);
  glBindTexture(GL_TEXTURE_2D, m_lumDataTextureName);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16F, 1, 1);
  glBindImageTexture(
      LUM_AVG_IMAGE_UNIT, m_lumDataTextureName, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R16F);

  const auto initialData = 0.5F;
  glBindTexture(GL_TEXTURE_2D, m_lumDataTextureName);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED, GL_FLOAT, &initialData);
}

auto DisplacementFilter::InitFilterBuffers() noexcept -> void
{
  ClearFilterBuffers();
}

auto DisplacementFilter::ClearFilterBuffers() noexcept -> void
{
  m_filterBuffersNeedClearing = true;
}

auto DisplacementFilter::CheckZeroFilterBuffers() noexcept -> void
{
  if (not m_filterBuffersNeedClearing)
  {
    return;
  }

  m_glFilterBuffData.filterBuff1Texture.ZeroTextureData();
  m_glFilterBuffData.filterBuff2Texture.ZeroTextureData();
  m_glFilterBuffData.filterBuff3Texture.ZeroTextureData();

  m_filterBuffersNeedClearing = false;
}

} // namespace GOOM::OPENGL
