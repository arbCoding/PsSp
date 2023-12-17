// Copyright 2023 Alexander R. Blanchette

// PsSp
#include "PsSp/Application/Application.hpp"
// fltk https://www.fltk.org/doc-1.4
// spdlog https://github.com/gabime/spdlog
#include <spdlog/spdlog.h>
// Standard library
//   https://en.cppreference.com/w/cpp/standard_library

int main() {
  std::unique_ptr<pssp::Application> app{std::make_unique<pssp::Application>()};
  spdlog::info("PsSp is ready!");
  int return_value{Fl::run()};
  return return_value;
}
