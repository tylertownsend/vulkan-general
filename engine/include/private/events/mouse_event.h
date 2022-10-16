#pragma once
#include <ostream>

#include "engine/private/events/event.h"

namespace engine {

struct MouseMoveEvent: public Event {
  const float x_offset;
  const float y_offset;
  MouseMoveEvent(const float x_offest, const float y_offset): x_offset(x_offset), y_offset(y_offset), Event(EventType::MouseMoved) {}
  ~MouseMoveEvent() {}

  friend std::ostream& operator << (std::ostream& o, const engine::MouseMoveEvent& e) {
    o << "MouseMoveEvent: " << e.x_offset << ", " << e.y_offset;
    return o; 
  }
};

struct MouseClickEvent: public Event {
  MouseClickEvent() : Event(EventType::MouseButtonPressed) {}
  ~MouseClickEvent() {}
};

}

// std::ostream& operator << (std::ostream& o, const engine::MouseMoveEvent& e) {
//   o << "MouseScrolledEvent: " << e.x_offset << ", " << e.y_offset;
//   return o; 
// }