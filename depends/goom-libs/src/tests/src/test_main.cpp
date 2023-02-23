#undef NO_LOGGING

#include "goom_control.h"
#include "goom_logger.h"

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <ostream>
#include <string>

using Catch::Session;
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

  const auto result = Session().run(argc, argv);

  // global clean-up...

  LogStop(*goomLogger);

  return result;
}
