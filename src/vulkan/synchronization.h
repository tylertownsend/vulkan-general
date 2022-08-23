#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>

namespace VT {

struct CreateSyncObjectsOptions {
  VkDevice device;
  int max_frames_in_flight;
};


void CreateSyncObjects(
    CreateSyncObjectsOptions& options,
    std::vector<VkSemaphore>& image_available_semaphores,
    std::vector<VkSemaphore>& render_finished_semaphores,
    std::vector<VkFence>& in_flight_fences) {
  image_available_semaphores.resize(options.max_frames_in_flight);
  render_finished_semaphores.resize(options.max_frames_in_flight);
  in_flight_fences.resize(options.max_frames_in_flight);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  for (size_t i = 0; i < options.max_frames_in_flight; i++) {
    if (vkCreateSemaphore(options.device, &semaphoreInfo, nullptr, &image_available_semaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(options.device, &semaphoreInfo, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS ||
        vkCreateFence(options.device, &fenceInfo, nullptr, &in_flight_fences[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create semaphores!");
    }
  }
}
}