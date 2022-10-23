#pragma once
#include <memory>

#include "engine/private/window.h"

namespace engine {

class IWindowController {

 public:
  virtual ~IWindowController() = default;
  virtual void OnUpdate(std::unique_ptr<Window>& window) = 0;
  virtual Window* CreateWindow(const WindowOptions& props) = 0;
  static IWindowController* Create();
};
} // namespace engine