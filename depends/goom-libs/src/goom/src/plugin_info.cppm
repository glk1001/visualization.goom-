module;

#include "goom/goom_config.h"

export module Goom.PluginInfo;

import Goom.Control.GoomSoundEvents;
import Goom.Utils.GoomTime;
import Goom.Lib.GoomTypes;

export namespace GOOM
{

class PluginInfo
{
public:
  PluginInfo() noexcept = delete;
  PluginInfo(const Dimensions& dimensions,
             const GoomTime& goomTime,
             const CONTROL::GoomSoundEvents& soundEvents) noexcept;
  PluginInfo(const PluginInfo&) noexcept           = delete;
  PluginInfo(PluginInfo&&) noexcept                = delete;
  virtual ~PluginInfo() noexcept                   = default;
  auto operator=(const PluginInfo&) -> PluginInfo& = delete;
  auto operator=(PluginInfo&&) -> PluginInfo&      = delete;

  [[nodiscard]] auto GetDimensions() const -> const Dimensions&;
  [[nodiscard]] auto GetTime() const -> const GoomTime&;
  [[nodiscard]] auto GetSoundEvents() const -> const CONTROL::GoomSoundEvents&;

private:
  Dimensions m_dimensions;
  const GoomTime* m_goomTime;
  const CONTROL::GoomSoundEvents* m_soundEvents;
};

} // namespace GOOM

namespace GOOM
{

inline PluginInfo::PluginInfo(const Dimensions& dimensions,
                              const GoomTime& goomTime,
                              const CONTROL::GoomSoundEvents& soundEvents) noexcept
  : m_dimensions{dimensions}, m_goomTime{&goomTime}, m_soundEvents{&soundEvents}
{
  Expects(dimensions.GetWidth() > 0);
  Expects(dimensions.GetHeight() > 0);
}

inline auto PluginInfo::GetDimensions() const -> const Dimensions&
{
  return m_dimensions;
}

inline auto PluginInfo::GetTime() const -> const GoomTime&
{
  return *m_goomTime;
}

inline auto PluginInfo::GetSoundEvents() const -> const CONTROL::GoomSoundEvents&
{
  return *m_soundEvents;
}

} // namespace GOOM
