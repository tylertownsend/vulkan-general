#include "engine/private/events/window.h"
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
  glfwDestroyWindow(this->window);
  glfwTerminate();
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
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(options.width, options.height, options.title.c_str(), nullptr, nullptr);
  glfwMakeContextCurrent(window);

  // glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
  //   engine::WindowOptions& data = *(engine::WindowOptions*)glfwGetWindowUserPointer(window);
  //   data.width = width;
  //   data.height = height;

  //   WindowResizeEvent event(width, height);
  //   data.callback(event);
  // });

  glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
    engine::WindowOptions& data = *(engine::WindowOptions*)glfwGetWindowUserPointer(window);
    auto event = std::make_unique<WindowOnCloseEvent>();
    data.callback(std::move(event));
  });

  // glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
  // {
  //   WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

  //   switch (action)
  //   {
  //     case GLFW_PRESS:
  //     {
  //       KeyPressedEvent event(key, 0);
  //       data.EventCallback(event);
  //       break;
  //     }
  //     case GLFW_RELEASE:
  //     {
  //       KeyReleasedEvent event(key);
  //       data.EventCallback(event);
  //       break;
  //     }
  //     case GLFW_REPEAT:
  //     {
  //       KeyPressedEvent event(key, true);
  //       data.EventCallback(event);
  //       break;
  //     }
  //   }
  // });

  // glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
  // {
  //   WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

  //   KeyTypedEvent event(keycode);
  //   data.EventCallback(event);
  // });

  // glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
  // {
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

  // glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
  // {
  //   WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

  //   MouseScrolledEvent event((float)xOffset, (float)yOffset);
  //   data.EventCallback(event);
  // });

  glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x_pos, double y_pos) {
    engine::WindowOptions& data = *(WindowOptions*)glfwGetWindowUserPointer(window);
    auto event = std::make_unique<MouseMoveEvent>((float)x_pos, (float)y_pos);
    std::cout << event->x_offset << "," << event->y_offset << std::endl;
    data.callback(std::move(event));
  });
  return window;
}

void WindowController::OnUpdate(std::unique_ptr<engine::Window>& window) {
  glfwPollEvents();
}


} // namespace p_linux
} // namespace engine