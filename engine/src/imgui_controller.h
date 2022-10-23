#pragma once
#include <memory>

#include "engine/private/layer.h"
#include "engine/private/window.h"

namespace engine {

class ImGuiController : public Layer {
 public:
  ImGuiController() = default;
  ~ImGuiController() = default;
  void OnAttach(std::unique_ptr<Window>& window);
  void OnUpdate(std::unique_ptr<Window>& window);
 private:
  float m_Time = 0.0f;
};
}