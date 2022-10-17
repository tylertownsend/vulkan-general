#pragma once
#include <memory>

#include "event.h"

namespace engine {

class IEventHandler {
 public:
  virtual void Call(Event& event) = 0;
};

template<class EventT>
class EventHandler : public IEventHandler {
 public:
  EventHandler() = delete;
  EventHandler(std::function<void(EventT&)> handle_func):
    _function(handle_func) { }

  void Call(Event& event) {
    auto upcast_event = static_cast<EventT&>(event);
    _function(upcast_event);
  }

 private:
  std::function<void(EventT&)> _function;
};
} // engine