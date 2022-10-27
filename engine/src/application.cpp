#include <iostream>
#include <memory>

#include "engine/private/events/window.h"
#include "engine/private/application.h"
#include "engine/private/log.h"
#include "engine/private/runtime_configuration.h"
#include "core.h"
#include "imgui_controller.h"

namespace engine {

struct ApplicationState {
  bool running;
  ApplicationState(): running(true) {}
};

// void
auto window_event_callback(std::unique_ptr<EventDispatcher>& event_dispatcher) {
  return [&event_dispatcher] (std::unique_ptr<Event> event) {
    event_dispatcher->Offer(event);
    std::cout << *event.get() << std::endl;
  };
}

Application::Application() {
  engine::monitor::LoggerOptions client_options("App");
  auto client_logger = engine::monitor::Create(engine::monitor::LoggerType::ClientLogger, client_options);
  client_logger->Info("Created Client Logger!");

  engine::monitor::LoggerOptions core_options("Engine");
  auto core_logger = engine::monitor::Create(engine::monitor::LoggerType::CoreLogger, core_options);
  core_logger->Warn("Created Core Logger!");

  runtime_configuration_.reset(new engine::RuntimeConfiguration(client_logger, core_logger));

  window_controller_.reset(engine::IWindowController::Create());

  event_dispatcher_ = std::make_unique<EventDispatcher>();

  auto event_callback = window_event_callback(event_dispatcher_);
  WindowOptions data(event_callback);
  window_.reset((window_controller_->CreateWindow(data)));

  application_stack_.reset(new ApplicationStack());
}

Application::~Application() {}


void Application::Run() {
  // std::cout << "her\n";
  auto application_state = std::make_unique<ApplicationState>();

  event_dispatcher_->Listen<WindowOnCloseEvent>(EventType::WindowClose, [&application_state](WindowOnCloseEvent& event) {
    application_state->running = false;
    std::cout << "Closing Window\n";
  });

  auto imgui = std::make_unique<ImGuiController>();
  imgui->OnAttach(window_);

  while (application_state->running) {
    window_controller_->OnUpdate(window_);
    imgui->Start();
    imgui->OnUpdate(window_);
    imgui->End();
  }
}

void Application::Push(std::unique_ptr<Layer> layer) {
  application_stack_->Push(std::move(layer));
  layer->OnAttach(window_);
}
}