#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <chrono>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>
#include <algorithm>
#include <unordered_map>

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