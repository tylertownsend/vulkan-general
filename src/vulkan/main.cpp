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
#include "command_pool.h"
#include "depth_resources.h"
#include "swapchain_manager.h"

// number of frames to be processed concurrently.
const int MAX_FRAMES_IN_FLIGHT = 2;
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangleApplication {
public:
  void Run() {
    init_window();
    init_vulkan();
    main_loop();
    cleanup();
  }

private:
  std::unique_ptr<phx::Window> _window;
  std::shared_ptr<VT::Vulkan> _instance;

  std::unique_ptr<VT::CommandPool> _command_pool;
  std::unique_ptr<VT::TextureView> _texture_image;

  std::unique_ptr<VT::SwapchainManager> _swapchain_manager;

  std::vector<VT::Vertex> vertices;
  std::vector<uint32_t> indices;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;


  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;

  uint32_t currentFrame = 0;

  void init_vulkan() {
    create_instance();
    create_command_pool();
    create_texture_image();
    create_swapchain_manager();
    load_model();
    create_vertex_buffer();
    create_index_buffer();

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

    // check to see if unique_ptr_handles this
    // otherwise
    // delete swapchain_manager.release();
    // this->cleanup_swap_chain();

    // desstroy descriptor set layout
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
  }

  void create_instance() {
    const char* application_name = "vulkan_demo";
    const char* engine_name= "townsend engine";
    auto window = this->_window->GetGLFWwindow();
    VT::VulkanOptions options(window, application_name, engine_name);
    _instance = VT::CreateInstance(options);
  }

  void create_command_pool() {
    _command_pool = std::make_unique<VT::CommandPool>(_instance, MAX_FRAMES_IN_FLIGHT);
  }

  void create_texture_image() {
    _texture_image = std::make_unique<VT::TextureView>(_instance, _command_pool);
  }

  void create_swapchain_manager() {
    _swapchain_manager = std::make_unique<VT::SwapchainManager>(_instance, _command_pool, _texture_image, _window, MAX_FRAMES_IN_FLIGHT);
  }

  void load_model() {
    VT::LoadModel(vertices, indices, VT::MODEL_PATH.c_str());
  }

  void create_vertex_buffer() {
    VT::CreateVertexBufferOptions options{this->_instance.get()->GetVkDevice(), this->_instance.get()->GetVkPhysicalDevice(), _command_pool->GetCommandPool(), this->_instance->GetGraphicsQueue(), vertices};
    VT::CreateVertexBuffer(options, vertexBuffer, vertexBufferMemory);
  }

  void create_index_buffer() {
    VT::CreateIndexBufferOptions options{this->_instance.get()->GetVkDevice(), this->_instance.get()->GetVkPhysicalDevice(),  _command_pool->GetCommandPool(), this->_instance->GetGraphicsQueue() };
    VT::CreateIndexBuffer(options, indices, indexBuffer, indexBufferMemory);
  }

  void create_sync_objects() {
    VT::CreateSyncObjectsOptions options { this->_instance.get()->GetVkDevice(), MAX_FRAMES_IN_FLIGHT };
    VT::CreateSyncObjects(options, imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences);
  }

  void draw_frame() {
    uint32_t imageIndex;

    vkWaitForFences(_instance->GetVkDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    VkResult result = _swapchain_manager->AcquireNextImage(imageAvailableSemaphores, currentFrame, imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      // swap chain has become incompatiable with surface and can no longer be used for rendering
      // (e.g window resize)
      _swapchain_manager->RecreateSwapchain(_window, _command_pool, _texture_image);
      return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      // swap chain can be used to successufly present to surface but surface properties no
      // longer exactly match.
      throw std::runtime_error("failed to acquire swap chain image");
    }

    _swapchain_manager->UpdateUnfiformBuffer(currentFrame);

    // delay resetting fence until after we know for sure we will be submitting work with it.
    // in the case of recreating swap chain:
    //   the current frame's fence was waited upon and reset. Since we returned immediately,
    //   no work is submitted for execution and the fence will never be signalled causing to 
    //   wait forever.
    vkResetFences(_instance->GetVkDevice(), 1, &inFlightFences[currentFrame]);

    // call on command buffer to make sure it is able to be recorded.
    auto command_buffer = _command_pool->GetCommandBuffer(currentFrame);
    vkResetCommandBuffer(command_buffer, 0);
    // call to record the commands we want.
    record_command_buffer(command_buffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    // want to wait until the image is available before writing colors to the image
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer;

    // The signalSemaphoreCount and pSignalSemaphores specifcy which semaphores to signal
    // once the command buffer finishes execution.
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(_instance->GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }
    // submitting the result back to the swap chain to have it eventually show up on the screen
    VkResult queueResult = _swapchain_manager->QueuePresentKHR(signalSemaphores, imageIndex);

    if (queueResult == VK_ERROR_OUT_OF_DATE_KHR ||
        queueResult == VK_SUBOPTIMAL_KHR ||
        _window->FrameBufferResized()) {
      // make sure to do this after queuepresentKHR to make sure semaphores are in consistent
      // state, otherwise a signalled semaphore may never be properly waited upon.
      _window->ResetFrameBuffer();
      _swapchain_manager->RecreateSwapchain(_window, _command_pool, _texture_image);
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

    // TODO Consider Render Pass to be a part of the swapchain manager
    // The first parameters are the render pass itself and the attachments to bind. We created a framebuffer for
    // each swap chain image where it is specified as a color attachment.
    // Thus we need to bind the framebuffer for the swapchain image we want to draw to. 
    _swapchain_manager->CompleteRenderPass(commandBuffer, imageIndex, currentFrame, vertexBuffer, indexBuffer, indices);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }
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