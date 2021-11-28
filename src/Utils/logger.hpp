#pragma once

#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace MS {

// define logger
class MetaLog {
public:
  static void init();
  MetaLog() = default;
  virtual ~MetaLog() = default;

  inline static const std::shared_ptr<spdlog::logger>& get_logger() {
    if (!s_logger) { init(); }
    return s_logger;
  }

private:
  static std::shared_ptr<spdlog::logger> s_logger;
};

}   // namespace MS

#ifndef META_NO_DEBUG

// Client log macros
#  define META_FATAL(...) ::MS::MetaLog::get_logger()->fatal(__VA_ARGS__)
#  define META_ERROR(...) ::MS::MetaLog::get_logger()->error(__VA_ARGS__)
#  define META_WARN(...) ::MS::MetaLog::get_logger()->warn(__VA_ARGS__)
#  define META_INFO(...) ::MS::MetaLog::get_logger()->info(__VA_ARGS__)
#  define META_TRACE(...) ::MS::MetaLog::get_logger()->trace(__VA_ARGS__)

#  define META_ASSERT(condition, ...) \
    do {                              \
      if (!(condition)) {             \
        META_ERROR(__VA_ARGS__);      \
        assert(condition);            \
      }                               \
    } while (false)

#else

#  define META_FATAL(...)
#  define META_ERROR(...)
#  define META_WARN(...)
#  define META_INFO(...)
#  define META_TRACE(...)

#  define META_ASSERT(condition, ...)

#endif