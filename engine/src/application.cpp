#include <iostream>
#include <memory>

#include "engine/private/events/event_dispatcher.h"
#include "engine/private/events/window.h"
#include "engine/private/application.h"
#include "engine/private/log.h"
#include "engine/private/runtime_configuration.h"
#include "engine/private/window.h"
#include "engine/private/window_controller.h"
#include "core.h"

namespace engine {

struct ApplicationState {
  bool running;
  ApplicationState(): running(true) {}
};

Application::Application() {
  engine::monitor::LoggerOptions client_options("App");
  auto client_logger = engine::monitor::Create(engine::monitor::LoggerType::ClientLogger, client_options);
  client_logger->Info("Created Client Logger!");

  engine::monitor::LoggerOptions core_options("Engine");
  auto core_logger = engine::monitor::Create(engine::monitor::LoggerType::CoreLogger, core_options);
  core_logger->Warn("Created Core Logger!");

  runtime_configuration_ = std::make_unique<engine::RuntimeConfiguration>(client_logger, core_logger);
}

Application::~Application() {}

void Application::Run() {

  auto application_state = std::make_unique<ApplicationState>();

  auto event_dispatcher = std::make_unique<EventDispatcher>();

  auto window_controller = engine::IWindowController::Create();

  WindowOptions data([&event_dispatcher] (std::unique_ptr<Event> event) {
    event_dispatcher->Offer(event);

    std::cout << *event.get() << std::endl;
 });
  auto window = std::unique_ptr<engine::Window>(window_controller->CreateWindow(data));

  event_dispatcher->Listen<WindowOnCloseEvent>(EventType::WindowClose, [&application_state](WindowOnCloseEvent& event) {
    application_state->running = false;
    std::cout << "Closing Window\n";
  });

  while (application_state->running) {
    window_controller->OnUpdate(window);
  }
}
}