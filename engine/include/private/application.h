#pragma once

#include <memory>

#include "runtime_configuration.h"

namespace engine {

class Application {

 public:
  Application();
  virtual ~Application();
  void Run();

 private:
  std::unique_ptr<RuntimeConfiguration> runtime_configuration_;
};


/**
 * @brief Create a Application object which is defined by the client.
 * 
 * @return Application* 
 */
Application* CreateApplication();

} // namespace engine