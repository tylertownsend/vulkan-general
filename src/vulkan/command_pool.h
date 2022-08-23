#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

#include "vulkan.h"
#include "renderpass.h"
#include "image.h"

namespace VT {
class CommandPool {
 private:
  // manage the memory that is used to store buffers.
  // command buffers allocated from them.
  VkCommandPool _command_pool;
  // command buffer is freed when their command pool is destroyed.
  // so we don't need to explicitly clean up.
  std::vector<VkCommandBuffer> _command_buffers;
  const std::shared_ptr<Vulkan> _instance;
  const int _max_frames_in_flight;

 public:
  CommandPool(const std::shared_ptr<Vulkan>& instance, int max_frames_in_flight):
  _instance(instance),
  _max_frames_in_flight(max_frames_in_flight) {
    init_pool(instance);
    init_command_buffers(instance, max_frames_in_flight);
  }

  // since we have a shared ptr to the instance of vulkan, this destructor
  // will be called before vulkan is destroyed
  ~CommandPool() {
    vkDestroyCommandPool(_instance->GetVkDevice(), _command_pool, nullptr);
  }

  void ResetCommandBuffers() {
    init_command_buffers(_instance, _max_frames_in_flight);
  }

  const VkCommandPool& GetCommandPool() {
    return _command_pool;
  }

  VkCommandBuffer GetCommandBuffer(uint32_t current_frame) {
    return _command_buffers[current_frame];
  }

 private:
  void init_pool(const std::shared_ptr<Vulkan>& instance) {
    VT::QueueFamilyIndices queueFamilyIndices = instance->GetQueueFamilyIndices();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // We will be recording a command buffer every frame, so we want to be able to
    // reset and rerecord over it. Thus, we need to set the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag bit for our command pool.
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    
    // Command buffers are executed by submitting them on one of the device queues,
    // like the graphics and presentation queues we retrieved
    if (vkCreateCommandPool(instance->GetVkDevice(), &poolInfo, nullptr, &_command_pool) != VK_SUCCESS) {
      throw std::runtime_error("failed to create command pool!");
    }
  }

  void init_command_buffers(const std::shared_ptr<Vulkan>& instance, int max_frames_in_flight){
    VT::CreateCommandBuffersOptions options { instance->GetVkDevice(), _command_pool, max_frames_in_flight};
    VT::CreateCommandBuffers(options, _command_buffers);
  }
};
}//