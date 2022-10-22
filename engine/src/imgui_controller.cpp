#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "core.h"
#include "linux_window.h"
#include "imgui_controller.h"

namespace engine {

const char* glsl_version = "#version 130";

bool init_glew();
void setup_backend();

// Consider how to handle events per window/layer
// the events should be handled in a top down fashion based on the window/layer
// with the current dispatcher this will create
// many copies per layer.
// on implementation detail is to have an additional layer on top of the dispatcher
// for layers. so the indexes are
// - layer -> eventType -> [EventDelgates]
// 
// problem with this is if the implementation of the stack changes then the delegates need
// to swap
// deleteing a layer will also invalidate the delgate
// if the layers is small ~20 there should be little overhead with managing indices corresponding
// to the layer of propcessing.
void ImGuiController::OnAttach(std::unique_ptr<Window>& window) {
  init_glew();
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  setup_backend();

#ifdef ENGINE_PLATFORM_LINUX
  auto platform_window = static_cast<engine::p_linux::Window*>(window.get());
  auto opengl_window = static_cast<GLFWwindow*>(platform_window->window);
#else
  #error "Only Linux Platform Supported"
#endif

  ImGui_ImplGlfw_InitForOpenGL(opengl_window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
}

void ImGuiController::OnUpdate(std::unique_ptr<Window>& window) {
  ImGuiIO& io = ImGui::GetIO();
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  float time = (float)glfwGetTime();
  io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f);
  m_Time = time;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // TODO: Move render contents to platform
  // BEGIN: Render Contents
  //
  bool show_demo_window = true;
  bool show_another_window = true;
  // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
  if (show_demo_window) {
    ImGui::ShowDemoWindow(&show_demo_window);
  }

  // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
  {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
    ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
  }

  //
  // END: Render Contents
  //


  // Rendering
  // Window controller will swap buffers after each event poll
  ImGui::Render();
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool init_glew() {
  auto result_code = glewInit();
  if (result_code != GLEW_OK) {
    auto out_string = glewGetErrorString(result_code);
    std::cout << "Failed to initialize GLEW\n " << out_string << std::endl;
  }
  return result_code;
}

void setup_backend() {
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // // Setup Platform/Renderer backends
  // io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
  // io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
  // io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
  // io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
  // io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
  // io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
  // io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
  // io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
  // io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
  // io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
  // io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
  // io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
  // io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
  // io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
  // io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
  // io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
  // io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
  // io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
  // io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
  // io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
  // io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
}
}