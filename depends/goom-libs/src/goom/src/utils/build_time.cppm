module;

#include <string>

export module Goom.Utils.BuildTime;

import Goom.Utils.DateUtils;

export namespace GOOM::UTILS
{

[[nodiscard]] auto GetBuildTime() -> std::string;

} // namespace GOOM::UTILS

module :private;

namespace GOOM::UTILS
{

auto GetBuildTime() -> std::string
{
  static constexpr auto BUILD_TIME     = __DATE__ ", " __TIME__;
  static constexpr auto BUILD_TIME_FMT = "%b %d %Y, %H:%M:%S";
  return GetStandardDateTimeString(BUILD_TIME, BUILD_TIME_FMT);
}

} // namespace GOOM::UTILS
