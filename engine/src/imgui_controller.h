#pragma once
#include <memory>

#include "engine/private/events/event_dispatcher.h"
#include "engine/private/layer.h"
#include "engine/private/window.h"

namespace engine {

class ImGuiController : public Layer {
 public:
  ImGuiController();
  ~ImGuiController() = default;
  void OnAttach(std::unique_ptr<Window>& window) override;
  void OnUpdate(std::unique_ptr<Window>& window) override;
  void Start();
  void End();
 private:
  float m_Time = 0.0f;
  std::unique_ptr<EventDispatcher> event_dispatcher_ = nullptr;
};
}