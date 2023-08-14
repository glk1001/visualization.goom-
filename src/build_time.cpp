#include "build_time.h"

#include <string>

auto GetBuildTime() -> std::string
{
  static constexpr const char* BUILD_TIME = __DATE__ ", " __TIME__;
  return BUILD_TIME;
}
