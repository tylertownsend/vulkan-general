#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <memory>

#include "swapchain.h"
#include "depth_resources.h"
#include "descriptor_set_layout.h"
#include "descriptor.h"
#include "graphics_pipeline.h"
#include "vulkan.h"
#include "window.h"

namespace VT {

class SwapchainManager {
  std::unique_ptr<VT::Swapchain> _swapchain;
  std::unique_ptr<VT::DescriptorSetLayout> _descriptor_set_layout;
  std::unique_ptr<VT::GraphicsPipeline> _graphics_pipeline;
  std::unique_ptr<VT::DepthResources> _depth_resources;

  std::vector<VkFramebuffer> swapChainFramebuffers;
  std::unique_ptr<VT::DescriptorSets> _descriptor_sets;

  const std::shared_ptr<VT::Vulkan> _instance;
  int _max_frames_in_flight;

public:
  SwapchainManager(
      const std::shared_ptr<VT::Vulkan>& instance,
      const std::unique_ptr<VT::CommandPool>& command_pool,
      const std::unique_ptr<VT::TextureView>& texture_image,
      const std::unique_ptr<phx::Window>& window,
      int max_frames_in_flight): _instance(instance),
                                 _max_frames_in_flight(max_frames_in_flight) {
    create_swapchain(window);
    create_descriptor_set_layout();
    create_graphics_pipeline();
    create_depth_resources(command_pool);

    // moving frame buffers to make sure it is called after the depth image
    // view has ben created
    create_frame_buffers();
    create_descriptor_sets(texture_image);
  }

  ~SwapchainManager() {
    auto instance = this->_instance->GetVkInstance();
    auto device = this->_instance->GetVkDevice();
    this->cleanup_swap_chain();
  }

  VkResult AcquireNextImage(std::vector<VkSemaphore>& image_available_semaphores, uint32_t current_frame, uint32_t& image_index) {
    return vkAcquireNextImageKHR(
        _instance->GetVkDevice(),
        _swapchain->GetSwapchain(),
        UINT64_MAX,
        image_available_semaphores[current_frame],
        VK_NULL_HANDLE,
        &image_index);
  }

  VkResult QueuePresentKHR(VkSemaphore signal_semaphores[], uint32_t image_index) {
    VkSwapchainKHR swapChains[] = {_swapchain->GetSwapchain()};

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signal_semaphores;
    presentInfo.swapchainCount = 1;
    // The next two parameters specify the swap chains to present
    // images to and the index of the image for each swap chain. This will almost always
    // be a single one.
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &image_index;
    // There is one last optional parameter called pResults. It allows you to specify an array
    // of VkResult values to check for every individual swap chain if presentation was successfu
    presentInfo.pResults = nullptr;

    return vkQueuePresentKHR(_instance->GetPresentQueue(), &presentInfo);
  }

  void UpdateUnfiformBuffer(uint32_t current_frame) {
    VT::UpdateUniformBuffer(_instance->GetVkDevice(), _descriptor_sets->GetUniformBufferMemory(), _swapchain->GetExtent(), current_frame);
  }

  void CompleteRenderPass(
      VkCommandBuffer command_buffer,
      uint32_t image_index,
      uint32_t current_frame,
      VkBuffer vertex_buffer,
      VkBuffer index_buffer,
      std::vector<uint32_t>& indices) {
    // The first parameters are the render pass itself and the attachments to bind. We created a framebuffer for
    // each swap chain image where it is specified as a color attachment.
    // Thus we need to bind the framebuffer for the swapchain image we want to draw to. 
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _graphics_pipeline->GetRenderPass();
    renderPassInfo.framebuffer = swapChainFramebuffers[image_index];
    // The render area defines where shader loads and stores will take place.
    // The pixels outside this region will have undefined values.
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _swapchain->GetExtent();

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
    vkCmdBeginRenderPass(command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    this->BindGraphicsPipeline(command_buffer);

    // bind vertex buffer during rendering operations
    VkBuffer vertexBuffers[] = {vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

    // Descriptor sets can be used in graphics or compute pipelines so we need to specify
    // which one to use.
    this->BindDescriptorSets(command_buffer, current_frame);

    // vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
    // instanceCount: Used for instanced rendering, use 1 if you're not doing that.
    // firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
    // firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex
    vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    vkCmdEndRenderPass(command_buffer);
  }

  void BindGraphicsPipeline(VkCommandBuffer command_buffer) {
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline->GetPipeline());
  }

  void BindDescriptorSets(VkCommandBuffer command_buffer, uint32_t current_frame) {
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline->GetPipelineLayout(), 0, 1, &_descriptor_sets->GetDescriptorSets()[current_frame], 0, nullptr);
  }

  // However, the disadvantage of this approach is that we need to stop all rendering before
  // creating the new swap chain. It is possible to create a new swap chain while drawing
  // commands on an image from the old swap chain are still in-flight. You need to pass the
  // previous swap chain to the oldSwapChain field in the VkSwapchainCreateInfoKHR struct
  // and destroy the old swap chain as soon as you've finished using it.
  void RecreateSwapchain(
      const std::unique_ptr<phx::Window>& window,
      const std::unique_ptr<VT::CommandPool>& command_pool,
      const std::unique_ptr<VT::TextureView>& texture_image) {
    std::cout << "Recreating swapshain" << std::endl;
    window->HandleMinimization();

    // don't touch resources still in use
    vkDeviceWaitIdle(_instance->GetVkDevice());

    cleanup_swap_chain();

    // Note that in chooseSwapExtent we already query the new window resolution to make sure
    // that the swap chain images have the (new) right size, so there's no need to modify
    // chooseSwapExtent (remember that we already had to use glfwGetFramebufferSize get the
    // resolution of the surface in pixels when creating the swap chain).
    create_swapchain(window);
    create_graphics_pipeline();
    create_depth_resources(command_pool);
    create_frame_buffers();
    create_descriptor_sets(texture_image);
    // TODO: consider reseting command pool after swapchain recreation.
    command_pool->ResetCommandBuffers();
  }

private:
  void create_swapchain(const std::unique_ptr<phx::Window>& window) {
    _swapchain = std::make_unique<VT::Swapchain>(_instance, window);
  }

  void create_descriptor_set_layout() {
    _descriptor_set_layout = std::make_unique<VT::DescriptorSetLayout>(_instance, _swapchain->GetImageFormat());
  }

  void create_graphics_pipeline() {
    _graphics_pipeline = std::make_unique<VT::GraphicsPipeline>(_instance, _swapchain, _descriptor_set_layout);
  }

  void create_depth_resources(const std::unique_ptr<VT::CommandPool>& command_pool) {
    _depth_resources = std::make_unique<VT::DepthResources>(_instance, command_pool, _swapchain->GetExtent());
  }

  void create_frame_buffers() {
    VT::CreateFrameBuffersOptions options {_instance->GetVkDevice(), _graphics_pipeline->GetRenderPass(), _swapchain->GetExtent(), _swapchain->GetSwapChainImageViews() };
    VT::CreateFrameBuffers(options, _depth_resources->GetDepthImageView(), swapChainFramebuffers);
  }

  void create_descriptor_sets(const std::unique_ptr<VT::TextureView>& texture_image) {
    _descriptor_sets = std::make_unique<VT::DescriptorSets>(_instance, _descriptor_set_layout, texture_image, _max_frames_in_flight);
  }

  void cleanup_swap_chain() {
    auto device = this->_instance.get()->GetVkDevice();

    delete _depth_resources.release();

    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
      vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    delete _swapchain.release();
  }
};
} //namespace VT