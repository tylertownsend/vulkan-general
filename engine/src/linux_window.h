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
 public:
  GLFWwindow* window;
  Window(const engine::WindowOptions& data, GLFWwindow* window): 
    window(window),
    engine::Window(data) {}

  ~Window();
};

class WindowController : public engine::IWindowController {
 public:
  void OnUpdate(std::unique_ptr<engine::Window>& window) override;
  engine::Window* CreateWindow(const engine::WindowOptions& options) override;
  static WindowController* Create();

 private:
  WindowController() {}
  GLFWwindow* create_glfw_window(const engine::WindowOptions& options);
};

}
} // namespace engine