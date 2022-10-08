#pragma once
#include <memory>

#include "event.h"

namespace engine {

class IEventHandler {
 public:
  virtual void Call(std::unique_ptr<Event>) = 0;
};

template<class T, class EventT>
class EventHandler : IEventHandler {
 public:
  EventHandler() = delete;
  EventHandler(T* state, void (*HandleFunc)(T*, EventT*)):
    _state(state),
    _function(HandleFunc) { }

  void Call(std::unique_ptr<Event> event) {
    (*_function)(static_cast<EventT*>(_state, event));
  }

 private:
  T* _state;
  void (*_function)(T*, EventT*);
};
} // engine