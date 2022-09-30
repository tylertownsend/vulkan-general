#pragma once
#include <unordered_map>
#include <vector>

#include "event.h"
#include "event_handler.h"

namespace engine {


class EventDispatcher {
 public:
  EventDispatcher() = default;

  template<class T, class EventT>
  void Listen(T*, void (T::*memFn)(EventT*)) {

  }

  void Offer(std::unique_ptr<Event> event) {
    for (auto& subscriber : this->_subscribers.at(event->type)) {
      subscriber.Call(std::move(event));
    }
  }

 private:
  std::unordered_map<EventType, std::vector<IEventHandler>> _subscribers;
};
} // engine