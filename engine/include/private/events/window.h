#pragma once

#include "engine/private/events/event.h"

namespace engine {

struct WindowOnCloseEvent: public Event {
  WindowOnCloseEvent(): Event(EventType::WindowClose) {}
  void Serialize(std::ostream& os) const override { Event::SerializeBase(os); }
  ~WindowOnCloseEvent() { }
};

struct WindowResizeEvent: public Event {
  const float height;
  const float width;

  WindowResizeEvent(const float new_height, const float new_width):
    height(new_height),
    width(new_width),
    Event(EventType::WindowResize) {}

  void Serialize(std::ostream& os) const override {
    Event::SerializeBase(os);
    os << ": " << height << ", " << width;
  }

  ~WindowResizeEvent() {}
};

}