#pragma once
#include <functional>
#include <memory>
#include <string>

#include <GLFW/glfw3.h>

#include "engine/private/events/event.h"

namespace engine {

using EventCallbackFn = std::function<void(std::unique_ptr<Event>)>;

struct WindowOptions {
  unsigned int width;
  unsigned int height;
  std::string title;
  EventCallbackFn callback;
  WindowOptions(EventCallbackFn callback,
                const std::string& title = "Engine",
                unsigned int width = 1280,
                unsigned int height = 720)
    : title(title), height(height), width(width), callback(callback) {}
  WindowOptions(WindowOptions& other) {
    *this = other;
  }
};

class Window {
 public:
  WindowOptions data;
  Window(const WindowOptions& options) : data(data) {
    data = options;
  }
  virtual ~Window() {}
};
} // namespace engine