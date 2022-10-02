#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "event.h"
#include "event_handler.h"

namespace engine {


class EventDispatcher {
 public:
  EventDispatcher() = default;

  template<class T, class EventT>
  void Listen(T* state, void (*HandleFunc)(T*, EventT*), EventType type) {
    auto handler = std::make_unique<EventHandler>(state, HandleFunc);
    _subscribers[type].push_back(handler);
  }

  void Offer(std::unique_ptr<Event> event) {
    for (auto& subscriber : this->_subscribers.at(event->type)) {
      subscriber->Call(std::move(event));
    }
  }

 private:
  std::unordered_map<EventType, std::vector<std::unique_ptr<IEventHandler>>> _subscribers;
};
} // engine