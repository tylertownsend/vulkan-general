#pragma once

#include <memory>

#include "runtime_configuration.h"
#include "engine/private/events/event_dispatcher.h"
#include "engine/private/window.h"
#include "engine/private/window_controller.h"

namespace engine {

class Application {

 public:
  Application();
  virtual ~Application();
  void Run();

 private:
  std::unique_ptr<RuntimeConfiguration> runtime_configuration_ = nullptr;
  std::unique_ptr<Window> window_ = nullptr;
  std::unique_ptr<IWindowController> window_controller_ = nullptr;
  std::unique_ptr<EventDispatcher> event_dispatcher_ = nullptr;
};


/**
 * @brief Create a Application object which is defined by the client.
 * 
 * @return Application* 
 */
Application* CreateApplication();

} // namespace engine