// Copyright 2023 Alexander R. Blanchette

// Program main window
#include "Application/Application.hpp"
// #include "Logging/Console_Sink.hpp"
// fltk https://www.fltk.org/doc-1.4
#include <FL/Fl.H>
// spdlog https://github.com/gabime/spdlog
#include <spdlog/spdlog.h>
// Standard library
//   https://en.cppreference.com/w/cpp/standard_library

int main() {
  std::unique_ptr<pssp::Application> app{std::make_unique<pssp::Application>()};
  int return_value{Fl::run()};
  return return_value;
}
