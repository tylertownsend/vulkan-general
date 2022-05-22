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

#include "buffer.h"
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
  
};

struct UnformBufferInfo {
  std::vector<VkBuffer> uniform_buffers;
  std::vector<VkDeviceMemory> uniform_buffers_memory;
};

void CreateUniformBuffers(
    CreateUniformBufferOptions& options, 
    std::vector<VkBuffer>& uniform_buffers,
    std::vector<VkDeviceMemory>& uniform_buffers_memory) {
  VkDeviceSize bufferSize = sizeof(VT::UniformBufferObject);

  uniform_buffers.resize(options.max_frames_in_flight);
  uniform_buffers_memory.resize(options.max_frames_in_flight);

  for (size_t i = 0; i < options.max_frames_in_flight; i++) {
    VT::CreateBuffer(bufferSize,
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      uniform_buffers[i],
                      uniform_buffers_memory[i],
                      options.device,
                      options.physical_device);
  }
}


// generate a new transformation every frame to make the geometry
// spin around
void UpdateUniformBuffer(VkDevice device, std::vector<VkDeviceMemory>& uniform_buffers_memory, VkExtent2D swap_chain_extent, uint32_t currentImage) {
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

  VT::UniformBufferObject ubo{};
  // simple rotatio around z axis using time variable
  auto rotation_angle =  time * glm::radians(90.0f);
  ubo.model = glm::rotate(glm::mat4(1.0f), 
                          rotation_angle,
                          glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                          glm::vec3(0.0f, 0.0f, 0.0f),
                          glm::vec3(0.0f, 0.0f, 1.0f));
  // prospective project with a 45 degree vertical field of view.
  // its important that he current swap chain textent to calculate the aspect
  // ratio to take into account the new width and height of the window after
  // resize
  ubo.proj = glm::perspective(glm::radians(45.0f),
                              swap_chain_extent.width / (float) swap_chain_extent.height,
                              1.0f,
                              10.0f);
  // GLM was originally designed for OpenGl where the Y coordinate of
  // of the clip coordinates is inverted. The easiest way to compensate
  // is to flip the sign of the scaling factor of the Y axis in the projection
  // matrix
  ubo.proj[1][1] *= -1;

  void* data;
  vkMapMemory(device, uniform_buffers_memory[currentImage], 0, sizeof(ubo), 0, &data);
  memcpy(data, &ubo, sizeof(ubo));
  vkUnmapMemory(device, uniform_buffers_memory[currentImage]);
}
} // VT