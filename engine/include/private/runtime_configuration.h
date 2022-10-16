#pragma once

#include <memory>

#include "log.h"

using LoggerPointer = std::shared_ptr<engine::monitor::ILog>;

namespace engine {

struct RuntimeConfiguration {
  const std::shared_ptr<engine::monitor::ILog> client_logger;
  const std::shared_ptr<engine::monitor::ILog> core_logger;
  RuntimeConfiguration(LoggerPointer client_logger, LoggerPointer core_logger): 
    client_logger(client_logger),
    core_logger(core_logger) {}
};

} // namespace engine
