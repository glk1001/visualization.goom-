#undef NO_LOGGING

#include "displacement_filter.h"

#include "filter_fx/normalized_coords.h"
#include "gl_utils.h"
#include "glsl_shader_file.h"
#include "goom/goom_logger.h"

#include <algorithm>
#include <filesystem>
#include <format>
#include <span>
#include <stdexcept>

namespace fs = std::filesystem;

// TODO - Need to pass goomLogger
//std_fmt::println("{}", __LINE__);

namespace GOOM::OPENGL
{

using GOOM::FILTER_FX::NormalizedCoords;

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
    m_aspectRatio{static_cast<float>(GetWidth()) / static_cast<float>(GetHeight())},
    m_frameDataArray(NUM_PBOS),
    m_previousFilterDestPos(m_buffSize)
{
}

auto DisplacementFilter::InitScene() -> void
{
  CompileAndLinkShaders();

  SetupRenderToTextureFBO();

  SetupScreenBuffers();

  SetupGlData();

  SetupGlLumComputeData();

  InitFilterBuffers();
}

auto DisplacementFilter::DestroyScene() noexcept -> void
{
  glDeleteTextures(1, &m_renderTextureName);
  glDeleteFramebuffers(1, &m_renderToTextureFbo);

  m_glFilterPosData.filterSrcePosTexture.DeleteBuffers();
  m_glFilterPosData.filterDestPosTexture.DeleteBuffers();
  m_glImageData.mainImageTexture.DeleteBuffers();
  m_glImageData.lowImageTexture.DeleteBuffers();
  m_glFilterBuffData.filterBuff1Texture.DeleteBuffers();
  m_glFilterBuffData.filterBuff2Texture.DeleteBuffers();
  m_glFilterBuffData.filterBuff3Texture.DeleteBuffers();

  m_programPass1UpdateFilterBuff1AndBuff3.DeleteProgram();
  m_programPass2FilterBuff1LuminanceHistogram.DeleteProgram();
  m_programPass3FilterBuff1LuminanceAverage.DeleteProgram();
  m_programPass4ResetFilterBuff2AndOutputBuff3.DeleteProgram();
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
  glGenTextures(1, &m_renderTextureName);
  glBindTexture(GL_TEXTURE_2D, m_renderTextureName);
  glTexStorage2D(GL_TEXTURE_2D, 1, FILTER_BUFF_TEX_INTERNAL_FORMAT, GetWidth(), GetHeight());

  // Bind the texture to the FBO.
  glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTextureName, 0);

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

auto DisplacementFilter::CompileAndLinkShaders() -> void
{
  const auto shaderMacros = std::map<std::string, std::string>{
      {   "FILTER_BUFF1_IMAGE_UNIT",       std::to_string(FILTER_BUFF1_IMAGE_UNIT)},
      {   "FILTER_BUFF2_IMAGE_UNIT",       std::to_string(FILTER_BUFF2_IMAGE_UNIT)},
      {   "FILTER_BUFF3_IMAGE_UNIT",       std::to_string(FILTER_BUFF3_IMAGE_UNIT)},
      {        "LUM_AVG_IMAGE_UNIT",            std::to_string(LUM_AVG_IMAGE_UNIT)},
      {"LUM_HISTOGRAM_BUFFER_INDEX",    std::to_string(LUM_HISTOGRAM_BUFFER_INDEX)},
      {              "ASPECT_RATIO",                 std::to_string(m_aspectRatio)},
      {      "FILTER_POS_MIN_COORD",   std::to_string(NormalizedCoords::MIN_COORD)},
      {    "FILTER_POS_COORD_WIDTH", std::to_string(NormalizedCoords::COORD_WIDTH)},
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
    throw std::runtime_error(std_fmt::format("Could not find output file '{}'", tempShaderFile));
  }

  program.CompileShader(tempShaderFile);
}

auto DisplacementFilter::SetupGlData() -> void
{
  SetupGlSettings();
  SetupGlFilterBuffData();
  SetupGlFilterPosData();
  SetupGlImageData();

  InitFrameDataArrayPointers(m_frameDataArray);
}

auto DisplacementFilter::SetupGlSettings() -> void
{
  glDisable(GL_BLEND);
}

auto DisplacementFilter::Render() noexcept -> void
{
  glViewport(0, 0, GetWidth(), GetHeight());

  Pass1UpdateFilterBuff1AndBuff3();
  //  SaveBuffersAfterPass1();

  Pass2FilterBuff3LuminanceHistogram();

  Pass3FilterBuff3LuminanceAverage();

  Pass4UpdateFilterBuff2AndOutputBuff3();
  //  SaveBuffersAfterPass4();

  Pass5OutputToScreen();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

auto DisplacementFilter::InitAllFrameDataToGl() noexcept -> void
{
  m_programPass1UpdateFilterBuff1AndBuff3.Use();

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
  Expects(m_programPass1UpdateFilterBuff1AndBuff3.IsInUse());

  UpdateFrameDataToGl(pboIndex);
  CheckZeroFilterBuffers();

  m_currentPboIndex = pboIndex;
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

auto DisplacementFilter::Pass1UpdateFilterBuff1AndBuff3() noexcept -> void
{
  m_programPass1UpdateFilterBuff1AndBuff3.Use();

  const auto receivedFrameData = m_requestNextFrameData();

  BindGlFilterPosData();
  BindGlFilterBuff2Data();
  BindGlImageData();

  glBindFramebuffer(GL_FRAMEBUFFER, m_renderToTextureFbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render the full-screen quad
  glBindVertexArray(m_fsQuad);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, NUM_VERTICES);

  if (receivedFrameData)
  {
    m_releaseCurrentFrameData(m_currentPboIndex);
  }

  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

auto DisplacementFilter::Pass4UpdateFilterBuff2AndOutputBuff3() noexcept -> void
{
  m_programPass4ResetFilterBuff2AndOutputBuff3.Use();

  //std_fmt::println("Before av lum {}", __LINE__);
  //m_program.SetUniform("u_averageLuminance", GetLumAverage());
  //std_fmt::println("After av lum {}", __LINE__);
  //std_fmt::println("LumAverage = {}.", GetLumAverage());

  UpdatePass4MiscDataToGl(m_currentPboIndex);

  glBindFramebuffer(GL_FRAMEBUFFER, m_renderToTextureFbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render the full-screen quad
  glBindVertexArray(m_fsQuad);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, NUM_VERTICES);

  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

auto DisplacementFilter::Pass5OutputToScreen() noexcept -> void
{
  glViewport(0, 0, GetFramebufferWidth(), GetFramebufferHeight());

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

auto DisplacementFilter::Pass2FilterBuff3LuminanceHistogram() noexcept -> void
{
  m_programPass2FilterBuff1LuminanceHistogram.Use();

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

auto DisplacementFilter::Pass3FilterBuff3LuminanceAverage() noexcept -> void
{
  m_programPass3FilterBuff1LuminanceAverage.Use();

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
  static constexpr auto MIN_LOG_LUM = -8.0F;
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
  glBindTexture(GL_TEXTURE_2D, m_lumAverageDataTextureName);

  auto lumAverage = 0.0F;
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
  UpdatePass1MiscDataToGl(pboIndex);
  UpdatePosDataToGl(pboIndex);
  UpdateImageDataToGl(pboIndex);
}

auto DisplacementFilter::UpdatePass1MiscDataToGl(const size_t pboIndex) noexcept -> void
{
  //LogInfo(GOOM::UTILS::GetGoomLogger(), "New lerpFactor = {}.", m_previousLerpFactor);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_LERP_FACTOR, m_frameDataArray.at(pboIndex).miscData.lerpFactor);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_BASE_COLOR_MULTIPLIER, m_frameDataArray.at(pboIndex).miscData.baseColorMultiplier);
}

auto DisplacementFilter::UpdatePass4MiscDataToGl(const size_t pboIndex) noexcept -> void
{
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(
      UNIFORM_BRIGHTNESS, m_frameDataArray.at(pboIndex).miscData.brightness);
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
  auto filterSrcePosBuffer = m_glFilterPosData.filterSrcePosTexture.GetMappedBuffer(pboIndex);
  const auto destToSrceLerpFactor =
      m_frameDataArray.at(pboIndex).filterPosArrays.lerpFactorForDestToSrceUpdate;
  std::transform(m_previousFilterDestPos.cbegin(),
                 m_previousFilterDestPos.cend(),
                 filterSrcePosBuffer.begin(),
                 filterSrcePosBuffer.begin(),
                 [&destToSrceLerpFactor](const Point2dFlt& prevDestPos, const Point2dFlt& srcePos)
                 { return lerp(srcePos, prevDestPos, destToSrceLerpFactor); });

  m_glFilterPosData.filterSrcePosTexture.CopyMappedBufferToTexture(pboIndex);
  m_glFilterPosData.filterDestPosTexture.CopyMappedBufferToTexture(pboIndex);

  const auto filterDestPosBuffer = m_glFilterPosData.filterDestPosTexture.GetMappedBuffer(pboIndex);
  std::copy(
      filterDestPosBuffer.begin(), filterDestPosBuffer.end(), m_previousFilterDestPos.begin());

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
  m_glFilterPosData.filterSrcePosTexture.BindTexture(m_programPass1UpdateFilterBuff1AndBuff3);
  m_glFilterPosData.filterDestPosTexture.BindTexture(m_programPass1UpdateFilterBuff1AndBuff3);
}

auto DisplacementFilter::BindGlFilterBuff2Data() noexcept -> void
{
  m_glFilterBuffData.filterBuff2Texture.BindTexture(m_programPass1UpdateFilterBuff1AndBuff3);
}

auto DisplacementFilter::BindGlImageData() noexcept -> void
{
  m_glImageData.mainImageTexture.BindTexture(m_programPass1UpdateFilterBuff1AndBuff3);
  m_glImageData.lowImageTexture.BindTexture(m_programPass1UpdateFilterBuff1AndBuff3);
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
  glGenTextures(1, &m_lumAverageDataTextureName);
  glActiveTexture(LUM_AVG_TEX_UNIT);
  glBindTexture(GL_TEXTURE_2D, m_lumAverageDataTextureName);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16F, 1, 1);
  glBindImageTexture(
      LUM_AVG_IMAGE_UNIT, m_lumAverageDataTextureName, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R16F);

  const auto initialData = 0.5F;
  glBindTexture(GL_TEXTURE_2D, m_lumAverageDataTextureName);
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
