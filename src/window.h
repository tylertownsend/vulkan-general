#pragma once
#include <GLFW/glfw3.h>

struct WindowOptions {
  int width;
  int height;
  char* title;
  void* user_pointer;
  GLFWframebuffersizefun framebuffer_resize_callback;
};

GLFWwindow* CreateWindow(WindowOptions& options) {
  GLFWwindow* window;
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(options.width, options.height, options.title, nullptr, nullptr);
  glfwSetWindowUserPointer(window, options.user_pointer);
  glfwSetFramebufferSizeCallback(window, options.framebuffer_resize_callback);
  return window;
}

namespace phx {

class Window {
 private:
  WindowOptions* _options;
  GLFWwindow* _window;

 public:
  Window(WindowOptions* options) {
    _options = options;
    _window = CreateWindow(*_options);
  }

  ~Window() {
    delete _options;
    glfwDestroyWindow(_window);
    glfwTerminate();
  }

  int WindowShouldClose() {
    glfwWindowShouldClose(_window);
  }

  void PollEvents() {
    glfwPollEvents();
  }
};

} // phx