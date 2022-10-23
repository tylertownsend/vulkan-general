#pragma once
#include <memory>

#include "engine/private/events/event.h"
#include "engine/private/window.h"

namespace engine {

class Layer {
 public:
  virtual void OnEvent(std::unique_ptr<Event>& event) = 0;
  virtual void OnUpdate(std::unique_ptr<Window>& window) = 0;
};
}