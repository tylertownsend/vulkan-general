#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

# define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


#include <chrono>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>
#include <algorithm>
#include <unordered_map>

#include "vulkan.h"
#include "buffer.h"
#include "device.h"
#include "image.h"
#include "swapchain.h"
#include "vertex.h"
#include "vulkan_debug.h"
#include "queue_families.h"
#include "renderpass.h"
#include "descriptor_set_layout.h"
#include "graphics_pipeline.h"
#include "command_buffer.h"
#include "texture_image.h"
#include "constants.h"
#include "uniform_buffer_object.h"
#include "descriptor.h"
#include "model.h"
#include "window.h"
#include "synchronization.h"
#include "indices.h"

// number of frames to be processed concurrently.
const int MAX_FRAMES_IN_FLIGHT = 2;
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// Removing these predefined vertices and indicies to use loaded models.
// const std::vector<Vertex> vertices = {
//   {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//   {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//   {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//   {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

//   {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//   {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//   {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//   {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
// };

// const std::vector<uint16_t> indices = {
//   0, 1, 2, 2, 3, 0,
//   4, 5, 6, 6, 7, 4
// };

// #ifdef NDEBUG
// const bool enableValidationLayers = true;
// #else
// const bool enableValidationLayers = false;
// #endif

class HelloTriangleApplication {
public:
  HelloTriangleApplication() {}
  void Run() {
    init_window();
    init_vulkan();
    main_loop();
    cleanup();
  }

private:
  std::unique_ptr<VT::Vulkan> _instance;
  std::unique_ptr<phx::Window> _window;

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;

  VkRenderPass renderPass;
  VkDescriptorSetLayout descriptorSetLayout;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  std::vector<VkFramebuffer> swapChainFramebuffers;

  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;

  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

  std::vector<VT::Vertex> vertices;
  std::vector<uint32_t> indices;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;

  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;

  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;

  uint32_t currentFrame = 0;

  void init_vulkan() {
    create_instance();

    create_swapchain();
    create_image_views();
    create_render_pass();
    create_descriptor_set_layout();
    create_graphics_pipeline();
    create_command_pool();
    create_depth_resources();
    // moving frame buffers to make sure it is called after the depth image
    // view has ben created
    create_frame_buffers();

    create_texture_image();
    create_texture_image_view();
    create_texture_sampler();

    load_model();
    create_vertex_buffer();
    create_index_buffer();
    create_uniform_buffers();
    create_descriptor_pool();
    create_descriptor_sets();
    create_command_buffers();

    create_sync_objects();
  }

  void init_window() {
    WindowOptions window_options{};
    window_options.height = 600;
    window_options.width = 800;
    window_options.title = "Townsend Window";
    _window = std::make_unique<phx::Window>(window_options);
  }

  void main_loop() {
    while (!_window->WindowShouldClose()) {
      glfwPollEvents();
      draw_frame();
    }
    this->_instance.get()->DeviceWaitIdle();
  }

