#pragma once
#include <functional>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "event.h"
#include "event_handler.h"

namespace engine {

class EventDispatcher {
 public:
  EventDispatcher() = default;
  ~EventDispatcher() {
    for (auto& subscriber : _subscribers) {
      for (auto& event_delegate : subscriber.second ) {
        delete event_delegate;
      }
    }
  }

  template<class EventT>
  void Listen(EventType type, std::function<void(EventT&)> handl_func) {
    IEventHandler* handler = new EventHandler<EventT>(handl_func);
    _subscribers[type].push_back(handler);
  }

  void Offer(std::unique_ptr<Event>& event) {

    // We want to use find here so we don't create a handler for this current level.
    auto subscribers = _subscribers.find(event->type);
    if (subscribers == _subscribers.end()) {
      return;
    }
    auto event_data = *event.get();
    for (auto& subscriber : subscribers->second) {
      subscriber->Call(event_data);
    }
  }

 private:
  std::unordered_map<EventType, std::vector<IEventHandler*>> _subscribers;
};
} // engine