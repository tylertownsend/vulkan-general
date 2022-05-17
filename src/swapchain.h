#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <algorithm>

#include "queue_families.h"

namespace VT {
struct SwapChainOptions {
  VkInstance* instance;
  VkSurfaceKHR* surface;
  VkPhysicalDevice* physical_device;
  VkDevice* device;
  GLFWwindow* window;
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct SwapchainInfo {
  VkSwapchainKHR swapchain;
  std::vector<VkImage> swapchain_images;
  VkFormat swapchain_image_format;
  VkExtent2D swapchain_extent;
};

SwapchainInfo CreateSwapchain(SwapChainOptions& options);
bool CheckSwapChainAdequate(bool extensionsSupported, VkPhysicalDevice& device, VkSurfaceKHR& surface);
VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
SwapChainSupportDetails query_swap_chain_support_details(VkPhysicalDevice& device, VkSurfaceKHR& surface);

SwapchainInfo CreateSwapchain(SwapChainOptions& options) {
  VkSwapchainKHR swapchain;
  std::vector<VkImage> swapchain_images;
  VkFormat swapchain_image_format;
  VkExtent2D swapchain_extent;
  SwapChainSupportDetails swapChainSupport = query_swap_chain_support_details(
    *options.physical_device,
    *options.surface);

  VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapChainSupport.formats);
  VkPresentModeKHR presentMode = choose_swap_present_mode(swapChainSupport.presentModes);
  VkExtent2D extent = choose_swap_extent(swapChainSupport.capabilities, options.window);
  // Simply sticking to this minimum means that we may sometimes have to wait on the driver
  // to complete internal operations before we can acquire another image to render to.
  // Therefore it is recommended to request at least one more image than the minimum:
  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = *options.surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  // This is always 1 unless you are developing a stereoscopic 3D application
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = FindQueueFamilies(
    *options.physical_device,
    *options.surface);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  // If the graphics queue family and presentation queue family are the same, which will be
  // the case on most hardware, then we should stick to exclusive mode, because concurrent
  // mode requires you to specify at least two distinct queue families.
  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0; // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  }

  // We can specify that a certain transform should be applied to images in the swap chain
  // if it is supported (supportedTransforms in capabilities), like a 90 degree clockwise
  // rotation or horizontal flip. 
  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

  // The compositeAlpha field specifies if the alpha channel should be used for blending
  // with other windows in the window system. You'll almost always want to simply ignore the
  // alpha channel, hence VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  createInfo.presentMode = presentMode;
  // If the clipped member is set to VK_TRUE then that means that we don't care about the
  // color of pixels that are obscured, for example because another window is in front of
  // them.
  createInfo.clipped = VK_TRUE;

  // For now assume we will have one swap chain
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(*options.device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }
  vkGetSwapchainImagesKHR(*options.device, swapchain, &imageCount, nullptr);
  swapchain_images.resize(imageCount);
  vkGetSwapchainImagesKHR(*options.device, swapchain, &imageCount, swapchain_images.data());

  swapchain_image_format = surfaceFormat.format;
  swapchain_extent = extent;

  return {
    swapchain,
    swapchain_images,
    swapchain_image_format,
    swapchain_extent
  };
}

bool CheckSwapChainAdequate(bool extensionsSupported, VkPhysicalDevice& device, VkSurfaceKHR& surface) {
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = query_swap_chain_support_details(device, surface);
    return !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }
  return false;
}

VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
  for (const auto& availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }
  // If that also fails then we could start ranking the available formats based on how
  // "good" they are, but in most cases it's okay to just settle with the first format that is specified.
  return availableFormats[0];
}

// VK_PRESENT_MODE_MAILBOX_KHR is a very nice trade-off if energy usage is not a concern.
// It allows us to avoid tearing while still maintaining a fairly low latency by rendering new images that are as up-to-date as possible right until the vertical blank. On mobile devices, where energy usage is more important, you will probably want to use VK_PRESENT_MODE_FIFO_KHR instead. 
// Only the VK_PRESENT_MODE_FIFO_KHR mode is guaranteed to be available, so we'll again have to write a function that looks for the best mode that is available:
VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
  for (const auto& availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

SwapChainSupportDetails query_swap_chain_support_details(VkPhysicalDevice& device, VkSurfaceKHR& surface) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}
} // VT