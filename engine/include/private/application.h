#pragma once
#include <memory>

#include "engine/private/window.h"
#include "engine/private/window_controller.h"

namespace engine {

class Application {

 public:
  Application();
  virtual ~Application();
  void Run();

 private:
  std::unique_ptr<Window> window_;
  std::unique_ptr<IWindowController> window_controller_;
};


/**
 * @brief Create a Application object which is defined by the client.
 * 
 * @return Application* 
 */
Application* CreateApplication();

} // namespace engine