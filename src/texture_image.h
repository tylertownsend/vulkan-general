#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>
#include <stdexcept>
#include <string>

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

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
void CreateTextureImageSampler() {
}
} // VT