#include "engine/private/window.h"
#include "window.h"
#include "core.h"

engine::Window* engine::Window::Create(const engine::WindowOptions& data) {
  auto window = new engine::p_linux::Window(data);
  return window;
}

namespace engine {
namespace p_linux {

GLFWwindow* CreateWindow(engine::WindowOptions& data) {
  GLFWwindow* window;
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(data.width, data.height, data.title.c_str(), nullptr, nullptr);
  glfwMakeContextCurrent(window);
  glfwSetWindowUserPointer(window, &data);
  return window;
}

Window::Window(const engine::WindowOptions& options): engine::Window(options) {
  this->window = CreateWindow(this->data);
}

Window::~Window() {
  glfwDestroyWindow(this->window);
  glfwTerminate();
}
}
}