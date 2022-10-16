#pragma once

#include "engine/private/events/event.h"

namespace engine {

struct WindowOnCloseEvent: public Event {
  WindowOnCloseEvent(): Event(EventType::WindowClose) {}
  void Serialize(std::ostream& os) const override { Event::SerializeBase(os); }
  ~WindowOnCloseEvent() { }
};

}