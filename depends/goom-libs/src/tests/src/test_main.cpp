#define CATCH_CONFIG_RUNNER
//#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#undef NO_LOGGING

#include "catch2/catch.hpp"
#include "goom_control.h"
#include "goom_logger.h"

#include <iostream>
#include <ostream>
#include <string>

using GOOM::GoomControl;
using GOOM::GoomLogger;

auto main(int argc, char* argv[]) -> int
{
  // global setup...
  auto goomLogger        = GoomControl::MakeGoomLogger();
  const auto fConsoleLog = [](GoomLogger::LogLevel, const std::string& str)
  { std::clog << str << std::endl; };
  AddLogHandler(*goomLogger, "console-log", fConsoleLog);
  SetLogLevel(*goomLogger, GoomLogger::LogLevel::INFO);
  SetLogLevelForFiles(*goomLogger, GoomLogger::LogLevel::INFO);
  LogStart(*goomLogger);

  LogInfo(*goomLogger, "Start unit tests...");

  const auto result = Catch::Session().run(argc, argv);

  // global clean-up...

  LogStop(*goomLogger);

  return result;
}
