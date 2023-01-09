#pragma once

#include <format>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace GOOM
{

class GoomLogger
{
public:
  enum class LogLevel
  {
    DEBUG,
    INFO,
    WARN,
    // TODO(glk) Make others use 'L_' (MSCV build issue)
    L_ERROR,
    _num // unused, and marks the enum end
  };
  using HandlerFunc = std::function<void(const LogLevel, const std::string&)>;

  GoomLogger(const GoomLogger&) = delete;
  GoomLogger(GoomLogger&&)      = delete;
  virtual ~GoomLogger() noexcept;
  auto operator=(const GoomLogger&) -> GoomLogger& = delete;
  auto operator=(GoomLogger&&) -> GoomLogger&      = delete;

  auto SetLogFile(const std::string& logF) -> void;
  auto AddHandler(const std::string& name, const HandlerFunc& handlerFunc) -> void;
  auto SetShowDateTime(bool val) -> void;
  auto Start() -> void;
  auto Stop() -> void;
  auto Flush() -> void;
  auto Suspend() -> void;
  auto Resume() -> void;
  [[nodiscard]] auto IsLogging() const -> bool;
  [[nodiscard]] auto GetFileLogLevel() const -> LogLevel;
  auto SetFileLogLevel(LogLevel lvl) -> void;
  [[nodiscard]] auto GetHandlersLogLevel() const -> LogLevel;
  auto SetHandlersLogLevel(LogLevel lvl) -> void;

  [[nodiscard]] virtual auto CanLog() const -> bool;
  auto Log(LogLevel lvl, int lineNum, const std::string& funcName, const std::string& msg) -> void;
  template<typename... Args>
  auto Log(LogLevel lvl,
           int lineNum,
           const std::string& funcName,
           const std::string& formatStr,
           const Args&... args) -> void;

protected:
  GoomLogger() noexcept;
  [[nodiscard]] virtual auto GetLogPrefix(LogLevel lvl,
                                          int lineNum,
                                          const std::string& funcName) const -> std::string;

private:
  LogLevel m_cutoffFileLogLevel     = LogLevel::INFO;
  LogLevel m_cutoffHandlersLogLevel = LogLevel::INFO;
  bool m_showDateTime               = true;
  bool m_doLogging                  = false;
  std::string m_logFile{};
  std::vector<std::pair<std::string, HandlerFunc>> m_handlers{};
  std::vector<std::string> m_logEntries{};
  std::mutex m_mutex{};
  auto DoFlush() -> void;
  auto VLog(LogLevel lvl,
            int lineNum,
            const std::string& funcName,
            const std::string& formatStr,
            std20::format_args args) -> void;
};

inline auto GoomLogger::GetFileLogLevel() const -> GoomLogger::LogLevel
{
  return m_cutoffFileLogLevel;
}

inline auto GoomLogger::GetHandlersLogLevel() const -> GoomLogger::LogLevel
{
  return m_cutoffHandlersLogLevel;
}

inline auto GoomLogger::SetLogFile(const std::string& logF) -> void
{
  m_logFile = logF;
}

inline auto GoomLogger::SetShowDateTime(const bool val) -> void
{
  m_showDateTime = val;
}

inline auto GoomLogger::Suspend() -> void
{
  m_doLogging = false;
}

inline auto GoomLogger::Resume() -> void
{
  m_doLogging = true;
}

inline auto GoomLogger::IsLogging() const -> bool
{
  return m_doLogging;
}

inline auto GoomLogger::CanLog() const -> bool
{
  return true;
}

template<typename... Args>
auto GoomLogger::Log(const LogLevel lvl,
                     const int lineNum,
                     const std::string& funcName,
                     const std::string& formatStr,
                     const Args&... args) -> void
{
  VLog(lvl, lineNum, funcName, formatStr, std20::make_format_args(args...));
}

} // namespace GOOM

// NOLINTBEGIN: Tricky logging code - need to improve

#ifdef NO_LOGGING
//#pragma message("Compiling " __FILE__ " with 'NO_LOGGING' = 'ON' - So there will be NO logging.")
#define SetLogFile(logger, logF)
#define AddLogHandler(logger, name, h)
#define SetShowDateTime(logger, val)
#define LogStart(logger)
#define LogStop(logger)
#define LogFlush(logger)
#define LogSuspend(logger)
#define LogResume(logger)
#define IsLogging(logger) false
#define GetLogLevel(logger)
#define SetLogLevel(logger, lvl)
#define LogDebug(logger, ...)
#define LogInfo(logger, ...)
#define LogWarn(logger, ...)
#define LogError(logger, ...)
#else
#pragma message("Compiling " __FILE__ " with 'NO_LOGGING' = 'OFF' - SO THERE WILL BE LOGGING.")
#define SetLogFile(logger, logF) (logger).SetLogFile(logF)
#define AddLogHandler(logger, name, h) (logger).AddHandler((name), (h))
#define SetShowDateTime(logger, val) (logger).SetShowDateTime(val)
#define LogStart(logger) (logger).Start()
#define LogStop(logger) (logger).Stop()
#define LogFlush(logger) (logger).Flush()
#define LogSuspend(logger) (logger).Suspend()
#define LogResume(logger) (logger).Resume()
#define IsLogging(logger) (logger).IsLogging()
#define GetLogLevel(logger) (logger).GetHandlersLogLevel()
#define SetLogLevel(logger, lvl) (logger).SetHandlersLogLevel(lvl)
#define SetLogLevelForFiles(logger, lvl) (logger).SetFileLogLevel(lvl)
#define LogDebug(logger, ...) \
  (logger).Log(GOOM::GoomLogger::LogLevel::DEBUG, __LINE__, __func__, __VA_ARGS__)
#define LogInfo(logger, ...) \
  (logger).Log(GOOM::GoomLogger::LogLevel::INFO, __LINE__, __func__, __VA_ARGS__)
#define LogWarn(logger, ...) \
  (logger).Log(GOOM::GoomLogger::LogLevel::WARN, __LINE__, __func__, __VA_ARGS__)
#define LogError(logger, ...) \
  (logger).Log(GOOM::GoomLogger::LogLevel::L_ERROR, __LINE__, __func__, __VA_ARGS__)
#endif

// NOLINTEND
