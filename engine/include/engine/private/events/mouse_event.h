#pragma once
#include <ostream>

#include "engine/private/events/event.h"

namespace engine {

struct MouseMoveEvent: public Event {
  const float x_offset;
  const float y_offset;
  MouseMoveEvent(const float x_offset, const float y_offset): 
    x_offset(x_offset),
    y_offset(y_offset),
    Event(EventType::MouseMoved) {}
  ~MouseMoveEvent() {}

  void Serialize(std::ostream& os) const override {
    Event::SerializeBase(os);
    os << ": " << x_offset << ", " << y_offset;
  }
};
struct MouseScrollEvent: public Event {
  const float x_offset;
  const float y_offset;

  MouseScrollEvent(const float x_offset, const float y_offset):
    x_offset(x_offset),
    y_offset(y_offset),
    Event(EventType::MouseScrolled) {}

  void Serialize(std::ostream& os) const override {
    Event::SerializeBase(os);
    os << ": " << x_offset << ", " << y_offset;
  }

  ~MouseScrollEvent() {}
};

struct MouseClickEvent: public Event {
  MouseClickEvent() : Event(EventType::MouseButtonPressed) {}
  void Serialize(std::ostream& os) const override { Event::SerializeBase(os); }
  ~MouseClickEvent() {}
};

}

// std::ostream& operator << (std::ostream& o, const engine::MouseMoveEvent& e) {
//   o << "MouseScrolledEvent: " << e.x_offset << ", " << e.y_offset;
//   return o; 
// }