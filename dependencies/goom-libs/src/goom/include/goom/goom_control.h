#pragma once

#include "utils/spimpl.h"

#include <cstdint>
#include <string>

namespace GOOM
{

class AudioSamples;
class PixelBuffer;

class GoomControl
{
public:
  static auto GetRandSeed() -> uint64_t;
  static void SetRandSeed(uint64_t seed);

  GoomControl() noexcept = delete;
  GoomControl(uint32_t width, uint32_t height, const std::string& resourcesDirectory);

  void SetScreenBuffer(const std::shared_ptr<PixelBuffer>& buffer);
  void Start();

  /*
   * Update the next goom frame
   *
   * songTitle:
   *      - empty if it is not the start of the song
   *      - only have a value at the start of the song
   */
  void Update(const AudioSamples& s,
              float fps,
              const std::string& songTitle,
              const std::string& message);

  void Finish();

private:
  class GoomControlImpl;
  spimpl::unique_impl_ptr<GoomControlImpl> m_controller;
};

} // namespace GOOM
