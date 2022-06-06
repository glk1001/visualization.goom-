#include "build_time.h"

auto GetBuildTime() -> std::string
{
  constexpr const char* BUILD_TIME = __DATE__ ", " __TIME__;
  return BUILD_TIME;
}