#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

# define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "command_buffer.h"

namespace VT {
struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

struct CreateUniformBufferOptions {
  int max_frames_in_flight;
  VkDevice device;
  VkPhysicalDevice physical_device;
  std::vector<VkBuffer> uniform_buffers;
  std::vector<VkDeviceMemory> uniform_buffers_memory;
};

struct UnformBufferInfo {
  std::vector<VkBuffer> uniform_buffers;
  std::vector<VkDeviceMemory> uniform_buffers_memory;
};

void CreateUniformBuffers(CreateUniformBufferOptions& options) {
  VkDeviceSize bufferSize = sizeof(VT::UniformBufferObject);

  options.uniform_buffers.resize(options.max_frames_in_flight);
  options.uniform_buffers_memory.resize(options.max_frames_in_flight);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VT::CreateBuffer(bufferSize,
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      options.uniform_buffers[i],
                      options.uniform_buffers_memory[i],
                      options.device,
                      options.physical_device);
  }
}
} // VT