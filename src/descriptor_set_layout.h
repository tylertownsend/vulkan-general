#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <array>

namespace VT {

struct DescriptorSetLayoutOptions {
  VkFormat format;
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

VkDescriptorSetLayout CreateDescriptorSetLayout(DescriptorSetLayoutOptions& options) {
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  // relavant for sampling related descriptors.
  uboLayoutBinding.pImmutableSamplers = nullptr;
  // shader stage the descriptor is referenced.
  // the stage flags can be a combinatino of VkShaderStageFlagBits
  // or the value VKSHADER_STAGE_ALL_GRAPHICS
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  // where the color of the fragment i sgoing to be determined.
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(options.device, &layoutInfo, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
  return descriptor_set_layout;
}
} // VT