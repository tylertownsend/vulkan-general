#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include "descriptor_set_layout.h"
#include "swapchain.h"
#include "vertex.h"
#include "vulkan.h"

namespace VT {

struct GraphicsPipelineOptions {
  VkDevice device;
  VkRenderPass render_pass;
  VkDescriptorSetLayout descriptor_set_layout;
  VkExtent2D swapchain_extent;
};

struct GraphicsPipelineInfo {
  VkPipelineLayout pipeline_layout;
  VkPipeline graphics_pipeline;
};

std::vector<char> read_file(const std::string& filename);
VkShaderModule create_shader_module(const std::vector<char>& code, GraphicsPipelineOptions& options);
GraphicsPipelineInfo create_pipleline_layout(VkPipelineShaderStageCreateInfo shaderStages[], GraphicsPipelineOptions& options);
GraphicsPipelineInfo CreateGraphicsPipeline(GraphicsPipelineOptions& options);

GraphicsPipelineInfo CreateGraphicsPipeline(GraphicsPipelineOptions& options) {

  auto vertShaderCode = read_file("/build/shaders/shader.vert.spv");
  auto fragShaderCode = read_file("/build/shaders/shader.frag.spv");

  VkShaderModule vertShaderModule = create_shader_module(vertShaderCode, options);
  VkShaderModule fragShaderModule = create_shader_module(fragShaderCode, options);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  // The first step, besides the obligatory sType member, is telling Vulkan in which
  // pipeline stage the shader is going to be used.
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  // Every subpass references one or more of the attachments that we've described
  // using the structure in the previous sections. These references are themselves
  // VkAttachmentReference structs that look like this
  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  auto result = create_pipleline_layout(shaderStages, options);

  vkDestroyShaderModule(options.device, fragShaderModule, nullptr);
  vkDestroyShaderModule(options.device, vertShaderModule, nullptr);
  return result;
}

GraphicsPipelineInfo create_pipleline_layout(VkPipelineShaderStageCreateInfo shaderStages[], GraphicsPipelineOptions& options) {
  VkPipelineLayout pipeline_layout;
  VkPipeline graphics_pipeline;

  auto bindingDescription = VT::Vertex::getBindingDescription();
  auto attributeDescriptions = VT::Vertex::getAttributeDescriptions();

  // The VkPipelineVertexInputStateCreateInfo structure describes the format of
  // the vertex data that will be passed to the vertex shader. 
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  // Because we're hard coding the vertex data directly in the vertex shader, we'll
  // fill in this structure to specify that there is no vertex data to load for now
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  // The VkPipelineInputAssemblyStateCreateInfo struct describes two things: what kind of geometry
  // will be drawn from the vertices and if primitive restart should be enabled.
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // A viewport basically describes the region of the framebuffer that the output will be rendered to. 
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float) options.swapchain_extent.width;
  viewport.height = (float) options.swapchain_extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // In this tutorial we simply want to draw to the entire framebuffer,
  // so we'll specify a scissor rectangle that covers it entirely:
  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = options.swapchain_extent;

  // Now this viewport and scissor rectangle need to be combined into a viewport state
  // using the VkPipelineViewportStateCreateInfo struct. It is possible to use multiple
  // viewports and scissor rectangles on some graphics cards, so its members reference
  // an array of them.
  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;


  // The rasterizer takes the geometry that is shaped by the vertices from the vertex shader and turns
  // it into fragments to be colored by the fragment shader.
  // It also performs depth testing, face culling and the scissor test, and it can be
  // configured to output fragments that fill entire polygons or just the edges
  // (wireframe rendering).
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  // It works by combining the fragment shader results of multiple polygons that rasterize
  // to the same pixel. This mainly occurs along edges, which is also where the most
  // noticeable aliasing artifacts occur.
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  // not used atm
  // VkPipelineDepthStencilStateCreateInfo. 

  // VkPipelineColorBlendAttachmentState contains the configuration per attached framebuffer
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  // VkPipelineColorBlendStateCreateInfo contains the global color blending settings.
  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  // pipelineLayoutInfo.pushConstantRangeCount = 0;
  // Need to specify the descriptor set layout during pipeline creation
  // to tell Vlkan which descriptors the shaders will be using.
  pipelineLayoutInfo.pSetLayouts = &options.descriptor_set_layout;

