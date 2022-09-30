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
  EventHandler(T* instance, void (T::*MemberFunc)(EventT*) function):
    _instance(instance),
    _function(function) { }

  void Call(std::unique_ptr<Event>) {
    (_instance->*_function)(static_cast<EventT*>(event));
  }

 private:
  T* _instance;
  void (T::*MemberFunc)(EventT*) _function;
};
} // engine