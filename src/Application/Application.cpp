// Copyright 2023 Alexander R. Blanchette

#include "Application.hpp"

namespace pssp {
Application::Application() {
  main_window.end();
  main_window.show();
  welcome_window.end();
  welcome_window.show();
}
}  // namespace pssp
