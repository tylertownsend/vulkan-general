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

  virtual ~Event() = default;

  void SerializeBase(std::ostream& os) const {
    auto output = get_text_from_enum(type);
    os << output;
  }

  virtual void Serialize(std::ostream& os) const = 0;

  // define the operator inside the class definition.
  friend std::ostream& operator << (std::ostream& os, const engine::Event& e) {
    e.Serialize(os);
    return os; 
  }
};
} // engine