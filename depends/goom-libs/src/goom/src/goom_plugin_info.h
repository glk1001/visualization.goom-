#pragma once

#include "control/goom_sound_events.h"

#include <cstdint>
#include <memory>

namespace GOOM
{

class PluginInfo
{
public:
  struct Screen
  {
    uint32_t width;
    uint32_t height;
    uint32_t size; // == screen.height * screen.width.
  };

  PluginInfo() noexcept = delete;
  PluginInfo(uint32_t width, uint32_t height, const CONTROL::GoomSoundEvents& soundEvents) noexcept;
  PluginInfo(const PluginInfo&) noexcept           = delete;
  PluginInfo(PluginInfo&&) noexcept                = delete;
  virtual ~PluginInfo() noexcept                   = default;
  auto operator=(const PluginInfo&) -> PluginInfo& = delete;
  auto operator=(PluginInfo&&) -> PluginInfo&      = delete;

  [[nodiscard]] auto GetScreenInfo() const -> const Screen&;
  [[nodiscard]] auto GetSoundEvents() const -> const CONTROL::GoomSoundEvents&;

private:
  const Screen m_screen;
  const CONTROL::GoomSoundEvents& m_soundEvents;
};

inline PluginInfo::PluginInfo(const uint32_t width,
                              const uint32_t height,
                              const CONTROL::GoomSoundEvents& soundEvents) noexcept
  : m_screen{width, height, width * height}, m_soundEvents{soundEvents}
{
}

inline auto PluginInfo::GetScreenInfo() const -> const PluginInfo::Screen&
{
  return m_screen;
}

inline auto PluginInfo::GetSoundEvents() const -> const CONTROL::GoomSoundEvents&
{
  return m_soundEvents;
}

} // namespace GOOM
