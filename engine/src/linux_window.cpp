#include "engine/private/events/window.h"
#include "engine/private/events/key_event.h"
#include "engine/private/events/mouse_event.h"
#include "engine/private/window.h"
#include "linux_window.h"
#include "core.h"

engine::IWindowController* engine::IWindowController::Create() {
  auto controller = engine::p_linux::WindowController::Create();
  return controller;
}

namespace engine {
namespace p_linux {

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

WindowController::~WindowController() {
  
}

WindowController* WindowController::Create() {
  return new WindowController();
}

engine::Window* WindowController::CreateWindow(const engine::WindowOptions& options) {
  auto glfw_window = create_glfw_window(options);
  auto window = new engine::p_linux::Window(options, glfw_window);
  glfwSetWindowUserPointer(window->window, &(window->data));
  return window;
}

GLFWwindow* WindowController::create_glfw_window(const engine::WindowOptions& options) {
  GLFWwindow* window;
  auto res = glfwInit();
  if (!res) {
    std::cout << "Failed to initialize glfw\n";
    exit(EXIT_FAILURE);
  }
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwSwapInterval(1); // Enable vsync

  window = glfwCreateWindow(options.width, options.height, options.title.c_str(), nullptr, nullptr);
  glfwMakeContextCurrent(window);
  if (glfwGetCurrentContext()) {
    std::cout << "initialized!\n";
  }

  glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
    engine::WindowOptions& data = *(engine::WindowOptions*)glfwGetWindowUserPointer(window);
    data.width = width;
    data.height = height;

    auto event = std::make_unique<WindowResizeEvent> (width, height);
    data.callback(std::move(event));
  });

  glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
    engine::WindowOptions& data = *(engine::WindowOptions*)glfwGetWindowUserPointer(window);
    auto event = std::make_unique<WindowOnCloseEvent>();
    data.callback(std::move(event));
  });

  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    engine::WindowOptions& data = *(engine::WindowOptions*)glfwGetWindowUserPointer(window);

    switch (action) {
      case GLFW_PRESS: {
        auto event = std::make_unique<KeyPressedEvent>(key, 0);
        data.callback(std::move(event));
        break;
      }
      case GLFW_RELEASE: {
        auto event = std::make_unique<KeyReleasedEvent>(key);
        data.callback(std::move(event));
        break;
      }
      case GLFW_REPEAT: {
        auto event = std::make_unique<KeyPressedEvent>(key, true);
        data.callback(std::move(event));
        break;
      }
    }
  });

  // glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
  //   WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

  //   switch (action)
  //   {
  //     case GLFW_PRESS:
  //     {
  //       MouseButtonPressedEvent event(button);
  //       data.EventCallback(event);
  //       break;
  //     }
  //     case GLFW_RELEASE:
  //     {
  //       MouseButtonReleasedEvent event(button);
  //       data.EventCallback(event);
  //       break;
  //     }
  //   }
  // });

  glfwSetScrollCallback(window, [](GLFWwindow* window, double x_offset, double y_offset) {
    engine::WindowOptions& data = *(engine::WindowOptions*)glfwGetWindowUserPointer(window);

    auto event = std::make_unique<MouseScrollEvent>((float)x_offset, (float)y_offset);
    data.callback(std::move(event));
  });

  glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x_pos, double y_pos) {
    engine::WindowOptions& data = *(WindowOptions*)glfwGetWindowUserPointer(window);
    auto event = std::make_unique<MouseMoveEvent>((float)x_pos, (float)y_pos);
    data.callback(std::move(event));
  });
  return window;
}

void WindowController::OnUpdate(std::unique_ptr<engine::Window>& window) {
  glfwPollEvents();
  auto platform_window = static_cast<engine::p_linux::Window*>(window.get());
  auto opengl_window = static_cast<GLFWwindow*>(platform_window->window);
  glfwSwapBuffers(opengl_window);
}
} // namespace p_linux
} // namespace engine