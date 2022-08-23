#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include "vulkan.h"
#include "command_pool.h"
#include "buffer.h"
#include "image.h"
#include "constants.h"

namespace VT {

struct CreateTextureImageOptions {
  VkDevice device;
  VkPhysicalDevice physical_device;
  VkCommandPool command_pool;
  VkQueue graphics_queue;
};

struct TextureImage {
  VkImage texture_image;
  VkDeviceMemory texture_image_memory;
};

void CreateTextureImage(CreateTextureImageOptions& options, VkImage& texture_image, VkDeviceMemory& texture_image_memory) {
  int texWidth, texHeight, texChannels;
  char buff[FILENAME_MAX]; //create string buffer to hold path
  char* cwd = GetCurrentDir( buff, FILENAME_MAX );
  std::string current_working_directory = std::string(buff);
  const char* full_path_to_file = current_working_directory.append("/build/" + TEXTURE_PATH).c_str();
  std::cout<<full_path_to_file<<std::endl;
  stbi_uc* pixels = stbi_load(full_path_to_file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  // pixels are laid out row by row with 4 bytes per pixel in th case of
  // STBI_rgb_alpha for a total of texWidth*texHeight *4
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }

  // Create a buffer in host visible memory so that we can use
  // vkMapMemory and copy pixels to it.
  // Buffer should be in host visible memory so we can map it and
  // should be usable as a transfer source so we can copy it
  // to an image later on.
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  VT::CreateBuffer(imageSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer,
                    stagingBufferMemory,
                    options.device,
                    options.physical_device);
  void *data;
  vkMapMemory(options.device, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(options.device, stagingBufferMemory);

  stbi_image_free(pixels);

  VT::CreateImageOptions image_options(
    texWidth,
    texHeight,
    VK_FORMAT_R8G8B8A8_SRGB,
    VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    options.device,
    options.physical_device);
  VT::CreateImage(image_options, texture_image, texture_image_memory);

  VT::TransitionImageLayout(options.device, options.command_pool, options.graphics_queue, texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  VT::CopyBufferToImage(options.device, options.command_pool, options.graphics_queue, stagingBuffer, texture_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
  VT::TransitionImageLayout(options.device, options.command_pool, options.graphics_queue, texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(options.device, stagingBuffer, nullptr);
  vkFreeMemory(options.device, stagingBufferMemory, nullptr);
}
void CreateTextureImageView() {
}

struct CreateTextureSamplerOptions {
  VkDevice device;
  VkPhysicalDevice physical_device;

};

VkSampler CreateTextureImageSampler(CreateTextureSamplerOptions& options) {
  VkSampler texture_sampler;

  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(options.physical_device, &properties);

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  // how to interpret pixels that are magnified or minified.
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  // we want to repeat texture when oing beyond image dimensions.
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  // limits the amount of samples that can be used to calculate the final
  // color a lower value result in beteter performance and lower quality
  // results. We can retrieve properties of physical device
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  // specifies which coordinate system you want to use to address
  // texels in an image.
  // real-world apps use normalized coordinates because its possible
  // to usess texture of varying resolutions on same coordinates.
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  if (vkCreateSampler(options.device, &samplerInfo, nullptr, &texture_sampler) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture sampler!");
  }
  return texture_sampler;
}

class TextureView {
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;

  const std::shared_ptr<Vulkan> _instance;
public:
  TextureView(
      const std::shared_ptr<Vulkan>& instance, 
      const std::unique_ptr<CommandPool>&  command_pool): _instance(instance) {
    create_texture_image(command_pool);
    create_texture_image_view();
    create_texture_sampler();
  }

  ~TextureView() {
    auto device = _instance->GetVkDevice();

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);

    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);
  }

  VkImageView& GetImageView() {
    return textureImageView;
  }

  VkSampler& GetTextureSampler() {
    return textureSampler;
  }

private:
  void create_texture_image(const std::unique_ptr<CommandPool>& command_pool) {
    VT::CreateTextureImageOptions options{
      _instance->GetVkDevice(),
      _instance->GetVkPhysicalDevice(),
      command_pool->GetCommandPool(),
      _instance->GetGraphicsQueue()
    };
    VT::CreateTextureImage(options, textureImage, textureImageMemory);
  }

  void create_texture_image_view() {
    VT::ImageViewOptions options{};
    options.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    options.format = VK_FORMAT_R8G8B8A8_SRGB;
    options.device = _instance->GetVkDevice();
    options.image = textureImage;
    textureImageView = VT::CreateImageView(options);
  }

  void create_texture_sampler() {
    VT::CreateTextureSamplerOptions options { _instance->GetVkDevice(), _instance->GetVkPhysicalDevice() };
    textureSampler = VT::CreateTextureImageSampler(options);
  }

};
} // VT