  void cleanup() {
    auto instance = this->_instance->GetVkInstance();
    auto device = this->_instance->GetVkDevice();

    this->cleanup_swap_chain();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroyBuffer(device, uniformBuffers[i], nullptr);
      vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);

    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    // should be available for use in rednering commands until the end
    // of the program.
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
      vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);
  }

  void create_instance() {
    const char* application_name = "vulkan_demo";
    const char* engine_name= "townsend engine";
    auto window = this->_window->GetGLFWwindow();
    VT::VulkanOptions options(window, application_name, engine_name);
    _instance = VT::Vulkan::CreateInstance(options);
  }

  void create_image_views() {
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
      VT::ImageViewOptions options{};
      options.device = this->_instance->GetVkDevice();
      options.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
      options.format = swapChainImageFormat;
      options.image = swapChainImages[i];
      swapChainImageViews[i] = VT::CreateImageView(options);
    }
  }
  void create_swapchain() {
    VT::Vulkan* instance = this->_instance.get();
    VT::SwapChainOptions options{
      instance->GetVkInstance(),
      instance->GetVkSurfaceKHR(),
      instance->GetVkPhysicalDevice(),
      instance->GetVkDevice(),
      _window->GetGLFWwindow()
    };
    auto swapchainInfo = VT::CreateSwapchain(options);
    swapChain = swapchainInfo.swapchain;
    swapChainImages = swapchainInfo.swapchain_images;
    swapChainImageFormat = swapchainInfo.swapchain_image_format;
    swapChainExtent = swapchainInfo.swapchain_extent;
  }

  void create_render_pass() {
    VT::RenderPassOptions options{swapChainImageFormat, this->_instance.get()->GetVkDevice(), this->_instance.get()->GetVkPhysicalDevice()};
    renderPass = VT::CreateRenderPass(options);
  }

  void create_descriptor_set_layout() {
    VT::DescriptorSetLayoutOptions options{ swapChainImageFormat, this->_instance.get()->GetVkDevice() };
    descriptorSetLayout = VT::CreateDescriptorSetLayout(options);
  }

  void create_graphics_pipeline() {
    VT::GraphicsPipelineOptions options {this->_instance.get()->GetVkDevice(), renderPass, descriptorSetLayout, swapChainExtent };
    auto result = VT::CreateGraphicsPipeline(options);
    graphicsPipeline = result.graphics_pipeline;
    pipelineLayout = result.pipeline_layout;
  }

  void create_depth_resources() {

    // TODO: find_depth_format should probably be initialied before this and renderpass
    // are called.
    VkFormat depthFormat = VT::find_depth_format(this->_instance.get()->GetVkPhysicalDevice());

    VT::CreateImageOptions imageOptions(
      swapChainExtent.width,
      swapChainExtent.height,
      depthFormat,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      this->_instance.get()->GetVkDevice(),
      this->_instance.get()->GetVkPhysicalDevice()
    );
    VT::CreateImage(imageOptions, depthImage, depthImageMemory);

    VT::ImageViewOptions options{};
    options.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    options.image = depthImage;
    options.format = depthFormat;
    options.device = this->_instance.get()->GetVkDevice();
    depthImageView = VT::CreateImageView(options);
    VT::TransitionImageLayout(this->_instance.get()->GetVkDevice(),
                              commandPool,
                              this->_instance->GetGraphicsQueue(),
                              depthImage,
                              depthFormat,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  }

  void create_command_pool() {
    VT::QueueFamilyIndices queueFamilyIndices = this->_instance->GetQueueFamiliyIndices();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // We will be recording a command buffer every frame, so we want to be able to
    // reset and rerecord over it. Thus, we need to set the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag bit for our command pool.
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    
    // Command buffers are executed by submitting them on one of the device queues,
    // like the graphics and presentation queues we retrieved
    if (vkCreateCommandPool(this->_instance.get()->GetVkDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
      throw std::runtime_error("failed to create command pool!");
    }
  }

  void create_frame_buffers() {
    VT::CreateFrameBuffersOptions options {this->_instance.get()->GetVkDevice(), renderPass, swapChainExtent, swapChainImageViews};
    VT::CreateFrameBuffers(options, depthImageView, swapChainFramebuffers);
  }

  void create_texture_image() {
    VT::CreateTextureImageOptions options{
      this->_instance.get()->GetVkDevice(),
      this->_instance.get()->GetVkPhysicalDevice(),
      commandPool,
      this->_instance->GetGraphicsQueue()
    };
    VT::CreateTextureImage(options, textureImage, textureImageMemory);
  }

  void create_texture_image_view() {
    VT::ImageViewOptions options{};
    options.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    options.format = VK_FORMAT_R8G8B8A8_SRGB;
    options.device = this->_instance.get()->GetVkDevice();
    options.image = textureImage;
    textureImageView = VT::CreateImageView(options);
  }

  void create_texture_sampler() {
    VT::CreateTextureSamplerOptions options { this->_instance.get()->GetVkDevice(), this->_instance.get()->GetVkPhysicalDevice() };
    textureSampler = VT::CreateTextureImageSampler(options);
  }

  void load_model() {
    VT::LoadModel(vertices, indices, VT::MODEL_PATH.c_str());
  }

  void create_vertex_buffer() {
    VT::CreateVertexBufferOptions options{this->_instance.get()->GetVkDevice(), this->_instance.get()->GetVkPhysicalDevice(), commandPool, this->_instance->GetGraphicsQueue(), vertices};
    VT::CreateVertexBuffer(options, vertexBuffer, vertexBufferMemory);
  }

  void create_index_buffer() {
    VT::CreateIndexBufferOptions options{this->_instance.get()->GetVkDevice(), this->_instance.get()->GetVkPhysicalDevice(), commandPool, this->_instance->GetGraphicsQueue() };
    VT::CreateIndexBuffer(options, indices, indexBuffer, indexBufferMemory);
  }

  void create_uniform_buffers() {
    VT::CreateUniformBufferOptions options {MAX_FRAMES_IN_FLIGHT, this->_instance.get()->GetVkDevice(), this->_instance.get()->GetVkPhysicalDevice()};
    VT::CreateUniformBuffers(options, uniformBuffers, uniformBuffersMemory);
  }

  void create_descriptor_pool() {
    VT::CreateDescriptorPoolOptions options { this->_instance.get()->GetVkDevice(), MAX_FRAMES_IN_FLIGHT };
    VT::CreateDescriptorPools(options, descriptorPool);
  }

  // We create one descriptor set for each swap chain image
  // with all the same layout.
  // we do need copies of all the layouts because the next function
  // expects an array matching the number of sets
  void create_descriptor_sets() {
    VT::CreateDescriptorSetOptions options {
      this->_instance.get()->GetVkDevice(),
      descriptorSetLayout,
      descriptorPool,
      textureImageView,
      textureSampler,
      uniformBuffers,
      MAX_FRAMES_IN_FLIGHT
    };
    VT::CreateDescriptorSets(options, descriptorSets);
  }

  void create_command_buffers() {
    VT::CreateCommandBuffersOptions options { this->_instance.get()->GetVkDevice(), commandPool, MAX_FRAMES_IN_FLIGHT};
    VT::CreateCommandBuffers(options, commandBuffers);
  }

  void create_sync_objects() {
    VT::CreateSyncObjectsOptions options { this->_instance.get()->GetVkDevice(), MAX_FRAMES_IN_FLIGHT };
    VT::CreateSyncObjects(options, imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences);
  }

  void draw_frame() {
    uint32_t imageIndex;
    vkWaitForFences(this->_instance.get()->GetVkDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    VkResult result = vkAcquireNextImageKHR(this->_instance.get()->GetVkDevice(), swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      // swap chain has become incompatiable with surface and can no longer be used for rendering
      // (e.g window resize)
      recreate_swap_chain();
      return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      // swap chain can be used to successufly present to surface but surface properties no
      // longer exactly match.
      throw std::runtime_error("failed to acquire swap chain image");
    }

    VT::UpdateUniformBuffer(this->_instance.get()->GetVkDevice(), uniformBuffersMemory, swapChainExtent, currentFrame);

    // delay resetting fence until after we know for sure we will be submitting work with it.
    // in the case of recreating swap chain:
    //   the current frame's fence was waited upon and reset. Since we returned immediately,
    //   no work is submitted for execution and the fence will never be signalled causing to 
    //   wait forever.
    vkResetFences(this->_instance.get()->GetVkDevice(), 1, &inFlightFences[currentFrame]);

    // call on command buffer to make sure it is able to be recorded.
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    // call to record the commands we want.
    record_command_buffer(commandBuffers[currentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    // want to wait until the image is available before writing colors to the image
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    // The signalSemaphoreCount and pSignalSemaphores specifcy which semaphores to signal
    // once the command buffer finishes execution.
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(this->_instance->GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }
    // submitting the result back to the swap chain to have it eventually show up on the screen
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    // The next two parameters specify the swap chains to present
    // images to and the index of the image for each swap chain. This will almost always
    // be a single one.
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    // There is one last optional parameter called pResults. It allows you to specify an array
    // of VkResult values to check for every individual swap chain if presentation was successfu
    presentInfo.pResults = nullptr;

    VkResult queueResult = vkQueuePresentKHR(this->_instance->GetPresentQueue(), &presentInfo);

    if (queueResult == VK_ERROR_OUT_OF_DATE_KHR ||
        queueResult == VK_SUBOPTIMAL_KHR ||
        this->_window->FrameBufferResized()) {
      // make sure to do this after queuepresentKHR to make sure semaphores are in consistent
      // state, otherwise a signalled semaphore may never be properly waited upon.
      this->_window->ResetFrameBuffer();
      recreate_swap_chain();
    } else if (queueResult != VK_SUCCESS) {
      throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // The flags parameter specifies how we're going to use the command buffer. The following values are available:
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
    // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
    // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // The first parameters are the render pass itself and the attachments to bind. We created a framebuffer for
    // each swap chain image where it is specified as a color attachment.
    // Thus we need to bind the framebuffer for the swapchain image we want to draw to. 
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    // The render area defines where shader loads and stores will take place.
    // The pixels outside this region will have undefined values.
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    // The last two parameters define the clear values to use for;
    // VK_ATTACHMENT_LOAD_OP_CLEAR, which we used as load operation for the color attachment
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color= {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    // The first parameter for every command is always the command buffer to record the
    // command to. The second parameter specifies the details of the render pass we've
    // just provided. The final parameter controls how the drawing commands within the render
    // pass will be provided. 
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // bind vertex buffer during rendering operations
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    // Descriptor sets can be used in graphics or compute pipelines so we need to specify
    // which one to use.
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

    // vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
    // instanceCount: Used for instanced rendering, use 1 if you're not doing that.
    // firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
    // firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }
  }

  // However, the disadvantage of this approach is that we need to stop all rendering before
  // creating the new swap chain. It is possible to create a new swap chain while drawing
  // commands on an image from the old swap chain are still in-flight. You need to pass the
  // previous swap chain to the oldSwapChain field in the VkSwapchainCreateInfoKHR struct
  // and destroy the old swap chain as soon as you've finished using it.
  void recreate_swap_chain() {
    this->_window->HandleMinimization();

    // don't touch resources still in use
    vkDeviceWaitIdle(this->_instance.get()->GetVkDevice());

    cleanup_swap_chain();

    // Note that in chooseSwapExtent we already query the new window resolution to make sure
    // that the swap chain images have the (new) right size, so there's no need to modify
    // chooseSwapExtent (remember that we already had to use glfwGetFramebufferSize get the
    // resolution of the surface in pixels when creating the swap chain).
    create_swapchain();
    create_image_views();
    create_render_pass();
    create_graphics_pipeline();
    create_depth_resources();
    create_frame_buffers();
    create_uniform_buffers();
    create_descriptor_pool();
    create_descriptor_sets();
    create_command_buffers();
  }

  void cleanup_swap_chain() {
    auto device = this->_instance.get()->GetVkDevice();
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);

    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
      vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
      vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
  }

    bool has_stensil_component(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
  }
};

int main() {
  HelloTriangleApplication app;

  try {
    app.Run();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}