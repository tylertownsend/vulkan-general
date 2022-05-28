
#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <optional>
#include <set>

#include "queue_families.h"
#include "swapchain_support.h"

namespace VT {

  VkDevice* CreateLogicalDevice(
      const VT::QueueFamilyIndices& indices,
      const VkPhysicalDevice& physical_device,
      const std::vector<const char*>& validation_layers,
      bool enable_validation_layers,
      const std::vector<const char*>& device_extensions) {

    VkDevice* device;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    createInfo.ppEnabledExtensionNames = device_extensions.data();

    if (enable_validation_layers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
      createInfo.ppEnabledLayerNames = validation_layers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physical_device, &createInfo, nullptr, device) != VK_SUCCESS) {
      throw std::runtime_error("failed to create logical device!");
    }

    // vkGetDeviceQueue(*device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    // vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    return device;
  }
}