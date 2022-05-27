#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string.h>
#include <stdexcept>
#include <memory>
#include <vector>

#include "surface.h"
#include "vulkan_debug.h"
#include "window.h"

#ifdef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

const std::vector<const char*> validationLayers = {
  "VK_LAYER_KHRONOS_validation"
};

namespace VT {

struct VulkanInstanceInfo {
  const VkInstance* instance;
  const VkDebugUtilsMessengerEXT* debug_messenger;
  GLFWwindow* window;
  const VkSurfaceKHR* surface;
  const VkPhysicalDevice* physical_device;
  const VkDevice* device;
};

struct VulkanOptions {
  const std::string application_name;
  const std::string engine_name;
  const char** extensions_array;
  uint32_t extension_count;

  VulkanOptions operator=(const VulkanOptions& other) const {
    return VulkanOptions {
      other.application_name,
      other.engine_name,
      other.extensions_array,
      other.extension_count
    };
  }
};

class Vulkan {
  std::unique_ptr<VulkanInstanceInfo> _instance_info;
  std::unique_ptr<VulkanOptions> _options;

  Vulkan(std::unique_ptr<VulkanOptions>&& options, std::unique_ptr<VulkanInstanceInfo>&& instance_info) {
    _options = std::move(options);
    _instance_info = std::move(instance_info);
  }

  ~Vulkan() {
    auto info = *_instance_info.get();
    vkDestroyDevice(*info.device, nullptr);

    if (enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(*info.instance, *info.debug_messenger, nullptr);
    }

    vkDestroySurfaceKHR(*info.instance, *info.surface, nullptr);
    vkDestroyInstance(*info.instance, nullptr);

    glfwDestroyWindow(info.window);

    glfwTerminate();
  }

public:
  Vulkan* CreateInstance(const VulkanOptions options) {
    auto instance = CreateVkInstance(options);

    auto debug_messenger = setup_debug_messenger(*instance, enableValidationLayers);

    WindowOptions window_options{};
    window_options.height = 600;
    window_options.width = 800;
    window_options.title = "Townsend Window";
    window_options.user_pointer = this;
    window_options.framebuffer_resize_callback = nullptr;
    auto window = CreateWindow(window_options);

    VulkanSurfaceOptions surface_options {
      instance,
      window
    };

    auto surface = VulkanSurface::CreateSurface(surface_options);

    auto instance_info = std::make_unique<VulkanInstanceInfo>(new VulkanInstanceInfo{
      instance,
      debug_messenger,
      window,
      surface
    });

    return new Vulkan(options, instance_info);
  }

private:
  VkInstance* CreateVkInstance(const VulkanOptions& options) {
    auto extensions = get_required_extensions(options.extensions_array, options.extension_count);
    VkInstance* instance;

    if (enableValidationLayers && !check_validation_layer_support()) {
      throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = options.application_name.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = options.engine_name.c_str();
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

  VkDebugUtilsMessengerEXT* setup_debug_messenger(VkInstance instance, bool enable_validation_layers) {
    VkDebugUtilsMessengerEXT* debug_messenger;
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, debug_messenger) != VK_SUCCESS) {
      throw std::runtime_error("failed to set up debug messenger!");
    }
    return debug_messenger;
  }

  std::vector<const char*> get_required_extensions(const char** extensions_array, uint32_t extension_count) {
    std::vector<const char*> extensions(extensions_array, extensions_array + extension_count);

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
};
} // VT