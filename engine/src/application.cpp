#include <iostream>
#include <memory>

#include "engine/private/application.h"

#include "window.h"
#include "core.h"

namespace engine {

Application::Application() {

// #ifdef ENGINE_PLATFORM_LINUX
  auto window_controller = engine::p_linux::WindowController::Create(nullptr);

  WindowOptions data(nullptr);
  auto window = window_controller->CreateWindow(data);

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