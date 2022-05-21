#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "device.h"


namespace VT {

  void CreateBuffer(
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkBuffer& buffer,
      VkDeviceMemory& bufferMemory,
      VkDevice* device,
      VkPhysicalDevice* physicalDevice) {

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(*device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to create buffer!");
    }

    // Buffer is created but doesn't have mem assigned to it so we need to figure
    // out the type of memory to allocate.
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

    if (vkAllocateMemory(*device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate buffer memory!");
    }

    // if memory allocation was successful we can associate this memory
    // with the buffer
    vkBindBufferMemory(*device, buffer, bufferMemory, 0);
  }
} // VT