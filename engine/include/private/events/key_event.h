#pragma once
#include <iostream>
#include <ostream>
#include <memory>

#include "event.h"

namespace engine {

struct KeyEvent {
  const int key_code;

  KeyEvent(int key_code) : key_code(key_code) {}
};

struct KeyPressedEvent : public KeyEvent, public Event {
  const int repeats;
  KeyPressedEvent(int key_code, int repeats) :
    KeyEvent(key_code),
    Event(EventType::KeyPressed),
    repeats(repeats) {}
  ~KeyPressedEvent(){}
};

struct KeyReleasedEvent : public KeyEvent, public Event {
  KeyReleasedEvent(int key_code) : KeyEvent(key_code), Event(EventType::KeyReleased) {}
  ~KeyReleasedEvent(){}
};
} // namespace engine

std::ostream& operator<<(std::ostream& out, engine::KeyPressedEvent const& curr) {
  out << "KeyPressedEvent: " << curr.key_code << " (" << curr.repeats << " repeats)";
  return out;
}

std::ostream& operator<<(std::ostream& out, engine::KeyReleasedEvent const& curr) {
  out << "KeyReleasedEvent: " << curr.key_code;
  return out;
}

inline void HandleKeyPressedEvent(std::unique_ptr<engine::KeyPressedEvent> event) {
  std::cout << "Handled: " << *event << "\n";
}