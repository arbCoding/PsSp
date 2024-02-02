// Copyright 2023-2024 Alexander R. Blanchette

/*!
  \file Application.cpp
  \brief Application implementation.
  \author Alexander R. Blanchette
  This file contains the Application class implementation.
  */

#include "PsSp/Application/Application.hpp"

namespace pssp {
/*!
  \brief Application constructor.

  Creates the main_window object and the welcome_window object.

  Logs status after creation.
  */
Application::Application() {
  main_window = std::make_unique<MainWindow>();
  main_window->show();
  welcome_window = std::make_unique<WelcomeWindow>();
  welcome_window->show();
  spdlog::trace("Application ready.");
}
}  // namespace pssp
