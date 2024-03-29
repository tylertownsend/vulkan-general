#include <iostream>

#include <engine/engine.h>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

glm::mat4 camera(float Translate, glm::vec2 const & Rotate) {
  glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.f);
  glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Translate));
  View = glm::rotate(View, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
  View = glm::rotate(View, Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
  return Projection * View * Model;
}

class ExampleLayer : public engine::Layer {

 public:
  ExampleLayer() : engine::Layer() {
    auto  cam = camera(5.0f, {0.5f, 0.5f});
    // cam.
  }

  void OnUpdate(std::unique_ptr<engine::Window>& window) override {

  }

  void OnAttach(std::unique_ptr<engine::Window>& window) override {

  }
};

class Sandbox : public engine::Application {
 public:
  Sandbox() {
  }
  ~Sandbox() { }
};

engine::Application* engine::CreateApplication() {
  return new Sandbox();
}