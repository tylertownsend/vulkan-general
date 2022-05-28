#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <optional>
#include <set>

#include "queue_families.h"
#include "swapchain.h"

// const std::vector<const char*> deviceExtensions = {
//   VK_KHR_SWAPCHAIN_EXTENSION_NAME
// };

namespace VT {

uint32_t FindMemoryType(
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties,
    VkPhysicalDevice physicalDevice) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("failed to find suitable memory type!");
}

std::vector<const char*> get_required_extensions(bool enable_validation_layers) {
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (enable_validation_layers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& device_extensions, bool enable_validation_layers) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(device_extensions.begin(), device_extensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

VT::QueueFamilyIndices IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& device_extensions, bool enable_validation_layers) {
  VT::QueueFamilyIndices indices = VT::FindQueueFamilies(device, surface);

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

  bool extensionsSupported = VT::CheckDeviceExtensionSupport(device, device_extensions, enable_validation_layers);
  bool swapChainAdequate = VT::CheckSwapChainAdequate(extensionsSupported, device, surface);

  if (indices.IsComplete() && 
      extensionsSupported && 
      swapChainAdequate && 
      supportedFeatures.samplerAnisotropy) {
    return indices;
  } else {
    VT::QueueFamilyIndices incomplete_indices;
    return incomplete_indices;
  }
}

VkPhysicalDevice* PickPhysicalDevice(const VkInstance instance, const VkSurfaceKHR surface, const std::vector<const char*>& device_extensions, bool enable_validation_layers) {
  uint32_t deviceCount = 0;
  VkPhysicalDevice* physical_device = VK_NULL_HANDLE;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (auto& device : devices) {
    auto indices = IsDeviceSuitable(device, surface, device_extensions, enable_validation_layers);
    if (indices.IsComplete()) {
      physical_device = &device;
      break;
    }
  }

  if (physical_device == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
  return physical_device;
}
}
