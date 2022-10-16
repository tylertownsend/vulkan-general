#pragma once

#include "engine/private/events/event.h"

namespace engine {

struct WindowOnCloseEvent: public Event {
  WindowOnCloseEvent(): Event(EventType::WindowClose) {}
  ~WindowOnCloseEvent() { }
};

}