// Copyright 2023 Alexander R. Blanchette

#include "PsSp/Application/Application.hpp"

namespace pssp {
Application::Application() {
  main_window = std::make_unique<Main_Window>();
  main_window->show();
  welcome_window = std::make_unique<Welcome_Window>();
  welcome_window->show();
  spdlog::info("Application ready.");
}
}  // namespace pssp
