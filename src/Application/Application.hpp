// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_APPLICATION_HPP_20231213_1048
#define PSSP_APPLICATION_HPP_20231213_1048
#pragma once
// Windows
#include "../Windows/Main.hpp"
#include "../Windows/Welcome.hpp"
/* Standard library
   https://en.cppreference.com/w/cpp/standard_library */
// std::unique_ptr
#include <memory>

namespace pssp {
class Application {
public:
  Application();

private:
  std::unique_ptr<Main_Window> main_window{};
  std::unique_ptr<Welcome_Window> welcome_window{};
};
}  // namespace pssp
#endif
