#pragma once
#include <functional>
#include <memory>
#include <string>

#include <GLFW/glfw3.h>

#include "engine/private/events/event_dispatcher.h"
#include "engine/private/events/event.h"
#include "engine/private/window.h"
#include "engine/private/window_controller.h"

namespace engine {
namespace p_linux {
class Window : public engine::Window {
  GLFWwindow* window;

 public:
  Window(const WindowOptions& options);

  ~Window();
};

class WindowController : public engine::IWindowController {
 public:
  void OnEvent(std::unique_ptr<engine::Window>& window, std::unique_ptr<engine::Event> event) override;
  engine::Window* CreateWindow(const engine::WindowOptions& data) override;
  static WindowController* Create(engine::EventDispatcher* dispatcher);

 private:
  WindowController() = delete;
  WindowController(engine::EventDispatcher* dispatcher);
  std::unique_ptr<engine::EventDispatcher> dispatcher_;
};

}
} // namespace engine