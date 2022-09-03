#pragma once

namespace engine {

class Application {

 public:
  Application();
  virtual ~Application();
  void Run();
};


/**
 * @brief Create a Application object which is defined by the client.
 * 
 * @return Application* 
 */
Application* CreateApplication();

} // namespace engine