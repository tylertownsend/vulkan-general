#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <optional>
#include <set>

// const std::vector<const char*> deviceExtensions = {
//   VK_KHR_SWAPCHAIN_EXTENSION_NAME
// };

// struct QueueFamilyIndices {
//   std::optional<uint32_t> graphicsFamily;
//   std::optional<uint32_t> presentFamily;

//   bool IsComplete() {
//     return graphicsFamily.has_value() && presentFamily.has_value();
//   }
// };

// struct SwapChainSupportDetails {
//   VkSurfaceCapabilitiesKHR capabilities;
//   std::vector<VkSurfaceFormatKHR> formats;
//   std::vector<VkPresentModeKHR> presentModes;
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
}

// class VulkanDevice {
//   VkPhysicalDevice* PickPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface) {
//     uint32_t deviceCount = 0;
//     VkPhysicalDevice* physical_device = VK_NULL_HANDLE;
//     vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

//     if (deviceCount == 0) {
//       throw std::runtime_error("failed to find GPUs with Vulkan support!");
//     }

//     std::vector<VkPhysicalDevice> devices(deviceCount);
//     vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

//     for (auto& device : devices) {
//       if (is_device_suitable(device, surface)) {
//         physical_device = &device;
//         break;
//       }
//     }

//     if (physical_device == VK_NULL_HANDLE) {
//       throw std::runtime_error("failed to find a suitable GPU!");
//     }
//     return physical_device;
//   }

//   VkDevice* CreateLogicalDevice(
//       const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
//       const VkPhysicalDevice& physical_device,
//       bool enable_validation_layers,
//       const std::vector<char*>& validation_layers) {
//     VkDevice* device;
//     VkPhysicalDeviceFeatures deviceFeatures{};
//     deviceFeatures.samplerAnisotropy = VK_TRUE;

//     VkDeviceCreateInfo createInfo{};
//     createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//     createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
//     createInfo.pQueueCreateInfos = queueCreateInfos.data();
//     createInfo.pEnabledFeatures = &deviceFeatures;
//     createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
//     createInfo.ppEnabledExtensionNames = deviceExtensions.data();

//     if (enable_validation_layers) {
//       createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
//       createInfo.ppEnabledLayerNames = validation_layers.data();
//     } else {
//       createInfo.enabledLayerCount = 0;
//     }

//     if (vkCreateDevice(physical_device, &createInfo, nullptr, device) != VK_SUCCESS) {
//       throw std::runtime_error("failed to create logical device!");
//     }
//     return device;
//   }

//   bool is_device_suitable(VkPhysicalDevice device, const VkSurfaceKHR& surface) {
//     QueueFamilyIndices indices = find_queue_families(device, surface);

//     VkPhysicalDeviceFeatures supportedFeatures;
//     vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

//     bool extensionsSupported = check_device_extension_support(device);
//     bool swapChainAdequate = check_swap_chain_adequate(extensionsSupported, device, surface);

//     return indices.IsComplete() && 
//            extensionsSupported && 
//            swapChainAdequate && 
//            supportedFeatures.samplerAnisotropy;
//   }

//   QueueFamilyIndices find_queue_families(VkPhysicalDevice device, const VkSurfaceKHR& surface) {
//     QueueFamilyIndices indices;

//     uint32_t queueFamilyCount = 0;
//     vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

//     std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
//     vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

//     int i = 0;
//     for (const auto& queueFamily : queueFamilies) {
//       if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
//         indices.graphicsFamily = i;
//       }

//       VkBool32 presentSupport = false;
//       vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

//       if (presentSupport) {
//         indices.presentFamily = i;
//       }

//       if (indices.IsComplete()) {
//         break;
//       }

//       i++;
//     }

//     return indices;
//   }

//   bool check_device_extension_support(VkPhysicalDevice device) {
//     uint32_t extensionCount;
//     vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

//     std::vector<VkExtensionProperties> availableExtensions(extensionCount);
//     vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

//     std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

//     for (const auto& extension : availableExtensions) {
//       requiredExtensions.erase(extension.extensionName);
//     }

//     return requiredExtensions.empty();
//   }

//   bool check_swap_chain_adequate(bool extensionsSupported, VkPhysicalDevice device, const VkSurfaceKHR& surface) {
//     if (extensionsSupported) {
//       SwapChainSupportDetails swapChainSupport = query_swap_chain_support_details(device, surface);
//       return !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
//     }
//     return false;
//   }

//   SwapChainSupportDetails query_swap_chain_support_details(VkPhysicalDevice device, const VkSurfaceKHR& surface) {
//     SwapChainSupportDetails details;

//     vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

//     uint32_t formatCount;
//     vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
//     if (formatCount != 0) {
//       details.formats.resize(formatCount);
//       vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
//     }

//     uint32_t presentModeCount;
//     vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
//     if (presentModeCount != 0) {
//       details.presentModes.resize(presentModeCount);
//       vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
//     }

//     return details;
//   }
// };