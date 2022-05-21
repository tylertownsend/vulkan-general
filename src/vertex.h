#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

# define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "command_buffer.h"

namespace VT {
struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;
  // how image is mapped to geometry
  glm::vec2 texCoord;

  bool operator==(const Vertex& other) const {
    return pos == other.pos && 
           color == other.color &&
           texCoord == other.texCoord;
  }

  /**
   * @brief Get the Binding Description object
   * @details Describes at which rate to load data from memory throughout
   * the verticies. It specifies the number of bytes between data entries
   * and wheter tho move to the next data entry after each vertex instance.
   * 
   * @return VkVertexInputBindingDescription 
   */
  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
  }

  /**
   * @brief Get the Attribute Descriptions object
   * @details Describes how to extract a vertex attribute from a chunk of vertex
   * data originating from the bind description.
   * 
   * @return std::array<VkVertexInputAttributeDescription, 2> 
   */
  static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

    // ---- position attribute ----
    // which binding the per-vertex data comes from
    attributeDescriptions[0].binding = 0;
    // the location directive of the input from the vertex shader.
    attributeDescriptions[0].location = 0;
    // describes the type of data fro the attribute.
    // you should use the amout of color channels matches the number of components
    // in the shader data type.
    // explicitly defines the byte size of attribute data
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    // specifies the the number of bytes since teh start of the per-vertex
    // data to read from.
    // the binding is loading one vertex at a time and the position attribute
    // is an offset of 0 bytes from the beggining of this struct.
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    // --- Color Attribute ---
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
  }
};

struct CreateVertexBufferOptions {
  VkDevice device;
  VkPhysicalDevice physical_device;
  VkCommandPool command_pool;
  VkQueue graphics_queue;
  std::vector<Vertex>& vertices;
};

void CreateVertexBuffer(CreateVertexBufferOptions& options, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory) {
  VkDeviceSize bufferSize = sizeof(options.vertices[0]) * options.vertices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  VT::CreateBuffer(bufferSize,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  stagingBuffer,
                  stagingBufferMemory,
                  options.device,
                  options.physical_device);
  // copy data to vertex buffer.
  // mapping buffer memory into the cpu accessible memory with vkMapMemory
  void* data;
  vkMapMemory(options.device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, options.vertices.data(), (size_t) bufferSize);
  vkUnmapMemory(options.device, stagingBufferMemory);

  // The most optimal memory has the VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT flag
  // and is usually not accessible by the CPU on dedicated graphics cards.
  // So we created the staging buffer in CPU accesible memory to upload
  // the data from the vertex array to, and the final vertex buffer
  // in device local memory.
  VT::CreateBuffer(bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    vertexBuffer,
                    vertexBufferMemory,
                    options.device,
                    options.physical_device);
  VT::CopyBufferOptions copy_buffer_options {options.device, options.command_pool, options.graphics_queue};
  VT::CopyBuffer(copy_buffer_options, stagingBuffer, vertexBuffer, bufferSize);
  vkDestroyBuffer(options.device, stagingBuffer, nullptr);
  vkFreeMemory(options.device, stagingBufferMemory, nullptr);
}
}

// Hash functions are a complex topic, but cppreference.com recommends
// the following approach combining the fields of a struct to create
// a decent quality hash function
namespace std {
  template<> struct hash<VT::Vertex> {
    size_t operator()(VT::Vertex const& vertex) const {
      return ((hash<glm::vec3>()(vertex.pos) ^
              (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
              (hash<glm::vec2>()(vertex.texCoord) << 1);
    }
  };
}