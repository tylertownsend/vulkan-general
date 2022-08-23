#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <memory.h>

#include "buffer.h"
#include "command_buffer.h"

namespace VT {
struct CreateIndexBufferOptions {
  VkDevice device;
  VkPhysicalDevice physical_device;
  VkCommandPool command_pool;
  VkQueue graphics_queue;
};

void CreateIndexBuffer(CreateIndexBufferOptions& options, std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory) {
  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  VT::CreateBuffer(bufferSize,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  stagingBuffer,
                  stagingBufferMemory, options.device, options.physical_device);

  void* data;
  vkMapMemory(options.device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), (size_t) bufferSize);
  vkUnmapMemory(options.device, stagingBufferMemory);

  VT::CreateBuffer(bufferSize,
                      // use index bit instead of vertex bit
                      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                      indexBuffer,
                      indexBufferMemory, options.device, options.physical_device);
  VT::CopyBufferOptions copy_buffer_options {options.device, options.command_pool, options.graphics_queue};
  VT::CopyBuffer(copy_buffer_options, stagingBuffer, indexBuffer, bufferSize);

  vkDestroyBuffer(options.device, stagingBuffer, nullptr);
  vkFreeMemory(options.device, stagingBufferMemory, nullptr);
}
} // VT