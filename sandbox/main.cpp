#include <iostream>

#include <engine/engine.h>

class Sandbox : public engine::Application {
 public:
  Sandbox() {
  }
  ~Sandbox() { }
};

engine::Application* engine::CreateApplication() {
  std::cout << "Starting sandbox app\n";
  return new Sandbox();
}