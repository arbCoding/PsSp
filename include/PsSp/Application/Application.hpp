// Copyright 2023 Alexander R. Blanchette

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
class Application {
public:
  Application();

private:
  std::unique_ptr<Main_Window> main_window{};
  std::unique_ptr<Welcome_Window> welcome_window{};
};
}  // namespace pssp
#endif
