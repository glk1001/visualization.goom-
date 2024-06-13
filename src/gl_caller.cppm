module;

#undef NO_LOGGING

#include "goom/goom_logger.h"

#include <concepts>
#include <print>
#include <source_location>
#include <utility>

export module Goom.GlCaller;

import Goom.GoomVisualization.GlUtils;
import Goom.Lib.AssertUtils;

static constexpr auto DEBUG_ALL_CALLS = false;

#ifdef GOOM_DEBUG
static constexpr auto WRAP_GL_CALL = true;
#else
static constexpr auto WRAP_GL_CALL = false;
#endif

export namespace GOOM::OPENGL
{

class GlCaller
{
public:
  explicit GlCaller(GoomLogger& goomLogger);

  [[nodiscard]] auto Call(std::source_location loc = std::source_location::current()) const;

private:
  GoomLogger* m_goomLogger;

  class Caller
  {
  public:
    explicit Caller(GoomLogger& goomLogger, std::source_location loc);
    template<typename... Args>
    auto operator()(std::invocable<Args...> auto glFunc, Args... args);

  private:
    GoomLogger* m_goomLogger;
    std::source_location m_location;

    std::string m_allArgs;

    template<typename... Args>
    auto SetAllArgsStr(Args... args) -> void;

    [[nodiscard]] static auto GetStr(auto arg) -> std::string;
    [[nodiscard]] static auto GetStr(auto* arg) -> std::string;

    auto ConcatArgs(auto arg) -> void;
    template<typename T, typename... Args>
    auto ConcatArgs(T item, Args... args) -> void;
  };
};

} // namespace GOOM::OPENGL

namespace GOOM::OPENGL
{

inline GlCaller::GlCaller(GoomLogger& goomLogger) : m_goomLogger{&goomLogger}
{
}

inline auto GlCaller::Call(std::source_location loc) const
{
  return Caller{*m_goomLogger, loc};
}

inline GlCaller::Caller::Caller(GoomLogger& goomLogger, const std::source_location loc)
  : m_goomLogger{&goomLogger}, m_location{loc}
{
}

template<typename... Args>
auto GlCaller::Caller::operator()(std::invocable<Args...> auto glFunc, Args... args)
{
  if constexpr (not WRAP_GL_CALL)
  {
    glFunc(std::forward<Args>(args)...);
  }
  else
  {
    GlClearError();

    if constexpr (DEBUG_ALL_CALLS)
    {
      SetAllArgsStr(std::forward<Args>(args)...);

      LogInfo(*m_goomLogger,
              "Calling OpenGL function at line {}, in file '{}'. Args = \"{}\".",
              m_location.line(),
              m_location.file_name(),
              m_allArgs);
    }

    glFunc(std::forward<Args>(args)...);

    if constexpr (DEBUG_ALL_CALLS)
    {
      LogInfo(*m_goomLogger,
              "Finished OpenGL function call at line {}, in file '{}'.",
              m_location.line(),
              m_location.file_name());
    }

    if (auto message = std::string{}; not CheckForOpenGLError(message))
    {
      SetAllArgsStr(std::forward<Args>(args)...);

      const auto errorMsg =
          std::format("OpenGL error: {}. At line {}, in file '{}'. Args = \"{}\".",
                      message,
                      m_location.line(),
                      m_location.file_name(),
                      m_allArgs);

      LogError(*m_goomLogger, errorMsg);
      std::println(stderr, "{}", errorMsg);

      std::terminate();
    }
  }
}

template<typename... Args>
auto GlCaller::Caller::SetAllArgsStr(Args... args) -> void
{
  m_allArgs = "";
  ConcatArgs(std::forward<Args>(args)...);
}

inline auto GlCaller::Caller::GetStr(const auto arg) -> std::string
{
  return std::format("{}", arg);
}

inline auto GlCaller::Caller::GetStr(auto* const arg) -> std::string
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return std::format("{}", reinterpret_cast<uint64_t>(arg));
}

auto GlCaller::Caller::ConcatArgs(const auto arg) -> void
{
  if (m_allArgs.empty())
  {
    m_allArgs = std::format("{}", GetStr(arg));
  }
  else
  {
    m_allArgs = std::format("{}, {}", m_allArgs, GetStr(arg));
  }
}

template<typename T, typename... Args>
auto GlCaller::Caller::ConcatArgs(const T item, Args... args) -> void
{
  ConcatArgs(item);
  ConcatArgs(args...);
}

} // namespace GOOM::OPENGL
