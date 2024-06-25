module;

#include <string>

export module Goom.Control.GoomMusicSettingsReactor;

import Goom.Control.GoomAllVisualFx;
import Goom.FilterFx.FilterSettingsService;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.SPimpl;
import Goom.PluginInfo;

using GOOM::FILTER_FX::FilterSettingsService;
using GOOM::UTILS::MATH::IGoomRand;
using GOOM::UTILS::NameValuePairs;

export namespace GOOM::CONTROL
{

class GoomMusicSettingsReactor
{
public:
  GoomMusicSettingsReactor(const PluginInfo& goomInfo,
                           const IGoomRand& goomRand,
                           GoomAllVisualFx& visualFx,
                           FilterSettingsService& filterSettingsService) noexcept;

  auto SetDumpDirectory(const std::string& dumpDirectory) -> void;

  auto Start() -> void;
  auto Finish() -> void;
  auto NewCycle() -> void;

  auto UpdateSettings() -> void;

  [[nodiscard]] auto GetNameValueParams() const -> NameValuePairs;

private:
  class GoomMusicSettingsReactorImpl;
  spimpl::unique_impl_ptr<GoomMusicSettingsReactorImpl> m_pimpl;
};

} // namespace GOOM::CONTROL
