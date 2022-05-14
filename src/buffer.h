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
  // void CreateVertexBuffer() {
  //   VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  //   VkBuffer stagingBuffer;
  //   VkDeviceMemory stagingBufferMemory;
  //   this->create_buffer(bufferSize,
  //                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  //                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //                       stagingBuffer,
  //                       stagingBufferMemory);
  //   // copy data to vertex buffer.
  //   // mapping buffer memory into the cpu accessible memory with vkMapMemory
  //   void* data;
  //   vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  //   memcpy(data, vertices.data(), (size_t) bufferSize);
  //   vkUnmapMemory(device, stagingBufferMemory);

  //   // The most optimal memory has the VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT flag
  //   // and is usually not accessible by the CPU on dedicated graphics cards.
  //   // So we created the staging buffer in CPU accesible memory to upload
  //   // the data from the vertex array to, and the final vertex buffer
  //   // in device local memory.
  //   this->create_buffer(bufferSize,
  //                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
  //                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
  //                       vertexBuffer,
  //                       vertexBufferMemory);
  //   this->copy_buffer(stagingBuffer, vertexBuffer, bufferSize);
  //   vkDestroyBuffer(device, stagingBuffer, nullptr);
  //   vkFreeMemory(device, stagingBufferMemory, nullptr);
  // }

  // // Notes: The right way to allocate memory for a large number of objects at
  // // the same time is to create a custom allocator that splits up a single
  // // allocation among many different objects by using the offset parameters
  // // that we've seen in many functions.
  // void copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

  //   VkCommandBuffer commandBuffer = this->begin_single_time_commands();

  //   VkBufferCopy copyRegion{};
  //   copyRegion.srcOffset = 0; // Optional
  //   copyRegion.dstOffset = 0; // Optional
  //   copyRegion.size = size;
  //   vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  //   this->end_single_time_commands(commandBuffer);
  // }

  // uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  //   VkPhysicalDeviceMemoryProperties memProperties;
  //   vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  //   for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
  //     if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
  //       return i;
  //     }
  //   }

  //   throw std::runtime_error("failed to find suitable memory type!");
  // }

  // void create_index_buffer() {
  //   VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  //   VkBuffer stagingBuffer;
  //   VkDeviceMemory stagingBufferMemory;
  //   this->create_buffer(bufferSize,
  //                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  //                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //                       stagingBuffer,
  //                       stagingBufferMemory);

  //   void* data;
  //   vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  //   memcpy(data, indices.data(), (size_t) bufferSize);
  //   vkUnmapMemory(device, stagingBufferMemory);

  //   this->create_buffer(bufferSize,
  //                       // use index bit instead of vertex bit
  //                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
  //                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
  //                       indexBuffer,
  //                       indexBufferMemory);

  //   this->copy_buffer(stagingBuffer, indexBuffer, bufferSize);

  //   vkDestroyBuffer(device, stagingBuffer, nullptr);
  //   vkFreeMemory(device, stagingBufferMemory, nullptr);
  // }

  // void create_uniform_buffers() {
  //   VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  //   uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  //   uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

  //   for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
  //     this->create_buffer(bufferSize,
  //                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  //                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //                         uniformBuffers[i],
  //                         uniformBuffersMemory[i]);
  //   }
  // }
} //