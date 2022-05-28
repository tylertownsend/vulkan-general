#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string.h>
#include <stdexcept>
#include <memory>
#include <vector>

#include "device.h"
#include "logical_device.h"
#include "queue_families.h"
#include "surface.h"
#include "vulkan_debug.h"
#include "window.h"

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = true;
#else
const bool ENABLE_VALIDATION_LAYERS = false;
#endif

const std::vector<const char*> DEVICE_EXTENSIONS = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> VALIDATION_LAYERS = {
  "VK_LAYER_KHRONOS_validation"
};

namespace VT {

struct VulkanInstanceInfo {
  const VkInstance* instance;
  const VkDebugUtilsMessengerEXT* debug_messenger;
  const VkSurfaceKHR* surface;
  const VkPhysicalDevice* physical_device;
  const VkDevice* device;
  VT::QueueFamilyIndices queue_family_indices;
  VkQueue* graphics_queue;
  VkQueue* present_queue;
};

struct VulkanOptions {
  GLFWwindow* window;
  const char* application_name;
  const char* engine_name;
  // const std::vector<const char*> extensions_array;
  // uint32_t extension_count;

  VulkanOptions(
      GLFWwindow* window,
      const char* application_name,
      const char* engine_name) :window(window),
                                 application_name(application_name),
                                 engine_name(engine_name) {}

  VulkanOptions operator=(const VulkanOptions& other) const {
    return VulkanOptions {
      other.window,
      other.application_name,
      other.engine_name,
    };
  }
};

class Vulkan {
 private:
  std::unique_ptr<VulkanInstanceInfo> _instance_info;
  std::unique_ptr<VulkanOptions> _options;
  std::unique_ptr<phx::Window> _window;


  bool _frame_buffer_resized = false;

public:
  Vulkan(std::unique_ptr<VulkanOptions>& options, std::unique_ptr<VulkanInstanceInfo>& instance_info):
      _options(std::move(options)),
      _instance_info(std::move(instance_info)) {}

  ~Vulkan() {
    auto info = *_instance_info.get();
    vkDestroyDevice(*info.device, nullptr);

    if (ENABLE_VALIDATION_LAYERS) {
      DestroyDebugUtilsMessengerEXT(*info.instance, *info.debug_messenger, nullptr);
    }

    vkDestroySurfaceKHR(*info.instance, *info.surface, nullptr);
    vkDestroyInstance(*info.instance, nullptr);

  }

  const VkInstance GetVkInstance() {
    return *this->_instance_info.get()->instance;
  }

  const VkDebugUtilsMessengerEXT GetVkDebugUtilsMessengerEXT() {
    return *this->_instance_info.get()->debug_messenger;
  }
  const VkSurfaceKHR GetVkSurfaceKHR() {
    return *this->_instance_info.get()->surface;
  }

  const VkPhysicalDevice GetVkPhysicalDevice() {
    return *this->_instance_info.get()->physical_device;
  }

  const VkDevice GetVkDevice() {
    return *this->_instance_info.get()->device;
  }

  const VkResult DeviceWaitIdle() {
    return vkDeviceWaitIdle(*this->_instance_info.get()->device);
  }

  const VT::QueueFamilyIndices& GetQueueFamilyIndices() {
    return this->_instance_info->queue_family_indices;
  }

  const VkQueue GetGraphicsQueue() {
    return *this->_instance_info->graphics_queue;
  }

  const VkQueue GetPresentQueue() {
    return *this->_instance_info->present_queue;
  }

  static std::unique_ptr<Vulkan> CreateInstance(const VulkanOptions& options) {
    VkInstance* instance;
    CreateVkInstance(options, instance);

    VkDebugUtilsMessengerEXT* debug_messenger;
    setup_debug_messenger(*instance, ENABLE_VALIDATION_LAYERS, debug_messenger);

    VulkanSurfaceOptions surface_options {
      instance,
      options.window
    };

    VkSurfaceKHR* surface;
    VulkanSurface::CreateSurface(surface_options, surface);

    
    VkPhysicalDevice* physical_device = VK_NULL_HANDLE;
    VT::PickPhysicalDevice(*instance, *surface, DEVICE_EXTENSIONS, ENABLE_VALIDATION_LAYERS, *physical_device);

    QueueFamilyIndices queue_family_indices;
    VT::FindQueueFamilies(*physical_device, *surface, queue_family_indices);

    VkDevice* device;
    VT::CreateLogicalDevice(queue_family_indices, *physical_device, VALIDATION_LAYERS, ENABLE_VALIDATION_LAYERS, DEVICE_EXTENSIONS, device);

    VkQueue* graphics_queue;
    VT::GetDeviceQueue(*device, queue_family_indices.graphicsFamily.value(), *graphics_queue);
    VkQueue* present_queue;
    VT::GetDeviceQueue(*device, queue_family_indices.graphicsFamily.value() , *graphics_queue);

    auto instance_info = std::make_unique<VulkanInstanceInfo>(VulkanInstanceInfo{
      instance,
      debug_messenger,
      surface,
      physical_device,
      device,
      queue_family_indices,
      graphics_queue,
      present_queue
  });

    auto vulkan_options = std::make_unique<VulkanOptions>(options);

    return std::make_unique<Vulkan>(vulkan_options, instance_info);
  }

// private:
  static void CreateVkInstance(const VulkanOptions& options, VkInstance* instance) {
    auto extensions = VT::get_required_extensions(ENABLE_VALIDATION_LAYERS);
    // VkInstance* instance;

    if (ENABLE_VALIDATION_LAYERS && !CheckValidationLayerSupport(VALIDATION_LAYERS)) {
      throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = options.application_name;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = options.engine_name;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (ENABLE_VALIDATION_LAYERS) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
      createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
      PopulateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, instance) != VK_SUCCESS) {
      throw std::runtime_error("failed to create instance!");
    }
  }

  static void setup_debug_messenger(VkInstance instance, bool enable_validation_layers, VkDebugUtilsMessengerEXT* debug_messenger) {
    if (!enable_validation_layers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, debug_messenger) != VK_SUCCESS) {
      throw std::runtime_error("failed to set up debug messenger!");
    }
  }
};
} // VT