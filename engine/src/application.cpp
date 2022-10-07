#include <iostream>

#include "engine/private/application.h"

#include "window.h"
#include "core.h"

namespace engine {

Application::Application() {

// #ifdef ENGINE_PLATFORM_LINUX
  WindowOptions data(nullptr);
  this->window_ = engine::p_linux::Window::Create(data);
// #else
//   #error Only Linux is supported
// #endif
}

Application::~Application() {}

void Application::Run() {
  while (true);
}
}