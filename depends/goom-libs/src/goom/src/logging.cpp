#include "logging.h"

#include <format>
#include <string>

namespace GOOM::UTILS
{

void Logging::VLog(const LogLevel lvl,
                   const int lineNum,
                   const std::string& funcName,
                   const std::string& formatStr,
                   const std20::format_args args)
{
  std20::memory_buffer buffer;
  // Pass custom argument formatter as a template arg to vwrite.
  std20::vformat_to(std20::detail::buffer_appender<char>(buffer), formatStr, args);
  Log(lvl, lineNum, funcName, std::string(buffer.data(), buffer.size()));
}

} // namespace GOOM::UTILS
