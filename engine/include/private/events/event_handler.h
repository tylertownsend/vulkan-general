#pragma once
#include "memory"

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
  EventHandler(T* state, void (*HandleFunc)(T*, EventT*) function):
    _state(state),
    _function(function) { }

  void Call(std::unique_ptr<Event>) {
    (*_function)(static_cast<EventT*>(_state, event));
  }

 private:
  T* _state;
  void (*HandleFunc)(T*, EventT*) _function;
};
} // engine