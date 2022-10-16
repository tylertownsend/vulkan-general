#include <iostream>
#include <memory>

#include "engine/private/events/window.h"
#include "engine/private/application.h"

#include "window.h"
#include "core.h"

namespace engine {

struct ApplicationState {
  bool running;
  ApplicationState(): running(true) {}
};

// todo start off will callbacks inlined (non-linear creation);
// then see if create class methods ease the flow
//  - e .g, on Update defined and then referenced in a callback to create
//  - linear c4reation
Application::Application() {}

Application::~Application() {}

void Application::Run() {

  auto application_state = std::make_unique<ApplicationState>();

  auto event_dispatcher = std::make_unique<EventDispatcher>();

// #ifdef ENGINE_PLATFORM_LINUX
  auto window_controller = std::unique_ptr<engine::IWindowController>(engine::p_linux::WindowController::Create());

  WindowOptions data([&event_dispatcher](std::unique_ptr<Event> event) {
    event_dispatcher->Offer(event);

    // I have moved data into the event dispatcher and no longer can process it.
    auto event_data = *event.get();
    std::cout << event_data << std::endl;
  });
  auto window = std::unique_ptr<engine::Window>(window_controller->CreateWindow(data));
// #else
//   #error Only Linux is supported
// #endif

  event_dispatcher->Listen<WindowOnCloseEvent>(EventType::WindowClose, [&application_state](WindowOnCloseEvent& event) {
    application_state->running = false;
    std::cout << "Closing Window\n";
  });

  while (application_state->running) {
    window_controller->OnUpdate(window);
  }
}
}