  if (vkCreatePipelineLayout(options.device, &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  // specifiies if the depth of new fragments should be compared to depth buffer
  // to see if they should be discareded.
  depthStencil.depthTestEnable = VK_TRUE;
  // test to see if fragment of the pass the depth test to be written to buffer.
  depthStencil.depthWriteEnable = VK_TRUE;
  // lower depth = closer so depth of new fragments should be less
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  // determine if we should keep the fragments within a certain range
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds = 0.0f; // Optional
  depthStencil.maxDepthBounds = 1.0f; // Optional
  // Configure Stencil Buffer Operations.
  depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.front = {}; // Optional
  depthStencil.back = {}; // Optional

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = pipeline_layout;
  pipelineInfo.renderPass = options.render_pass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.pDepthStencilState = &depthStencil;

  if (vkCreateGraphicsPipelines(options.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphics_pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  return GraphicsPipelineInfo { pipeline_layout, graphics_pipeline };
}

VkShaderModule create_shader_module(const std::vector<char>& code, GraphicsPipelineOptions& options) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  // When you perform a cast like this, you also need to ensure that the data satisfies the
  // alignment requirements of uint32_t. Lucky for us, the data is stored in an std::vector
  // where the default allocator already ensures that the data satisfies the worst case alignment
  // requirements.
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(options.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }

  return shaderModule;
}

std::vector<char> read_file(const std::string& filename) {
  // read at the end of the file is that we can use the read position to determine
  // the size of the file and allocate a buffer
  // Read the file as binary file (avoid text transformations)
  char buff[FILENAME_MAX]; //create string buffer to hold path
  char* cwd = GetCurrentDir( buff, FILENAME_MAX );
  std::string current_working_directory = std::string(buff);
  std::string full_path_to_file = current_working_directory.append(filename);
  std::cout<<full_path_to_file<<std::endl;
  std::ifstream file(full_path_to_file, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
      throw std::runtime_error("failed to open file!");
  }

  size_t fileSize = (size_t) file.tellg();
  std::vector<char> buffer(fileSize);

  // After that, we can seek back
  // to the beginning of the file and read all of the bytes at once:
  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

class GraphicsPipeline {
  VkRenderPass _render_pass;
  VkPipelineLayout _graphics_pipeline_layout;
  VkPipeline _graphics_pipeline;

  const std::shared_ptr<VT::Vulkan> _instance;

public:
  GraphicsPipeline(
      const std::shared_ptr<VT::Vulkan>& instance,
      const std::unique_ptr<VT::Swapchain>& swapchain,
      const std::unique_ptr<VT::DescriptorSetLayout>& descriptor_set_layout):_instance(instance) {
    create_render_pass(swapchain);
    create_graphics_pipeline(swapchain, descriptor_set_layout);
  }

  ~GraphicsPipeline() {
    auto device = _instance->GetVkDevice();
    vkDestroyPipeline(device, _graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(device, _graphics_pipeline_layout, nullptr);
    vkDestroyRenderPass(device, _render_pass, nullptr);
  }

  VkRenderPass& GetRenderPass() {
    return _render_pass;
  }

  VkPipelineLayout& GetPipelineLayout() {
    return _graphics_pipeline_layout;
  }

  VkPipeline& GetPipeline() {
    return _graphics_pipeline;
  }

private:
  void create_render_pass(
      const std::unique_ptr<VT::Swapchain>& swapchain) {
    VT::RenderPassOptions options{
      swapchain->GetImageFormat(),
      _instance->GetVkDevice(),
      _instance->GetVkPhysicalDevice()
    };
    _render_pass = VT::CreateRenderPass(options);
  }

  void create_graphics_pipeline(
      const std::unique_ptr<VT::Swapchain>& swapchain,
      const std::unique_ptr<VT::DescriptorSetLayout>& descriptor_set_layout) {
    VT::GraphicsPipelineOptions options {
      _instance->GetVkDevice(),
      _render_pass,
      descriptor_set_layout->GetLayout(),
      swapchain->GetExtent()
    };
    auto result = VT::CreateGraphicsPipeline(options);
    _graphics_pipeline = result.graphics_pipeline;
    _graphics_pipeline_layout = result.pipeline_layout;
  }
};
} // VT