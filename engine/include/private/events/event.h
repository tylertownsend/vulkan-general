#pragma once

// #include <cstdint>

namespace engine {

enum class EventType {
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

struct Event {
  const EventType type;
  Event() = delete;
  explicit Event(EventType type) : type(type) {}
  virtual ~Event() = 0;
};
} // engine