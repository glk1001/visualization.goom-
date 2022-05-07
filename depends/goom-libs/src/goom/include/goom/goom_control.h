#pragma once

#include "spimpl.h"

#include <cstdint>
#include <string>

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
  auto SetShowTitle(ShowTitleType value) -> void;

  auto Start() -> void;
  auto Finish() -> void;

  auto SetScreenBuffer(const std::shared_ptr<PixelBuffer>& buffer) -> void;
  auto NoZooms(bool value) -> void;
  auto ShowGoomState(bool value) -> void;
  auto SetDumpDirectory(const std::string& dumpDirectory) -> void;

  /*
   * Update the next goom frame
   *
   * songTitle:
   *      - empty if it is not the start of the song
   *      - only have a value at the start of the song
   */
  auto Update(const AudioSamples& audioSamples,
              float fps,
              const std::string& songTitle,
              const std::string& message) -> void;

  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

  [[nodiscard]] static auto GetCompilerVersion() -> std::string;
  [[nodiscard]] static auto GetGoomVersionInfo() -> std::string;
  [[nodiscard]] static auto GetGoomLibBuildTime() -> std::string;

private:
  class GoomControlImpl;
  spimpl::unique_impl_ptr<GoomControlImpl> m_controller;
};

} // namespace GOOM
