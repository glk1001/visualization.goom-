#undef NO_LOGGING

#include "goom/goom_logger.h"

#include <catch2/catch_session.hpp>
#include <iostream>
#include <ostream>
#include <string>

import Goom.Utils.DebuggingLogger;
import Goom.Lib.GoomControl;

using Catch::Session;
using GOOM::GoomControl;
using GOOM::GoomLogger;
using GOOM::UTILS::SetGoomLogger;

auto main(int argc, char* argv[]) -> int
{
  // global setup...
  auto goomLogger        = GoomControl::MakeGoomLogger();
  const auto fConsoleLog = [](const GoomLogger::LogLevel, const std::string& str)
  { std::clog << str << "\n"; };
  AddLogHandler(*goomLogger, "console-log", fConsoleLog);
  SetLogLevel(*goomLogger, GoomLogger::LogLevel::INFO);
  SetLogLevelForFiles(*goomLogger, GoomLogger::LogLevel::INFO);
  LogStart(*goomLogger);

  SetGoomLogger(*goomLogger);

  LogInfo(*goomLogger, "Start unit tests...");

  const auto result = Session().run(argc, argv);

  // global clean-up...

  LogStop(*goomLogger);

  return result;
}
