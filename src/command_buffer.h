#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "device.h"


namespace VT {

VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool command_pool) {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = command_pool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

  // We are using the command buffer only once and wait with returning
  // to the function until copy op has finished.
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkDevice device, VkCommandPool command_pool, VkQueue graphics_queue) {
  // The command buffer only contains the copy command so we can stop
  // recording after that.
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  // TODO: Create own dedicated queue for copy transfers
  vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
  // if multiple transfer can be done concurrently, a fence would allow that
  // instead of executing 1 at a time. Here we only have 1 so we wait
  // idle until it completes
  vkQueueWaitIdle(graphics_queue);

  vkFreeCommandBuffers(device, command_pool, 1, &commandBuffer);
}
} // VT