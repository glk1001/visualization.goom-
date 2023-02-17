#include "goom_logger.h"

#include "date_utils.h"
#include "goom_config.h"
#include "utils/enum_utils.h"

#include <format>
#include <ostream>
#include <stdexcept>
#include <string>

namespace GOOM
{

using UTILS::EnumMap;
using UTILS::GetCurrentDateTimeAsString;

static constexpr auto LOG_LEVEL_STR = EnumMap<GoomLogger::LogLevel, const char*>{{{
    {GoomLogger::LogLevel::DEBUG, "Debug"},
    {GoomLogger::LogLevel::INFO, "Info"},
    {GoomLogger::LogLevel::WARN, "Warn"},
    {GoomLogger::LogLevel::ERROR, "Error"},
}}};

GoomLogger::GoomLogger() noexcept
{
  SetFileLogLevel(m_cutoffFileLogLevel);
  SetHandlersLogLevel(m_cutoffHandlersLogLevel);
}

GoomLogger::~GoomLogger() noexcept
{
  Expects(not m_doLogging);
}

auto GoomLogger::VLog(LogLevel lvl,
                      const std::string& funcName,
                      const int lineNum,
                      const std::string& formatStr,
                      std20::format_args args) -> void
{
  std20::memory_buffer buffer;
  // Pass custom argument formatter as a template arg to pass further down.
  std20::vformat_to(std20::detail::buffer_appender<char>(buffer), formatStr, args);
  Log(lvl, lineNum, funcName, std::string(buffer.data(), buffer.size()));
}

auto GoomLogger::Log(const LogLevel lvl,
                     const int lineNum,
                     const std::string& funcName,
                     const std::string& msg) -> void
{
  const auto lock = std::scoped_lock{m_mutex};
  if (not m_doLogging or not CanLog())
  {
    return;
  }

  const auto mainMsg = GetLogPrefix(lvl, lineNum, funcName) + ":" + msg;
  const auto logMsg =
      std::string{not m_showDateTime ? mainMsg : ((GetCurrentDateTimeAsString() + ":") + mainMsg)};

  if (lvl >= m_cutoffFileLogLevel)
  {
    m_logEntries.push_back(logMsg);
  }
  if (lvl >= m_cutoffHandlersLogLevel)
  {
    for (const auto& handler : m_handlers)
    {
      handler.second(lvl, logMsg);
    }
  }
}

auto GoomLogger::GetLogPrefix(const LogLevel lvl,
                              const int lineNum,
                              const std::string& funcName) const -> std::string
{
  return std20::format("{}:{}:{}", funcName, lineNum, LOG_LEVEL_STR[lvl]);
}

auto GoomLogger::Start() -> void
{
  const auto lock = std::scoped_lock{m_mutex};
  m_doLogging     = true;
  m_logEntries.clear();
}

auto GoomLogger::Stop() -> void
{
  const auto lock = std::scoped_lock{m_mutex};
  m_doLogging     = false;
  DoFlush();
}

auto GoomLogger::SetFileLogLevel(const LogLevel lvl) -> void
{
  m_cutoffFileLogLevel = lvl;
}

auto GoomLogger::SetHandlersLogLevel(const LogLevel lvl) -> void
{
  m_cutoffHandlersLogLevel = lvl;
}

auto GoomLogger::AddHandler(const std::string& name, const HandlerFunc& handlerFunc) -> void
{
  for (const auto& handler : m_handlers)
  {
    if (handler.first == name)
    {
      return;
    }
  }
  m_handlers.emplace_back(name, handlerFunc);
}

auto GoomLogger::Flush() -> void
{
  const auto lock = std::scoped_lock{m_mutex};
  DoFlush();
}

auto GoomLogger::DoFlush() -> void
{
  if (m_logFile.empty())
  {
    return;
  }
  if (m_logEntries.empty())
  {
    return;
  }

  auto logFile = std::ofstream{m_logFile, std::ios::out | std::ios::app};
  if (!logFile.good())
  {
    throw std::runtime_error("Could not open log file \"" + m_logFile + "\".");
  }
  for (const auto& entry : m_logEntries)
  {
    logFile << entry << "\n";
  }

  m_logEntries.clear();
}

} // namespace GOOM
