#pragma once

#include "application.h"
#include "log.h"

extern engine::Application* engine::CreateApplication();

int main(int argc, char** argv) {
  engine::monitor::LoggerOptions clientOptions("App");
  auto clientLogger = engine::monitor::Create(engine::monitor::LoggerType::ClientLogger, clientOptions);
  clientLogger->Info("Created Client Logger!");

  engine::monitor::LoggerOptions coreOptions("Engine");
  auto coreLogger = engine::monitor::Create(engine::monitor::LoggerType::CoreLogger, coreOptions);
  coreLogger->Warn("Created Core Logger!");

  auto app = engine::CreateApplication();
  app->Run();
  delete app;
}