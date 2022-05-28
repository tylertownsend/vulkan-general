#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

struct VulkanSurfaceOptions {
  VkInstance instance;
  GLFWwindow* window;
};

class VulkanSurface {
public:
  static VkSurfaceKHR* CreateSurface(VulkanSurfaceOptions& options, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(options.instance, (options.window), nullptr, surface) != VK_SUCCESS) {
      throw std::runtime_error("failed to create window surface!");
    }
    return surface;
  }
};