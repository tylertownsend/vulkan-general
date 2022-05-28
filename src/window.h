#pragma once
#include <GLFW/glfw3.h>

struct WindowOptions {
  int width;
  int height;
  const char* title;
  void* user_pointer;
  GLFWframebuffersizefun framebuffer_resize_callback;
  WindowOptions(){}
  WindowOptions(WindowOptions& other) {
    *this = other;
  }
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
  WindowOptions _options;
  GLFWwindow* _window;
  bool _frame_buffer_resized;

 public:
  Window(WindowOptions& options) {
    options.user_pointer = this;
    options.framebuffer_resize_callback;
    _options = options;
    _window = CreateWindow(_options);
    _frame_buffer_resized = false;
  }

  ~Window() {
    glfwDestroyWindow(_window);
    glfwTerminate();
  }

  GLFWwindow* GetGLFWwindow() {
    return _window;
  }

  int WindowShouldClose() {
    return glfwWindowShouldClose(_window);
  }

  void PollEvents() {
    glfwPollEvents();
  }

  bool FrameBufferResized() {
    return _frame_buffer_resized;
  }

  void ResetFrameBuffer() {
    _frame_buffer_resized = false;
  }

  // Minimization results in a framebuffer of size 0.
  // Tutorial mode :) - we pause process until window is in foreground;
  void HandleMinimization() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(_window, &width, &height);
    while (width == 0 || height == 0) {
      glfwGetFramebufferSize(_window, &width, &height);
      glfwWaitEvents();
    }
  }

  static void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->_frame_buffer_resized = true;
  }
};

} // phx