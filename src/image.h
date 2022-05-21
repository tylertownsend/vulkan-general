#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

#include "device.h"

namespace VT {

struct CreateImageOptions {
  const uint32_t width;
  const uint32_t height;
  const VkFormat format;
  const VkImageTiling tiling;
  const VkImageUsageFlags usage;
  const VkMemoryPropertyFlags properties;
  const VkDevice device;
  const VkPhysicalDevice physical_device;

  CreateImageOptions(
      const uint32_t width,
      const uint32_t height,
      const VkFormat format,
      const VkImageTiling tiling,
      const VkImageUsageFlags usage,
      const VkMemoryPropertyFlags properties,
      const VkDevice device,
      const VkPhysicalDevice physical_device): 
        width(width),
        height(height),
        format(format),
        tiling(tiling),
        usage(usage),
        properties(properties),
        device(device),
        physical_device(physical_device){}
};

// TODO: pass by reference works but not having them doesn't look into it.
void CreateImage(const CreateImageOptions& options, VkImage& image, VkDeviceMemory& imageMemory) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = options.width;
  imageInfo.extent.height = options.height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = options.format;
  imageInfo.tiling = options.tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = options.usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(options.device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(options.device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = VT::FindMemoryType(memRequirements.memoryTypeBits, options.properties, options.physical_device);

  if (vkAllocateMemory(options.device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }
  vkBindImageMemory(options.device, image, imageMemory, 0);
}

struct ImageViewOptions {
  VkImage image;
  VkFormat format;
  VkImageAspectFlagBits aspectFlags;
  VkDevice device;
  // ImageViewOptions(const VkImage* image, 
  //                  const VkFormat format,
  //                  const VkImageAspectFlagBits aspectFlags,
  //                  const VkDevice* device):
  //   image(image),
  //   format(format),
  //   aspectFlags(aspectFlags),
  //   device(device) {}
};

VkImageView CreateImageView(ImageViewOptions& options) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = options.image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = options.format;
  viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.subresourceRange.aspectMask = options.aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;
  if (vkCreateImageView(options.device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }

  return imageView;
}
} // VT