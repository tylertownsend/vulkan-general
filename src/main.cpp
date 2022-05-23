#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

# define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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

// number of frames to be processed concurrently.
const int MAX_FRAMES_IN_FLIGHT = 2;
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;


const std::vector<const char*> validationLayers = {
  "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


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

#ifdef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

class HelloTriangleApplication {
public:
  void Run() {
    init_window();
    init_vulkan();
    main_loop();
    cleanup();
  }

private:
  GLFWwindow* window;

  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkSurfaceKHR surface;

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VT::QueueFamilyIndices _indices;
  VkDevice device;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

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

  bool framebufferResized = false;

  uint32_t currentFrame = 0;

  void init_window() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
  }

  static void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
  }

  void init_vulkan() {
    create_instance();
    setup_debug_messenger();
    create_surface();
    pick_physical_device();
    create_logical_device();
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

  void main_loop() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      draw_frame();
    }
    vkDeviceWaitIdle(device);
  }

  void cleanup() {
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

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
  }

  void create_instance() {
    if (enableValidationLayers && !check_validation_layer_support()) {
      throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = get_required_extensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();

      PopulateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;

      createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
      throw std::runtime_error("failed to create instance!");
    }
  }

  void setup_debug_messenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
      throw std::runtime_error("failed to set up debug messenger!");
    }
  }

  void create_surface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
      throw std::runtime_error("failed to create window surface!");
    }
  }

  void pick_physical_device() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
      throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
      auto queue_family_indices = is_device_suitable(device);
      if (queue_family_indices.IsComplete()) {
        physicalDevice = device;
        _indices = queue_family_indices;
        break;
      }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
      throw std::runtime_error("failed to find a suitable GPU!");
    }
  }

  void create_logical_device() {
    VT::QueueFamilyIndices indices = _indices;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
      throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
  }

  void create_image_views() {
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
      VT::ImageViewOptions options{};
      options.device = device;
      options.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
      options.format = swapChainImageFormat;
      options.image = swapChainImages[i];
      swapChainImageViews[i] = VT::CreateImageView(options);
    }
  }

  void create_swapchain() {
    VT::SwapChainOptions options{ &instance, &surface, &physicalDevice, &device, window };
    auto swapchainInfo = VT::CreateSwapchain(options);
    swapChain = swapchainInfo.swapchain;
    swapChainImages = swapchainInfo.swapchain_images;
    swapChainImageFormat = swapchainInfo.swapchain_image_format;
    swapChainExtent = swapchainInfo.swapchain_extent;
  }


  void create_render_pass() {
    VT::RenderPassOptions options{swapChainImageFormat, device, physicalDevice};
    renderPass = VT::CreateRenderPass(options);
  }

  void create_descriptor_set_layout() {
    VT::DescriptorSetLayoutOptions options{ swapChainImageFormat, device };
    descriptorSetLayout = VT::CreateDescriptorSetLayout(options);
  }

  void create_graphics_pipeline() {
    VT::GraphicsPipelineOptions options {device, renderPass, descriptorSetLayout, swapChainExtent };
    auto result = VT::CreateGraphicsPipeline(options);
    graphicsPipeline = result.graphics_pipeline;
    pipelineLayout = result.pipeline_layout;
  }

  void create_frame_buffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++ ) {

      std::array<VkImageView, 2> attachments = {
        swapChainImageViews[i],
        depthImageView
      };

      VkFramebufferCreateInfo framebufferInfo{};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      // The attachmentCount and pAttachments parameters specify the VkImageView objects that should
      // be bound to the respective attachment descriptions in the render pass pAttachment array.
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
      framebufferInfo.pAttachments = attachments.data();;
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
      }
    }
  }

  void create_command_pool() {
    VT::QueueFamilyIndices queueFamilyIndices = _indices;

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // We will be recording a command buffer every frame, so we want to be able to
    // reset and rerecord over it. Thus, we need to set the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag bit for our command pool.
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    
    // Command buffers are executed by submitting them on one of the device queues,
    // like the graphics and presentation queues we retrieved
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
      throw std::runtime_error("failed to create command pool!");
    }
  }

  void create_depth_resources() {

    VkFormat depthFormat = find_depth_format();

    VT::CreateImageOptions imageOptions(
      swapChainExtent.width,
      swapChainExtent.height,
      depthFormat,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      device,
      physicalDevice
    );
    VT::CreateImage(imageOptions, depthImage, depthImageMemory);

    VT::ImageViewOptions options{};
    options.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    options.image = depthImage;
    options.format = depthFormat;
    options.device = device;
    depthImageView = VT::CreateImageView(options);
    VT::TransitionImageLayout(device,
                              commandPool,
                              graphicsQueue,
                              depthImage,
                              depthFormat,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  }


  void create_texture_image() {
    VT::CreateTextureImageOptions options{
      device,
      physicalDevice,
      commandPool,
      graphicsQueue
    };
    VT::CreateTextureImage(options, textureImage, textureImageMemory);
  }

  void create_texture_image_view() {
    VT::ImageViewOptions options{};
    options.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    options.format = VK_FORMAT_R8G8B8A8_SRGB;
    options.device = device;
    options.image = textureImage;
    textureImageView = VT::CreateImageView(options);
  }

  void create_texture_sampler() {
    VT::CreateTextureSamplerOptions options { device, physicalDevice };
    textureSampler = VT::CreateTextureImageSampler(options);
  }

  void load_model() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // obj files can contain an arbitrary number of vertices whereas our application
    // can only render triangles. Luckily the LoadObj has optional parameter to triangulate
    // such faces which is enabled by default.
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, VT::MODEL_PATH.c_str())) {
      throw std::runtime_error(warn + err);
    }

    std::unordered_map<VT::Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes) {
      for (const auto& index: shape.mesh.indices) {
        VT::Vertex vertex{};

        vertex.pos = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2],
        };

        vertex.texCoord = {
          attrib.texcoords[2 * index.texcoord_index + 0],
          1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
        };

        vertex.color = { 1.0f, 1.0f, 1.0f };

        if (uniqueVertices.count(vertex) == 0) {
          uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
          vertices.push_back(vertex);
        }

        indices.push_back(uniqueVertices[vertex]);
      }
    }
  }

  void create_vertex_buffer() {
    VT::CreateVertexBufferOptions options{device, physicalDevice, commandPool, graphicsQueue, vertices};
    VT::CreateVertexBuffer(options, vertexBuffer, vertexBufferMemory);
  }

  void create_index_buffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VT::CreateBuffer(bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer,
                    stagingBufferMemory, device, physicalDevice);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    VT::CreateBuffer(bufferSize,
                        // use index bit instead of vertex bit
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        indexBuffer,
                        indexBufferMemory, device, physicalDevice);
    VT::CopyBufferOptions copy_buffer_options {device, commandPool, graphicsQueue};
    VT::CopyBuffer(copy_buffer_options, stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
  }

  void create_uniform_buffers() {
    VT::CreateUniformBufferOptions options {MAX_FRAMES_IN_FLIGHT, device, physicalDevice};
    VT::CreateUniformBuffers(options, uniformBuffers, uniformBuffersMemory);
  }

  void create_descriptor_pool() {
    VT::CreateDescriptorPoolOptions options { device, MAX_FRAMES_IN_FLIGHT };
    VT::CreateDescriptorPools(options, descriptorPool);
  }

  // We create one descriptor set for each swap chain image
  // with all the same layout.
  // we do need copies of all the layouts because the next function
  // expects an array matching the number of sets
  void create_descriptor_sets() {
    VT::CreateDescriptorSetOptions options {
      device,
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
    VT::CreateCommandBuffersOptions options { device, commandPool, MAX_FRAMES_IN_FLIGHT};
    VT::CreateCommandBuffers(options, commandBuffers);
  }

  void create_sync_objects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
          vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
          vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphores!");
      }
    }
  }

  void draw_frame() {
    uint32_t imageIndex;
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

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

    VT::UpdateUniformBuffer(device, uniformBuffersMemory, swapChainExtent, currentFrame);

    // delay resetting fence until after we know for sure we will be submitting work with it.
    // in the case of recreating swap chain:
    //   the current frame's fence was waited upon and reset. Since we returned immediately,
    //   no work is submitted for execution and the fence will never be signalled causing to 
    //   wait forever.
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

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

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
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

    VkResult queueResult = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (queueResult == VK_ERROR_OUT_OF_DATE_KHR ||
        queueResult == VK_SUBOPTIMAL_KHR ||
        framebufferResized) {
      // make sure to do this after queuepresentKHR to make sure semaphores are in consistent
      // state, otherwise a signalled semaphore may never be properly waited upon.
      this->framebufferResized = false;
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
    handle_minimization();

    // don't touch resources still in use
    vkDeviceWaitIdle(device);

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

  // Minimization results in a framebuffer of size 0.
  // Tutorial mode :) - we pause process until window is in foreground;
  void handle_minimization() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
      glfwGetFramebufferSize(window, &width, &height);
      glfwWaitEvents();
    }
  }

  VT::QueueFamilyIndices is_device_suitable(VkPhysicalDevice device) {
    VT::QueueFamilyIndices indices = VT::FindQueueFamilies(device, surface);

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    bool extensionsSupported = check_device_extension_support(device);
    bool swapChainAdequate = VT::CheckSwapChainAdequate(extensionsSupported, device, surface);

    if (indices.IsComplete() && 
        extensionsSupported && 
        swapChainAdequate && 
        supportedFeatures.samplerAnisotropy) {
      return indices;
    } else {
      VT::QueueFamilyIndices incomplete_indices;
      return incomplete_indices;
    }
  }

  bool check_device_extension_support(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
  }


  std::vector<const char*> get_required_extensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
  }

  bool check_validation_layer_support() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
      bool layerFound = false;

      for (const auto& layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          layerFound = true;
          break;
        }
      }

      if (!layerFound) {
        return false;
      }
    }

    return true;
  }

  VkFormat find_depth_format() {
    return find_supported_format(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
  }

  /**
   * @brief Takes a list of candidates from most desirable to least desirable and
   * returns the first one that is supported.
   * 
   * @param candidates 
   * @param tiling 
   * @param features 
   * @return VkFormat The most desirable supported format.
   */
  VkFormat find_supported_format(
      const std::vector<VkFormat>& candidates, 
      VkImageTiling tiling,
      VkFormatFeatureFlags features) {

    for (auto format : candidates) {
      VkFormatProperties props;

      vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

      if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features)) {
        return format;
      } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
        return format;
      }
    }
    throw std::runtime_error("failed to find supported format!");
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