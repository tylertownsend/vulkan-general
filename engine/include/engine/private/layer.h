#pragma once
#include <memory>

#include "engine/private/window.h"

namespace engine {

class Layer {
 public:
  virtual void OnAttach(std::unique_ptr<Window>& window) = 0;
  virtual void OnUpdate(std::unique_ptr<Window>& window) = 0;
};
}