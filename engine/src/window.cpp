#include "engine/private/window.h"
#include "window.h"
#include "core.h"

engine::IWindowController* engine::IWindowController::Create(EventDispatcher* dispatcher) {
  auto controller = engine::p_linux::WindowController::Create(dispatcher);
  return controller;
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

WindowController* WindowController::Create(EventDispatcher* dispatcher) {
  return new WindowController(dispatcher);
}

WindowController::WindowController(EventDispatcher* dispatcher) {
  dispatcher_ = std::unique_ptr<EventDispatcher>(dispatcher);
}

engine::Window* WindowController::CreateWindow(const engine::WindowOptions& data) {
  auto window = new engine::p_linux::Window(data);
  return window;
}

void WindowController::OnEvent(std::unique_ptr<engine::Window>& window, std::unique_ptr<engine::Event> event) {
  dispatcher_->Offer(std::move(event));
}

} // namespace p_linux
} // namespace engine