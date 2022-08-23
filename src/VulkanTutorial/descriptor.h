#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <array>
#include <vector>

#include "descriptor_set_layout.h"
#include "texture_image.h"
#include "uniform_buffer_object.h"
#include "vulkan.h"

namespace VT {

struct CreateDescriptorPoolOptions {
  VkDevice device;
  int max_frames_in_flight;
};

void CreateDescriptorPools(CreateDescriptorPoolOptions& options, VkDescriptorPool& descriptor_pool) {
  std::array<VkDescriptorPoolSize, 2> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(options.max_frames_in_flight);

  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(options.max_frames_in_flight);

  VkDescriptorPoolCreateInfo poolInfo{};
  // allocate a descriptor for every frame
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  // We need to specify the maximum bumber of descriptor sets
  // that may be allocated.
  poolInfo.maxSets = static_cast<uint32_t>(options.max_frames_in_flight);

  if (vkCreateDescriptorPool(options.device, &poolInfo, nullptr, &descriptor_pool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

struct CreateDescriptorSetOptions {
  VkDevice device;
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorPool descriptor_pool;
  VkImageView texture_image_view;
  VkSampler texture_sampler;
  std::vector<VkBuffer> uniform_buffers;
  int max_frames_in_flight;
};

void CreateDescriptorSets(CreateDescriptorSetOptions& options, std::vector<VkDescriptorSet>& descriptor_sets) {
  std::vector<VkDescriptorSetLayout> layouts(options.max_frames_in_flight, options.descriptor_set_layout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  // descriptor pool to allocate from
  allocInfo.descriptorPool = options.descriptor_pool;
  // number of descriptor set to allocate
  allocInfo.descriptorSetCount = static_cast<uint32_t>(options.max_frames_in_flight);
  // descriptor layout to base them on.
  allocInfo.pSetLayouts = layouts.data();

  descriptor_sets.resize(options.max_frames_in_flight);
  if (vkAllocateDescriptorSets(options.device, &allocInfo, descriptor_sets.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate descriptor sets!");
  }

  for (size_t i = 0; i < options.max_frames_in_flight; i++) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = options.uniform_buffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(VT::UniformBufferObject);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = options.texture_image_view;
    imageInfo.sampler = options.texture_sampler;

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptor_sets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptor_sets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(options.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
}

class DescriptorSets {
  std::vector<VkBuffer> _uniform_buffers;
  std::vector<VkDeviceMemory> _uniform_buffers_memory;

  VkDescriptorPool _descriptor_pool;
  std::vector<VkDescriptorSet> _descriptor_sets;

  const std::shared_ptr<VT::Vulkan> _instance;
  int _max_frames_in_flight;

public:
  DescriptorSets(
      const std::shared_ptr<VT::Vulkan>& instance,
      const std::unique_ptr<VT::DescriptorSetLayout>& descriptor_set_layout,
      const std::unique_ptr<VT::TextureView>& texture_image,
      int max_frames_in_flight): _instance(instance), _max_frames_in_flight(max_frames_in_flight) {
    create_uniform_buffers();
    create_descriptor_pool();
    create_descriptor_sets(descriptor_set_layout, texture_image);
  }

  ~DescriptorSets() {
    auto device = this->_instance->GetVkDevice();
    for (size_t i = 0; i < _max_frames_in_flight; i++) {
      vkDestroyBuffer(device, _uniform_buffers[i], nullptr);
      vkFreeMemory(device, _uniform_buffers_memory[i], nullptr);
    }

    vkDestroyDescriptorPool(device, _descriptor_pool, nullptr);
  }

  std::vector<VkBuffer>& GetUniformBuffers() {
    return _uniform_buffers;
  }

  std::vector<VkDeviceMemory>& GetUniformBufferMemory() {
    return _uniform_buffers_memory;
  }

  VkDescriptorPool& GetDescriptorPool() {
    return _descriptor_pool;
  }

  std::vector<VkDescriptorSet>& GetDescriptorSets() {
    return _descriptor_sets;
  }

private:
  void create_uniform_buffers() {
    VT::CreateUniformBufferOptions options {_max_frames_in_flight, _instance->GetVkDevice(), _instance.get()->GetVkPhysicalDevice()};
    VT::CreateUniformBuffers(options, _uniform_buffers, _uniform_buffers_memory);
  }

  void create_descriptor_pool() {
    VT::CreateDescriptorPoolOptions options { this->_instance.get()->GetVkDevice(), _max_frames_in_flight };
    VT::CreateDescriptorPools(options, _descriptor_pool);
  }


  // We create one descriptor set for each chain image
  // with all the same layout.
  // we do need copies of all the layouts because the next function
  // expects an array matching the number of sets
  void create_descriptor_sets(
    const std::unique_ptr<VT::DescriptorSetLayout>& _descriptor_set_layout,
    const std::unique_ptr<VT::TextureView>& texture_image) {
    VT::CreateDescriptorSetOptions options {
      _instance->GetVkDevice(),
      _descriptor_set_layout->GetLayout(),
      _descriptor_pool,
      texture_image->GetImageView(),
      texture_image->GetTextureSampler(),
      _uniform_buffers,
      _max_frames_in_flight
    };
    VT::CreateDescriptorSets(options, _descriptor_sets);
  } 
};
} // VT