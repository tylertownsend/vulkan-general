#pragma once

#include <ostream>
#include <iostream>
// #include <concept>

namespace engine {

enum EventType {
  WindowClose,
  WindowResize,
  WindowFocus,
  WindowLostFocus,
  WindowMoved,

  AppTick,
  AppUpdate,
  AppRender,

  KeyPressed,
  KeyReleased,

  MouseButtonPressed,
  MouseButtonReleased,
  MouseMoved,
  MouseScrolled
};

static const char * enum_strings[] = {
  "WindowCloseEvent",
  "WindowResizeEvent",
  "WindowFocusEvent",
  "WindowLostFocusEvent",
  "WindowMovedEvent",

  "AppTickEvent",
  "AppUpdateEvent",
  "AppRenderEvent",

  "KeyPressedEvent",
  "KeyReleasedEvent",

  "MouseButtonPressedEvent",
  "MouseButtonReleasedEvent",
  "MouseMovedEvent",
  "MouseScrolledEvent"
};

inline const char * get_text_from_enum(int enum_val) {
  return enum_strings[enum_val];
}

struct Event {
  const EventType type;
  Event() = delete;
  explicit Event(EventType type) : type(type) {}
  // virtual ~Event();

  // define the operator inside the class definition.
  friend std::ostream& operator << (std::ostream& o, const engine::Event& e) {
    auto output = get_text_from_enum(e.type);
    o << output;
    return o; 
  }
};
// Requires c++20
// concept Printable = requires(std::ostream& os, Event a)
// {
//     os << a;
// };
} // engine