#pragma once
#include <string>
#include <functional>

#include <GLFW/glfw3.h>

#include "engine/private/window.h"

namespace engine {
namespace p_linux {
class Window : public engine::Window {
  GLFWwindow* window;

 public:
  Window(const WindowOptions& options);

  ~Window();
};
}
} // namespace engine