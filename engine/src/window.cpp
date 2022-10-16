#include "engine/private/events/window.h"
#include "engine/private/events/mouse_event.h"
#include "engine/private/window.h"
#include "linux_window.h"
#include "core.h"



namespace engine {

engine::IWindowController* engine::IWindowController::Create() {
  engine::IWindowController* controller = nullptr;
  #ifdef ENGINE_PLATFORM_LINUX
    auto window_controller = engine::p_linux::WindowController::Create();
  #else
    #error Only Linux is supported
  #endif
  return controller;
}
} // namespace engine
