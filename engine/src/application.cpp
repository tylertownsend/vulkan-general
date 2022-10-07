#include <iostream>
#include <memory>

#include "engine/private/application.h"

#include "window.h"
#include "core.h"

namespace engine {

Application::Application() {

// #ifdef ENGINE_PLATFORM_LINUX
  WindowOptions data(nullptr);
  auto window = engine::p_linux::Window::Create(data);
  window_ = std::unique_ptr<engine::Window>(window);
// #else
//   #error Only Linux is supported
// #endif
}

Application::~Application() {}

void Application::Run() {
  while (true);
}
}