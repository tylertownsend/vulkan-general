#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <memory>

#include "image.h"
#include "swapchain_utils.h"
#include "vulkan.h"
#include "window.h"

namespace VT {

class Swapchain {
  VkSwapchainKHR _swapchain;
  std::vector<VkImage> _swapchain_images;
  std::vector<VkImageView> _swapchain_image_views;
  VkFormat _swapchain_image_format;
  VkExtent2D _swapchain_extent;

  const std::shared_ptr<VT::Vulkan> _instance;
public:
  Swapchain(
      const std::shared_ptr<VT::Vulkan>& instance,
      const std::unique_ptr<phx::Window>& window): _instance(instance) {
    create_swapchain(window);
    create_image_views();
  }

  ~Swapchain() {
    cleanup_swap_chain();
  }

  VkSwapchainKHR& GetSwapchain() {
    return _swapchain;
  }

  std::vector<VkImage>& GetSwapChainImages() {
    return _swapchain_images;
  }

  std::vector<VkImageView>& GetSwapChainImageViews() {
    return _swapchain_image_views;
  }

  VkFormat& GetImageFormat() {
    return _swapchain_image_format;
  }

  VkExtent2D& GetExtent() {
    return _swapchain_extent;
  }

private:
  void create_swapchain(const std::unique_ptr<phx::Window>& window) {
    VT::Vulkan* instance = _instance.get();
    VT::SwapChainOptions options{
      instance->GetVkInstance(),
      instance->GetVkSurfaceKHR(),
      instance->GetVkPhysicalDevice(),
      instance->GetVkDevice(),
      window->GetGLFWwindow()
    };
    auto swapchainInfo = VT::CreateSwapchain(options);
    _swapchain = swapchainInfo.swapchain;
    _swapchain_images = swapchainInfo.swapchain_images;
    _swapchain_image_format = swapchainInfo.swapchain_image_format;
    _swapchain_extent = swapchainInfo.swapchain_extent;
  }

  void create_image_views() {
    _swapchain_image_views.resize(_swapchain_images.size());

    for (uint32_t i = 0; i < _swapchain_images.size(); i++) {
      VT::ImageViewOptions options{};
      options.device = _instance->GetVkDevice();
      options.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
      options.format = _swapchain_image_format;
      options.image = _swapchain_images[i];
      _swapchain_image_views[i] = VT::CreateImageView(options);
    }
  }

  void cleanup_swap_chain() {
    auto device = _instance->GetVkDevice();

    for (size_t i = 0; i < _swapchain_image_views.size(); i++) {
      vkDestroyImageView(device, _swapchain_image_views[i], nullptr);
    }
    vkDestroySwapchainKHR(device, _swapchain, nullptr);
  }
};
} // VT