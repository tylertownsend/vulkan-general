#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

#include "vulkan.h"
#include "command_pool.h"
#include "renderpass.h"
#include "image.h"

namespace VT {
class DepthResources { 
 private:
  const std::shared_ptr<Vulkan>& _instance;

  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

 public:
  DepthResources(
      const std::shared_ptr<Vulkan>& instance, 
      const std::unique_ptr<VT::CommandPool>& command_pool,
      VkExtent2D swap_extent): _instance(instance) {
    create_depth_resources(command_pool, swap_extent);
  }

  ~DepthResources() {
    auto device = _instance->GetVkDevice();
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
  }

  VkImageView& GetDepthImageView() {
    return depthImageView;
  }

 private:
  void create_depth_resources(const std::unique_ptr<VT::CommandPool>& command_pool, VkExtent2D& swap_extent) {
    // TODO: find_depth_format should probably be initialied before this and renderpass
    // are called.
    VkFormat depthFormat = VT::find_depth_format(_instance->GetVkPhysicalDevice());

    VT::CreateImageOptions imageOptions(
      swap_extent.width,
      swap_extent.height,
      depthFormat,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      _instance->GetVkDevice(),
      _instance->GetVkPhysicalDevice()
    );
    VT::CreateImage(imageOptions, depthImage, depthImageMemory);

    VT::ImageViewOptions options{};
    options.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    options.image = depthImage;
    options.format = depthFormat;
    options.device = _instance.get()->GetVkDevice();
    depthImageView = VT::CreateImageView(options);
    VT::TransitionImageLayout(_instance->GetVkDevice(),
                              command_pool->GetCommandPool(),
                              _instance->GetGraphicsQueue(),
                              depthImage,
                              depthFormat,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  }
};
}