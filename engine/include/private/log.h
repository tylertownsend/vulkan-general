#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace engine::monitor {

class ILog {
 public:
  virtual void Info(std::string_view fmt) = 0;
  virtual void Error(std::string_view fmt) = 0;
  virtual void Trace(std::string_view fmt) = 0;
  virtual void Warn(std::string_view fmt) = 0;
};

enum LoggerType {
  CoreLogger,
  ClientLogger
};

struct LoggerOptions {
  const std::string name;

  LoggerOptions(std::string name) :
    name(name) {}
};

std::shared_ptr<ILog> Create(LoggerType type, const LoggerOptions& options);

} // namespace engine::monitor

// #define ENGINE_CORE_ERROR(...) ::engine::Logger::Error(__VA_ARGS__);