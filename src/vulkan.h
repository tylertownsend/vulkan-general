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
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

const std::vector<const char*> deviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> validationLayers = {
  "VK_LAYER_KHRONOS_validation"
};

namespace VT {

struct VulkanInstanceInfo {
  const VkInstance* instance;
  const VkDebugUtilsMessengerEXT* debug_messenger;
  const VkSurfaceKHR* surface;
  const VkPhysicalDevice* physical_device;
  const VkDevice* device;
  VT::QueueFamilyIndices queue_familiy_indices;
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

    if (enableValidationLayers) {
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

  const VT::QueueFamilyIndices& GetQueueFamiliyIndices() {
    return this->_instance_info->queue_familiy_indices;
  }

  const VkQueue GetGraphicsQueue() {
    return *this->_instance_info->graphics_queue;
  }

  const VkQueue GetPresentQueue() {
    return *this->_instance_info->present_queue;
  }

  static std::unique_ptr<Vulkan> CreateInstance(const VulkanOptions& options) {
    auto instance = CreateVkInstance(options);

    auto debug_messenger = setup_debug_messenger(*instance, enableValidationLayers);

    VulkanSurfaceOptions surface_options {
      instance,
      options.window
    };

    auto surface = VulkanSurface::CreateSurface(surface_options);

    auto physical_device = VT::PickPhysicalDevice(*instance, *surface, deviceExtensions, enableValidationLayers);

    auto queue_familiy_indices = VT::FindQueueFamilies(*physical_device, *surface);

    auto device = VT::CreateLogicalDevice(queue_familiy_indices, *physical_device, validationLayers, enableValidationLayers, deviceExtensions);

    VkQueue* graphics_queue;
    VT::GetDeviceQueue(*device, queue_familiy_indices.graphicsFamily.value(), *graphics_queue);
    VkQueue* present_queue;
    VT::GetDeviceQueue(*device, queue_familiy_indices.graphicsFamily.value() , *graphics_queue);

    auto instance_info = std::make_unique<VulkanInstanceInfo>(VulkanInstanceInfo{
      instance,
      debug_messenger,
      surface,
      physical_device,
      device,
      queue_familiy_indices,
      graphics_queue,
      present_queue
  });

    auto vulkan_options = std::make_unique<VulkanOptions>(options);

    return std::make_unique<Vulkan>(vulkan_options, instance_info);
  }

private:
  static VkInstance* CreateVkInstance(const VulkanOptions& options) {
    auto extensions = get_required_extensions(deviceExtensions, deviceExtensions.size(), enableValidationLayers);
    VkInstance* instance;

    if (enableValidationLayers && !check_validation_layer_support()) {
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
    if (enableValidationLayers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();

      PopulateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;

      createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, instance) != VK_SUCCESS) {
      throw std::runtime_error("failed to create instance!");
    }
    return instance;
  }

  static VkDebugUtilsMessengerEXT* setup_debug_messenger(VkInstance instance, bool enable_validation_layers) {
    VkDebugUtilsMessengerEXT* debug_messenger;
    if (!enable_validation_layers) return VK_NULL_HANDLE;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, debug_messenger) != VK_SUCCESS) {
      throw std::runtime_error("failed to set up debug messenger!");
    }
    return debug_messenger;
  }

  static std::vector<const char*> get_required_extensions(const std::vector<const char*>& extensions_array, uint32_t extension_count, bool enable_validation_layers) {
    // std::vector<const char*> extensions(extensions_array.data(), extensions_array.size() + extension_count);
    std::vector<const char*> extensions(extensions_array.begin(), extensions_array.end());

    if (enableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
  }


  static bool check_validation_layer_support() {
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
};
} // VT