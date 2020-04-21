#include "goom_logging.h"

#include <stdarg.h>
#include <stdio.h>

void (*goom_logger)(int lvl, int line_num, const char* func_name, const char* msg) = NULL;

#ifndef NO_GOOM_LOGGING
static int log_level = GOOM_LOG_LVL_DEBUG;

int goom_log_get_level()
{
  return log_level;
}

void goom_log_set_level(int lvl)
{
  // TODO Do a check!
  log_level = lvl;
}

void goom_log(int lvl, int line_num, const char* func_name, const char* fmt, ...)
{
  if (!goom_logger) {
    return;
  }
  if (lvl < log_level) {
    return;
  }

  va_list args;
  va_start(args, fmt);

  char msg[512];
  vsnprintf(msg, 512, fmt, args);
  goom_logger(lvl, line_num, func_name, msg);

  va_end(args);
}
#endif
