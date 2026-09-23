#pragma once

#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <string>

enum class LogLevel : int {
  Off = 0,
  Error = 1,
  Warn = 2,
  Info = 3,
  Debug = 4,
  Trace = 5,
};

inline LogLevel g_logLevel = LogLevel::Info;
inline FILE* g_logFile = nullptr;

inline void logSetLevel(LogLevel level) { g_logLevel = level; }
inline void logSetFile(FILE* f) { g_logFile = f; }

inline void logMsg(LogLevel level, const char* fmt, ...) {
  if (level > g_logLevel) return;

  const char* prefix = "";
  switch (level) {
    case LogLevel::Error: prefix = "[ERROR] "; break;
    case LogLevel::Warn:  prefix = "[WARN]  "; break;
    case LogLevel::Info:  prefix = "[INFO]  "; break;
    case LogLevel::Debug: prefix = "[DEBUG] "; break;
    case LogLevel::Trace: prefix = "[TRACE] "; break;
    default: break;
  }

  char buf[4096];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if (g_logFile) {
    fprintf(g_logFile, "%s%s\n", prefix, buf);
    fflush(g_logFile);
  }
  fprintf(stderr, "%s%s\n", prefix, buf);
}

#define VKWIND11_LOG(level, ...) logMsg(LogLevel::level, __VA_ARGS__)
#define VKWIND11_LOG_ERROR(...) VKWIND11_LOG(Error, __VA_ARGS__)
#define VKWIND11_LOG_WARN(...)  VKWIND11_LOG(Warn, __VA_ARGS__)
#define VKWIND11_LOG_INFO(...)  VKWIND11_LOG(Info, __VA_ARGS__)
#define VKWIND11_LOG_DEBUG(...) VKWIND11_LOG(Debug, __VA_ARGS__)
#define VKWIND11_LOG_TRACE(...) VKWIND11_LOG(Trace, __VA_ARGS__)

void logInitFromEnv();
