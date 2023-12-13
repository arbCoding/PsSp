// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_APPLICATION_HPP_20231213_1048
#define PSSP_APPLICATION_HPP_20231213_1048
#pragma once
// Windows
#include "../Windows/Main.hpp"
#include "../Windows/Welcome.hpp"

namespace pssp {
class Application {
public:
  Application();

private:
  inline static Main_Window main_window{};
  inline static Welcome_Window welcome_window{};
};
}  // namespace pssp
#endif
