#include "logging.h"
#include <cstring>

void logInitFromEnv() {
  const char* level = getenv("VKWIND11_LOG_LEVEL");
  if (level) {
    if (strcmp(level, "off") == 0) g_logLevel = LogLevel::Off;
    else if (strcmp(level, "error") == 0) g_logLevel = LogLevel::Error;
    else if (strcmp(level, "warn") == 0) g_logLevel = LogLevel::Warn;
    else if (strcmp(level, "info") == 0) g_logLevel = LogLevel::Info;
    else if (strcmp(level, "debug") == 0) g_logLevel = LogLevel::Debug;
    else if (strcmp(level, "trace") == 0) g_logLevel = LogLevel::Trace;
  }

  const char* logFile = getenv("VKWIND11_LOG_FILE");
  if (logFile) {
    g_logFile = fopen(logFile, "w");
    if (g_logFile) {
      logMsg(LogLevel::Info, "Logging to %s", logFile);
    }
  }
}
