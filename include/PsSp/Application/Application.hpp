// Copyright 2023-2024 Alexander R. Blanchette

/*!
  \file Application.hpp
  \brief Contains the Application class
  \author Alexander R. Blanchette
  This file contains the application class which is used to handle
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
//! Application class
class Application {
public:
  Application();

private:
  //! Unique Pointer to the Main_Window object
  std::unique_ptr<Main_Window> main_window{};
  //! Unique Pointer to the Welcome_Window object
  std::unique_ptr<Welcome_Window> welcome_window{};
};
}  // namespace pssp
#endif
