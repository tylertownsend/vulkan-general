#pragma once
#include <memory>

#include "engine/private/events/event_dispatcher.h"
#include "engine/private/events/event.h"
#include "engine/private/window.h"

namespace engine {

class IWindowController {

 public:
  virtual void OnEvent(std::unique_ptr<Window>& window, std::unique_ptr<Event> event) = 0;
  virtual Window* CreateWindow(const WindowOptions& props) = 0;
  static IWindowController* Create(engine::EventDispatcher* dispatcher);
};
} // namespace engine