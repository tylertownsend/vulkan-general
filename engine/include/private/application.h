#pragma once
#include <memory>

#include "engine/private/window.h"

namespace engine {

class Application {

 public:
  Application();
  virtual ~Application();
  void Run();

 private:
  Window* window_;
};


/**
 * @brief Create a Application object which is defined by the client.
 * 
 * @return Application* 
 */
Application* CreateApplication();

} // namespace engine