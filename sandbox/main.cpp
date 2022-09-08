#include <iostream>

#include <engine/engine.h>

class Sandbox : public engine::Application {
 public:
  Sandbox() {
  }
  ~Sandbox() { }
};

engine::Application* engine::CreateApplication() {
  return new Sandbox();
}