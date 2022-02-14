#pragma once

#include "spimpl.h"

#include <cstdint>

namespace GOOM
{

class AudioSamples;
class PixelBuffer;
struct GoomShaderEffects;

class GoomControl
{
public:
  static auto GetRandSeed() -> uint64_t;
  static void SetRandSeed(uint64_t seed);

  GoomControl() noexcept = delete;
  GoomControl(uint32_t width, uint32_t height, const std::string& resourcesDirectory);

  enum class ShowTitleType
  {
    AT_START,
    ALWAYS,
    NEVER,
  };
  void SetShowTitle(ShowTitleType value);

  void Start();

  /*
   * Update the next goom frame
   *
   * songTitle:
   *      - empty if it is not the start of the song
   *      - only have a value at the start of the song
   */
  void SetScreenBuffer(const std::shared_ptr<PixelBuffer>& buffer);
  void Update(const AudioSamples& audioSamples,
              float fps,
              const std::string& songTitle,
              const std::string& message);

  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

  void Finish();

  [[nodiscard]] static auto GetCompilerVersion() -> std::string;
  [[nodiscard]] static auto GetGoomVersionInfo() -> std::string;

private:
  class GoomControlImpl;
  spimpl::unique_impl_ptr<GoomControlImpl> m_controller;
};

} // namespace GOOM
