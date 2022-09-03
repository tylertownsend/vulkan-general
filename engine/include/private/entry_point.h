#pragma once

#include "application.h"

extern engine::Application* engine::CreateApplication();

int main(int argc, char** argv) {
  auto app = engine::CreateApplication();
  app->Run();
  delete app;
}