#include <stdarg.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "private/log.h"

namespace engine::monitor {

bool ENABLED = false;

enum LogLevel {
  Trace
};

spdlog::level::level_enum get_level(LogLevel level);

// string view to take ownership of the data
class Logger : public ILog {
 private:
  std::shared_ptr<spdlog::logger> _spdlog;

 public:
  Logger(std::string logger_name, LogLevel level) {
    auto spdlog_level = get_level(level);
    _spdlog = spdlog::stdout_color_mt(logger_name);
    _spdlog->set_level(spdlog_level);
  }

  ~Logger() {}

  void Error(std::string_view fmt) {
    _spdlog->error(fmt);
  }

  void Info(std::string_view fmt) {
    _spdlog->info(fmt);
  }


  void Trace(std::string_view fmt) {
    _spdlog->trace(fmt);
  }

  void Warn(std::string_view fmt) {
    _spdlog->warn(fmt);
  }
};

std::shared_ptr<ILog> Create(LoggerType type, const LoggerOptions& options) {
  if (!ENABLED) {
    ENABLED = true;
    spdlog::set_pattern("%^[%T] %n: %v%$");
  }
  switch(type) {
    case LoggerType::ClientLogger: return std::make_shared<Logger>(options.name, LogLevel::Trace);
    default:                       return std::make_shared<Logger>(options.name, LogLevel::Trace);
  }
}

spdlog::level::level_enum get_level(LogLevel level) {
  switch(level) {
    case LogLevel::Trace: return spdlog::level::trace;
    default:              return spdlog::level::trace;
  }
}
} // namespace engine::monitor