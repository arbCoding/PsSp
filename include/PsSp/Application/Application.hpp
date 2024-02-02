// Copyright 2023-2024 Alexander R. Blanchette

/*!
  \file Application.hpp
  \brief Application interface.
  \author Alexander R. Blanchette
  This file contains the Application class interface which is used to handle
  the main program using RAII.
  */

#ifndef PSSP_APPLICATION_HPP_20231213_1048
#define PSSP_APPLICATION_HPP_20231213_1048
#pragma once
// PsSp
#include "PsSp/Windows/Main.hpp"
#include "PsSp/Windows/Welcome.hpp"
// spdlog  https://github.com/gabime/spdlog
#include <spdlog/spdlog.h>
// Standard library
//   https://en.cppreference.com/w/cpp/standard_library
// std::unique_ptr
#include <memory>

namespace pssp {
/*!
  \brief Main application class.

  This manages the application (created in main()).

  \class Application
 */
class Application {
public:
  Application();

private:
  //! Unique Pointer to the Main_Window object
  std::unique_ptr<MainWindow> main_window{};
  //! Unique Pointer to the Welcome_Window object
  std::unique_ptr<WelcomeWindow> welcome_window{};
};
}  // namespace pssp
#endif
