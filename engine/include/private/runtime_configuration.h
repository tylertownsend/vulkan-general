#pragma once

#include <memory>

#include "log.h"

namespace engine {

struct RuntimeConfiguration {
  std::shared_ptr<engine::monitor::ILog> clientLogger;
  std::shared_ptr<engine::monitor::ILog> coreLogger;
};

} // namespace engine
