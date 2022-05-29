#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "renderpass.h"
#include "image.h"

namespace VT {
class DepthResources { 
 public:
  DepthResources() {

  }

  void CreateDepthResources() {
    // TODO: find_depth_format should probably be initialied before this and renderpass
    // are called.
    VkFormat depthFormat = VT::find_depth_format(physicalDevice);

    VT::CreateImageOptions imageOptions(
      swapChainExtent.width,
      swapChainExtent.height,
      depthFormat,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      device,
      physicalDevice
    );
    VT::CreateImage(imageOptions, depthImage, depthImageMemory);

    VT::ImageViewOptions options{};
    options.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    options.image = depthImage;
    options.format = depthFormat;
    options.device = device;
    depthImageView = VT::CreateImageView(options);
    VT::TransitionImageLayout(device,
                              commandPool,
                              graphicsQueue,
                              depthImage,
                              depthFormat,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  }

 private:

  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;
};
}