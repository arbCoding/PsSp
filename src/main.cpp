// Copyright 2023 Alexander R. Blanchette

// Program main window
#include "Application/Application.hpp"
/* fltk https://www.fltk.org/doc-1.4 */
#include <FL/Fl.H>
/* Standard library
   https://en.cppreference.com/w/cpp/standard_library */
// std::unique_ptr
#include <memory>

int main() {
  std::unique_ptr<pssp::Application> app{std::make_unique<pssp::Application>()};
  return Fl::run();
}
